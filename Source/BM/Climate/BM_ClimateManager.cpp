#include "BM_ClimateManager.h"

#include "Engine/World.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "TimerManager.h"

TObjectPtr<ABM_ClimateManager> ABM_ClimateManager::GlobalClimateManagerInstance = nullptr;

ABM_ClimateManager::ABM_ClimateManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.033f; // ~30 FPS update rate for weather

	CurrentWeather = EWeatherType::Clear;
	TargetWeather = EWeatherType::Clear;
	TransitionProgress = 0.0f;
	TransitionDuration = 5.0f;
	bIsTransitioning = false;

	GlobalWetness = 0.0f;
	PuddleDepth = 0.0f;
	SpecularReflectionMultiplier = 1.0f;
	MudAccumulationRate = 0.0f;

	GlobalEnvironmentMPC = nullptr;
}

void ABM_ClimateManager::BeginPlay()
{
	Super::BeginPlay();

	// Register this instance as the global singleton
	if (GlobalClimateManagerInstance == nullptr)
	{
		GlobalClimateManagerInstance = this;
	}
	else if (GlobalClimateManagerInstance != this)
	{
		// Destroy duplicate instances
		Destroy();
		return;
	}

	// Initialize MPC parameters to current weather
	SetWeatherImmediate(CurrentWeather);
}

void ABM_ClimateManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update weather transition if active
	if (bIsTransitioning)
	{
		UpdateWeatherTransition(DeltaTime);
	}
}

