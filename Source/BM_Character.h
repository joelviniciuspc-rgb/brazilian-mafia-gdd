// BM_Character.h
// Brazilian Mafia - Character System
// Gameplay Engineer: Senior Level | UE5 Production Ready
// Purpose: Base character class supporting three protagonists with abilities, movement, and aiming

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BM_Character.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ABM_Vehicle;

/**
 * ECharacterType
 * Enum representing the three playable protagonists
 * Each has unique abilities and playstyle
 */
UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	ALEMAO = 0		UMETA(DisplayName = "Thiago Alemao - Hacker/Brain"),
	MARCOLA = 1		UMETA(DisplayName = "Marcos Marcola - Combatant"),
	PAMPA = 2		UMETA(DisplayName = "Roberto Beto Pampa - Driver")
};

/**
 * ABM_Character
 * Core playable character class supporting:
 * - Movement and sprinting
 * - Aiming and weapon readiness
 * - Character-specific abilities
 * - Vehicle interaction
 */
UCLASS()
class BRAZILIANMAFIA_API ABM_Character : public ACharacter
{
	GENERATED_BODY()

public:
	ABM_Character();

protected:
	// ==================== COMPONENTS ====================
	
	/** Spring arm for camera tracking */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	/** Third-person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	// ==================== CHARACTER PROPERTIES ====================

	/** Character type (Alemao, Marcola, or Pampa) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	ECharacterType CharacterType;

	/** Character display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FString CharacterName;

	/** Health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Health")
	float MaxHealth;

	UPROPERTY(BlueprintReadWrite, Category = "Character|Health")
	float CurrentHealth;

	/** Armor/Resistance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Health")
	float Armor;

	// ==================== MOVEMENT PROPERTIES ====================

	/** Base walking speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed;

	/** Sprint speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed;

	/** Current sprint state */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting;

	/** Current aiming state */
	UPROPERTY(BlueprintReadOnly, Category = "Aiming")
	bool bIsAiming;

	/** Aiming speed reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aiming")
	float AimingSpeedMultiplier;

	// ==================== ABILITY PROPERTIES ====================

	/** Ability cooldown timer */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	float AbilityCooldown;

	/** Is ability ready */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	bool bAbilityReady;

	/** Ability description for display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FString AbilityDescription;

	// ==================== INPUT SYSTEM (Enhanced Input System) ====================

	/** Base input mapping context */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	/** Move input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	/** Look input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	/** Sprint input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	/** Aim input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimAction;

	/** Ability input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AbilityAction;

	/** Interact input action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	// ==================== VEHICLE INTERACTION ====================

	/** Currently targeted vehicle for interaction */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
	ABM_Vehicle* TargetVehicle;

	/** Can interact with vehicle */
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
	bool bCanInteractWithVehicle;

	/** Interaction range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float InteractionRange;

public:
	// ==================== LIFECYCLE ====================

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ==================== CHARACTER INITIALIZATION ====================

	/**
	 * Initialize character with specific type
	 * Sets up abilities, stats, and visuals based on character
	 */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void InitializeCharacter(ECharacterType NewCharacterType);

	/**
	 * Get character type
	 */
	UFUNCTION(BlueprintCallable, Category = "Character")
	ECharacterType GetCharacterType() const { return CharacterType; }

	/**
	 * Get character name
	 */
	UFUNCTION(BlueprintCallable, Category = "Character")
	FString GetCharacterName() const { return CharacterName; }

	// ==================== MOVEMENT ====================

	/**
	 * Handle movement input
	 */
	void MoveInput(const FInputActionValue& Value);

	/**
	 * Handle look input
	 */
	void LookInput(const FInputActionValue& Value);

	/**
	 * Start sprinting
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	/**
	 * Stop sprinting
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	/**
	 * Check if character is sprinting
	 */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }

	// ==================== AIMING ====================

	/**
	 * Start aiming
	 */
	UFUNCTION(BlueprintCallable, Category = "Aiming")
	void StartAiming();

	/**
	 * Stop aiming
	 */
	UFUNCTION(BlueprintCallable, Category = "Aiming")
	void StopAiming();

	/**
	 * Check if character is aiming
	 */
	UFUNCTION(BlueprintCallable, Category = "Aiming")
	bool IsAiming() const { return bIsAiming; }

	// ==================== ABILITIES ====================

	/**
	 * Activate character-specific ability
	 * - ALEMAO: Visão Tática (Time Slow + Analysis)
	 * - MARCOLA: Adrenalina (Damage Reduction + Accuracy Boost)
	 * - PAMPA: Fuga Extrema (Vehicle Control Enhancement)
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ActivateAbility();

	/**
	 * Set ability ready status
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void SetAbilityReady(bool bReady) { bAbilityReady = bReady; }

	/**
	 * Get ability ready status
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool IsAbilityReady() const { return bAbilityReady; }

	/**
	 * Get ability description
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	FString GetAbilityDescription() const { return AbilityDescription; }

	// ==================== HEALTH & DAMAGE ====================

	/**
	 * Apply damage to character
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float DamageAmount);

	/**
	 * Heal character
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float HealAmount);

	/**
	 * Get current health
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	/**
	 * Get health percentage
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetHealthPercentage() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	/**
	 * Check if character is alive
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	// ==================== VEHICLE INTERACTION ====================

	/**
	 * Check for nearby vehicles
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void FindNearbyVehicle();

	/**
	 * Interact with targeted vehicle (carjacking)
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void InteractWithVehicle();

	/**
	 * Get target vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	ABM_Vehicle* GetTargetVehicle() const { return TargetVehicle; }

	/**
	 * Check if can interact
	 */
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	bool CanInteractWithVehicle() const { return bCanInteractWithVehicle; }

	// ==================== LOGGING & DEBUG ====================

	/**
	 * Log character state for debugging
	 */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void LogCharacterState();

private:
	/**
	 * Setup input system (Enhanced Input System)
	 */
	void SetupInput();

	/**
	 * Apply character-specific stats based on type
	 */
	void ApplyCharacterStats();

	/**
	 * Setup character-specific ability
	 */
	void SetupAbility();
};
