// BM_Character.cpp
// Brazilian Mafia - Character System Implementation
// UE5 Production Ready

#include "BM_Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "BM_Vehicle.h"
#include "Kismet/GameplayStatics.h"

ABM_Character::ABM_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate character with camera
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 300.0f;

	// Create camera boom (pulls in towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Initialize character properties
	CharacterType = ECharacterType::ALEMAO;
	CharacterName = TEXT("Thiago Alemao");
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	Armor = 0.0f;
	WalkSpeed = 600.0f;
	SprintSpeed = 1200.0f;
	bIsSprinting = false;
	bIsAiming = false;
	AimingSpeedMultiplier = 0.5f;
	InteractionRange = 200.0f;
	bAbilityReady = true;
	AbilityCooldown = 0.0f;
	TargetVehicle = nullptr;
	bCanInteractWithVehicle = false;
}

void ABM_Character::BeginPlay()
{
	Super::BeginPlay();

	// Initialize character
	InitializeCharacter(CharacterType);

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Log spawn
	UE_LOG(LogTemp, Warning, TEXT("[BM] Character spawned: %s (Type: %d)"), *CharacterName, (int32)CharacterType);
}

void ABM_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update ability cooldown
	if (!bAbilityReady && AbilityCooldown > 0.0f)
	{
		AbilityCooldown -= DeltaTime;
		if (AbilityCooldown <= 0.0f)
		{
			bAbilityReady = true;
			AbilityCooldown = 0.0f;
			UE_LOG(LogTemp, Log, TEXT("[BM] Ability ready for %s"), *CharacterName);
		}
	}

	// Check for nearby vehicles
	FindNearbyVehicle();
}

void ABM_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = 
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABM_Character::MoveInput);
		}

		// Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABM_Character::LookInput);
		}

		// Sprint
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABM_Character::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABM_Character::StopSprint);
		}

		// Aim
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABM_Character::StartAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABM_Character::StopAiming);
		}

		// Ability
		if (AbilityAction)
		{
			EnhancedInputComponent->BindAction(AbilityAction, ETriggerEvent::Started, this, &ABM_Character::ActivateAbility);
		}

		// Interact
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ABM_Character::InteractWithVehicle);
		}
	}
}

void ABM_Character::InitializeCharacter(ECharacterType NewCharacterType)
{
	CharacterType = NewCharacterType;

	switch (CharacterType)
	{
		case ECharacterType::ALEMAO:
			CharacterName = TEXT("Thiago Alemao");
			MaxHealth = 100.0f;
			CurrentHealth = MaxHealth;
			Armor = 0.0f;
			AbilityDescription = TEXT("Visao Tatica - Desacelera o tempo por 5 segundos");
			break;

		case ECharacterType::MARCOLA:
			CharacterName = TEXT("Marcos Marcola");
			MaxHealth = 120.0f;
			CurrentHealth = MaxHealth;
			Armor = 10.0f;
			AbilityDescription = TEXT("Adrenalina de Combate - Reduz dano recebido e aumenta precisao");
			break;

		case ECharacterType::PAMPA:
			CharacterName = TEXT("Roberto Beto Pampa");
			MaxHealth = 95.0f;
			CurrentHealth = MaxHealth;
			Armor = 5.0f;
			AbilityDescription = TEXT("Fuga Extrema - Controle perfeito de veiculo");
			break;

		default:
			break;
	}

	ApplyCharacterStats();
	SetupAbility();

	UE_LOG(LogTemp, Warning, TEXT("[BM] Character initialized: %s"), *CharacterName);
}

void ABM_Character::MoveInput(const FInputActionValue& Value)
{
	if (!IsAlive()) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Find forward direction
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotator(0, YawRotation.Yaw, 0).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MovementVector.Y);

		// Find right direction
		const FVector RightDirection = FRotator(0, YawRotation.Yaw, 0).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ABM_Character::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABM_Character::StartSprint()
{
	if (!IsAlive() || bIsAiming) return;

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	UE_LOG(LogTemp, Log, TEXT("[BM] %s started sprinting"), *CharacterName);
}

void ABM_Character::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	UE_LOG(LogTemp, Log, TEXT("[BM] %s stopped sprinting"), *CharacterName);
}

void ABM_Character::StartAiming()
{
	if (!IsAlive()) return;

	bIsAiming = true;
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * AimingSpeedMultiplier;
	UE_LOG(LogTemp, Log, TEXT("[BM] %s started aiming"), *CharacterName);
}