void ABM_ClimateManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up singleton reference
	if (GlobalClimateManagerInstance == this)
	{
		GlobalClimateManagerInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

ABM_ClimateManager* ABM_ClimateManager::GetClimateManager()
{
	return GlobalClimateManagerInstance;
}

void ABM_ClimateManager::TransitionToWeather(EWeatherType NewWeather, float InTransitionDuration)
{
	if (NewWeather == CurrentWeather)
	{
		return; // Already at target weather
	}

	TargetWeather = NewWeather;
	TransitionDuration = FMath::Max(InTransitionDuration, 0.1f);
	TransitionProgress = 0.0f;
	bIsTransitioning = true;

	OnWeatherTransitionStarted.Broadcast(CurrentWeather, TargetWeather);
}

void ABM_ClimateManager::SetWeatherImmediate(EWeatherType NewWeather)
{
	CurrentWeather = NewWeather;
	TargetWeather = NewWeather;
	TransitionProgress = 1.0f;
	bIsTransitioning = false;

	// Get parameters for this weather type and apply immediately
	float TargetWetness = 0.0f;
	float TargetPuddleDepth = 0.0f;
	float TargetSpecularReflection = 1.0f;
	float TargetMudAccumulation = 0.0f;

	GetWeatherParameters(NewWeather, TargetWetness, TargetPuddleDepth, TargetSpecularReflection, TargetMudAccumulation);

	GlobalWetness = TargetWetness;
	PuddleDepth = TargetPuddleDepth;
	SpecularReflectionMultiplier = TargetSpecularReflection;
	MudAccumulationRate = TargetMudAccumulation;

	UpdateMaterialParameterCollection();
}

void ABM_ClimateManager::SetGlobalWetness(float NewWetness)
{
	GlobalWetness = FMath::Clamp(NewWetness, 0.0f, 1.0f);
	UpdateMaterialParameterCollection();
}

void ABM_ClimateManager::SetPuddleDepth(float NewDepth)
{
	PuddleDepth = FMath::Clamp(NewDepth, 0.0f, 1.0f);
	UpdateMaterialParameterCollection();
}

void ABM_ClimateManager::SetSpecularReflectionMultiplier(float NewMultiplier)
{
	SpecularReflectionMultiplier = FMath::Max(NewMultiplier, 0.0f);
	UpdateMaterialParameterCollection();
}

void ABM_ClimateManager::SetMudAccumulationRate(float NewRate)
{
	MudAccumulationRate = FMath::Clamp(NewRate, 0.0f, 1.0f);
	UpdateMaterialParameterCollection();
}

void ABM_ClimateManager::UpdateWeatherTransition(float DeltaTime)
{
	TransitionProgress += DeltaTime / TransitionDuration;

	if (TransitionProgress >= 1.0f)
	{
		// Transition complete
		TransitionProgress = 1.0f;
		CurrentWeather = TargetWeather;
		bIsTransitioning = false;

		// Ensure we're exactly at target values
		float TargetWetness = 0.0f;
		float TargetPuddleDepth = 0.0f;
		float TargetSpecularReflection = 1.0f;
		float TargetMudAccumulation = 0.0f;

		GetWeatherParameters(CurrentWeather, TargetWetness, TargetPuddleDepth, TargetSpecularReflection, TargetMudAccumulation);

		GlobalWetness = TargetWetness;
		PuddleDepth = TargetPuddleDepth;
		SpecularReflectionMultiplier = TargetSpecularReflection;
		MudAccumulationRate = TargetMudAccumulation;

		UpdateMaterialParameterCollection();
		BroadcastWeatherTransitionComplete();
		return;
	}

	// Apply easing to transition progress
	float EasedProgress = TransitionProgress;
	if (TransitionEasingCurve.GetRichCurveConst() && TransitionEasingCurve.GetRichCurveConst()->IsEmpty() == false)
	{
		EasedProgress = TransitionEasingCurve.Eval(TransitionProgress);
	}
	else
	{
		// Default: smooth cubic interpolation
		EasedProgress = FMath::SmoothStep(0.0f, 1.0f, TransitionProgress);
	}

	// Get current and target parameters
	float CurrentWetness, CurrentPuddleDepth, CurrentSpecularReflection, CurrentMudAccumulation;
	GetWeatherParameters(CurrentWeather, CurrentWetness, CurrentPuddleDepth, CurrentSpecularReflection, CurrentMudAccumulation);

	float TargetWetness, TargetPuddleDepth, TargetSpecularReflection, TargetMudAccumulation;
	GetWeatherParameters(TargetWeather, TargetWetness, TargetPuddleDepth, TargetSpecularReflection, TargetMudAccumulation);

	// Lerp between current and target
	GlobalWetness = FMath::Lerp(CurrentWetness, TargetWetness, EasedProgress);
	PuddleDepth = FMath::Lerp(CurrentPuddleDepth, TargetPuddleDepth, EasedProgress);
	SpecularReflectionMultiplier = FMath::Lerp(CurrentSpecularReflection, TargetSpecularReflection, EasedProgress);
	MudAccumulationRate = FMath::Lerp(CurrentMudAccumulation, TargetMudAccumulation, EasedProgress);

	UpdateMaterialParameterCollection();
}

void ABM_ClimateManager::GetWeatherParameters(EWeatherType WeatherType, float& OutWetness, float& OutPuddleDepth,
	float& OutSpecularReflection, float& OutMudAccumulation) const
{
	// Default values
	OutWetness = 0.0f;
	OutPuddleDepth = 0.0f;
	OutSpecularReflection = 1.0f;
	OutMudAccumulation = 0.0f;

	switch (WeatherType)
	{
		case EWeatherType::Clear:
		{
			OutWetness = 0.0f;
			OutPuddleDepth = 0.0f;
			OutSpecularReflection = 0.8f; // Clear sky has lower spec reflection
			OutMudAccumulation = 0.0f;
			break;
		}

		case EWeatherType::PartlyCloudy:
		{
			OutWetness = 0.1f;
			OutPuddleDepth = 0.05f;
			OutSpecularReflection = 0.9f; // Slight increase
			OutMudAccumulation = 0.02f;
			break;
		}

		case EWeatherType::Overcast:
		{
			OutWetness = 0.2f;
			OutPuddleDepth = 0.1f;
			OutSpecularReflection = 1.0f;
			OutMudAccumulation = 0.05f;
			break;
		}

		case EWeatherType::Drizzle:
		{
			OutWetness = 0.4f;
			OutPuddleDepth = 0.2f;
			OutSpecularReflection = 1.2f;
			OutMudAccumulation = 0.1f;
			break;
		}

		case EWeatherType::Rain:
		{
			OutWetness = 0.7f;
			OutPuddleDepth = 0.5f;
			OutSpecularReflection = 1.5f;
			OutMudAccumulation = 0.25f;
			break;
		}

		case EWeatherType::HeavyRain:
		{
			OutWetness = 0.95f;
			OutPuddleDepth = 0.85f;
			OutSpecularReflection = 1.8f;
			OutMudAccumulation = 0.4f;
			break;
		}

		case EWeatherType::Thunderstorm:
		{
			OutWetness = 1.0f;
			OutPuddleDepth = 0.95f;
			OutSpecularReflection = 2.0f;
			OutMudAccumulation = 0.5f;
			break;
		}

		case EWeatherType::Dust:
		{
			// Campo Grande dust storm: dry but dusty
			OutWetness = 0.0f;
			OutPuddleDepth = 0.0f;
			OutSpecularReflection = 0.6f; // Dust reduces reflections
			OutMudAccumulation = 0.9f; // Heavy dust accumulation
			break;
		}

		default:
		{
			OutWetness = 0.0f;
			OutPuddleDepth = 0.0f;
			OutSpecularReflection = 1.0f;
			OutMudAccumulation = 0.0f;
			break;
		}
	}
}

void ABM_ClimateManager::UpdateMaterialParameterCollection()
{
	if (GlobalEnvironmentMPC == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Get or create the MPC instance
	// This must be called on the game thread to ensure thread safety
	UMaterialParameterCollectionInstance* MPCInstance = World->GetParameterCollectionInstance(GlobalEnvironmentMPC);
	if (MPCInstance == nullptr)
	{
		return;
	}

	// Update all parameters
	// These parameter names MUST match the MPC asset created in the editor
	MPCInstance->SetScalarParameterValue(FName(TEXT("GlobalWetness")), GlobalWetness);
	MPCInstance->SetScalarParameterValue(FName(TEXT("PuddleDepth")), PuddleDepth);
	MPCInstance->SetScalarParameterValue(FName(TEXT("SpecularReflectionMultiplier")), SpecularReflectionMultiplier);
	MPCInstance->SetScalarParameterValue(FName(TEXT("MudAccumulationRate")), MudAccumulationRate);

	// Additional derived parameters for convenience
	MPCInstance->SetScalarParameterValue(FName(TEXT("IsRaining")), GlobalWetness > 0.3f ? 1.0f : 0.0f);
	MPCInstance->SetScalarParameterValue(FName(TEXT("RainIntensity")), GlobalWetness);
}

void ABM_ClimateManager::BroadcastWeatherTransitionComplete()
{
	OnWeatherTransitionComplete.Broadcast(CurrentWeather, TargetWeather);
}