void ABM_Character::StopAiming()
{
	bIsAiming = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	UE_LOG(LogTemp, Log, TEXT("[BM] %s stopped aiming"), *CharacterName);
}

void ABM_Character::ActivateAbility()
{
	if (!IsAlive() || !bAbilityReady) return;

	UE_LOG(LogTemp, Warning, TEXT("[BM] %s activated ability: %s"), *CharacterName, *AbilityDescription);

	bAbilityReady = false;
	AbilityCooldown = 10.0f; // 10 second cooldown

	// Ability effects are handled by subclasses or separate ability system
	// For now, just log
	switch (CharacterType)
	{
		case ECharacterType::ALEMAO:
			// Visao Tatica - Time slow would be implemented here
			UE_LOG(LogTemp, Warning, TEXT("[BM] ALEMAO: Activating Tactical Vision"));
			break;

		case ECharacterType::MARCOLA:
			// Adrenalina - Combat boost would be implemented here
			UE_LOG(LogTemp, Warning, TEXT("[BM] MARCOLA: Activating Combat Adrenaline"));
			break;

		case ECharacterType::PAMPA:
			// Fuga Extrema - Vehicle control boost would be implemented here
			UE_LOG(LogTemp, Warning, TEXT("[BM] PAMPA: Activating Extreme Escape"));
			break;

		default:
			break;
	}
}

void ABM_Character::TakeDamage(float DamageAmount)
{
	if (!IsAlive()) return;

	// Apply armor reduction
	float ActualDamage = DamageAmount * (1.0f - (Armor / 100.0f));
	CurrentHealth -= ActualDamage;

	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		UE_LOG(LogTemp, Error, TEXT("[BM] %s died"), *CharacterName);
		// Death logic would go here
	}

	UE_LOG(LogTemp, Log, TEXT("[BM] %s took %.1f damage (Actual: %.1f, Health: %.1f)"), 
		*CharacterName, DamageAmount, ActualDamage, CurrentHealth);
}

void ABM_Character::Heal(float HealAmount)
{
	if (!IsAlive()) return;

	CurrentHealth = FMath::Min(CurrentHealth + HealAmount, MaxHealth);
	UE_LOG(LogTemp, Log, TEXT("[BM] %s healed for %.1f (Health: %.1f)"), 
		*CharacterName, HealAmount, CurrentHealth);
}

void ABM_Character::FindNearbyVehicle()
{
	TargetVehicle = nullptr;
	bCanInteractWithVehicle = false;

	// Trace for vehicles in interaction range
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = TraceStart + GetActorForwardVector() * InteractionRange;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Pawn,
		QueryParams
	);

	if (bHit)
	{
		ABM_Vehicle* Vehicle = Cast<ABM_Vehicle>(HitResult.GetActor());
		if (Vehicle)
		{
			TargetVehicle = Vehicle;
			bCanInteractWithVehicle = true;
		}
	}
}

void ABM_Character::InteractWithVehicle()
{
	if (!bCanInteractWithVehicle || !TargetVehicle) return;

	UE_LOG(LogTemp, Warning, TEXT("[BM] %s interacting with vehicle"), *CharacterName);
	
	// Call carjack on the vehicle
	TargetVehicle->CarjackVehicle(this);
}

void ABM_Character::LogCharacterState()
{
	FString StateLog = FString::Printf(
		TEXT("[BM_CHARACTER_STATE]\n")
		TEXT("Name: %s\n")
		TEXT("Type: %d\n")
		TEXT("Health: %.1f / %.1f (%.1f%%)\n")
		TEXT("Armor: %.1f\n")
		TEXT("Sprinting: %s\n")
		TEXT("Aiming: %s\n")
		TEXT("Ability Ready: %s\n")
		TEXT("Speed: %.1f\n"),
		*CharacterName,
		(int32)CharacterType,
		CurrentHealth, MaxHealth, GetHealthPercentage() * 100.0f,
		Armor,
		bIsSprinting ? TEXT("YES") : TEXT("NO"),
		bIsAiming ? TEXT("YES") : TEXT("NO"),
		bAbilityReady ? TEXT("YES") : TEXT("NO"),
		GetCharacterMovement()->MaxWalkSpeed
	);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *StateLog);
}

void ABM_Character::ApplyCharacterStats()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ABM_Character::SetupAbility()
{
	bAbilityReady = true;
	AbilityCooldown = 0.0f;
}
