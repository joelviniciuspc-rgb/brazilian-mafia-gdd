#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialParameterCollection.h"
#include "BM_ClimateManager.generated.h"

/**
 * EWeatherType
 * Enumeration representing weather states in the Brazilian Mafia game world.
 * Used to drive global MPC parameters for Lumen and Ray Tracing.
 */
UENUM(BlueprintType)
enum class EWeatherType : uint8
{
	Clear = 0			UMETA(DisplayName = "Clear Sky"),
	PartlyCloudy = 1	UMETA(DisplayName = "Partly Cloudy"),
	Overcast = 2		UMETA(DisplayName = "Overcast"),
	Drizzle = 3			UMETA(DisplayName = "Drizzle"),
	Rain = 4			UMETA(DisplayName = "Rain"),
	HeavyRain = 5		UMETA(DisplayName = "Heavy Rain"),
	Thunderstorm = 6	UMETA(DisplayName = "Thunderstorm"),
	Dust = 7			UMETA(DisplayName = "Dust Storm (Campo Grande)")
};

/**
 * ABM_ClimateManager
 *
 * Singleton-pattern AActor that manages global weather and environmental transitions.
 * Drives Material Parameter Collection (MPC) updates for Lumen/Ray Tracing integration.
 *
 * Responsibilities:
 * - Maintain current weather state
 * - Smoothly transition between weather types with lerping
 * - Update global MPC with wetness, puddle depth, specular reflection, mud accumulation
 * - Synchronize weather with visual effects (rain particles, lighting)
 * - Thread-safe parameter updates for multi-threaded rendering
 */
UCLASS()
class BRAZILIANMAFIA_API ABM_ClimateManager : public AActor
{
	GENERATED_BODY()

public:
	ABM_ClimateManager();

	// ========================================================================
	// Lifecycle
	// ========================================================================
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// ========================================================================
	// Singleton Pattern
	// ========================================================================

	/**
	 * Get the global climate manager instance.
	 * Returns nullptr if not yet instantiated in the world.
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate")
	static ABM_ClimateManager* GetClimateManager();

	// ========================================================================
	// Weather Transitions
	// ========================================================================

	/**
	 * Smoothly transition to a new weather state.
	 * This is thread-safe and lerps all MPC parameters over the specified duration.
	 *
	 * @param NewWeather Target weather type
	 * @param TransitionDuration Time in seconds for the transition (default 5.0 seconds)
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Weather")
	void TransitionToWeather(EWeatherType NewWeather, float TransitionDuration = 5.0f);

	/**
	 * Set weather instantly without transition.
	 * Immediately updates all MPC parameters to match the target weather.
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Weather")
	void SetWeatherImmediate(EWeatherType NewWeather);

	/**
	 * Get current weather type
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Weather")
	EWeatherType GetCurrentWeather() const { return CurrentWeather; }

	/**
	 * Get target weather type (if transitioning)
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Weather")
	EWeatherType GetTargetWeather() const { return TargetWeather; }

	// ========================================================================
	// MPC Parameter Access
	// ========================================================================

	/**
	 * Get the global wetness value (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Parameters")
	float GetGlobalWetness() const { return GlobalWetness; }

	/**
	 * Get the puddle depth value (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Parameters")
	float GetPuddleDepth() const { return PuddleDepth; }

	/**
	 * Get the specular reflection multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Parameters")
	float GetSpecularReflectionMultiplier() const { return SpecularReflectionMultiplier; }

	/**
	 * Get the mud accumulation rate (for Campo Grande soil)
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Parameters")
	float GetMudAccumulationRate() const { return MudAccumulationRate; }

	// ========================================================================
	// Manual Parameter Adjustment
	// ========================================================================

	/**
	 * Manually set global wetness (bypasses weather state)
	 * Used for narrative moments or debugging
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Parameters")
	void SetGlobalWetness(float NewWetness);

	/**
	 * Manually set puddle depth
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Parameters")
	void SetPuddleDepth(float NewDepth);

	/**
	 * Manually set specular reflection multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Parameters")
	void SetSpecularReflectionMultiplier(float NewMultiplier);

	/**
	 * Manually set mud accumulation rate
	 */
	UFUNCTION(BlueprintCallable, Category = "Climate|Parameters")
	void SetMudAccumulationRate(float NewRate);

	// ========================================================================
	// Callbacks & Events
	// ========================================================================

	/**
	 * Delegate called when weather transition completes
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWeatherTransitionComplete, EWeatherType /*OldWeather*/, EWeatherType /*NewWeather*/);
	FOnWeatherTransitionComplete OnWeatherTransitionComplete;

	/**
	 * Delegate called when weather starts transitioning
	 */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWeatherTransitionStarted, EWeatherType /*FromWeather*/, EWeatherType /*ToWeather*/);
	FOnWeatherTransitionStarted OnWeatherTransitionStarted;

protected:
	// ========================================================================
	// MPC Reference
	// ========================================================================

	/**
	 * Material Parameter Collection for global environment parameters.
	 * Must be assigned in editor or set via SetMaterialParameterCollection().
	 * Expected parameters: GlobalWetness, PuddleDepth, SpecularReflectionMultiplier, MudAccumulationRate
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate|MPC")
	TObjectPtr<UMaterialParameterCollection> GlobalEnvironmentMPC;

	// ========================================================================
	// Weather State
	// ========================================================================

	/** Current active weather type */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate")
	EWeatherType CurrentWeather;

	/** Target weather type (if transitioning) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate")
	EWeatherType TargetWeather;

	/** Transition progress (0.0 to 1.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate")
	float TransitionProgress;

	/** Total transition duration in seconds */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate")
	float TransitionDuration;

	/** Are we currently transitioning? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate")
	bool bIsTransitioning;

	// ========================================================================
	// MPC Parameter Values
	// ========================================================================

	/** Global wetness (0.0 = dry, 1.0 = soaking wet) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate|Parameters")
	float GlobalWetness;

	/** Puddle depth on flat surfaces (0.0 = none, 1.0 = deep puddles) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate|Parameters")
	float PuddleDepth;

	/** Multiplier for specular reflections (wet surfaces are more reflective) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate|Parameters")
	float SpecularReflectionMultiplier;

	/** Rate of mud accumulation on vehicles (Campo Grande specific) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climate|Parameters")
	float MudAccumulationRate;

	// ========================================================================
	// Transition Easing
	// ========================================================================

	/** Easing curve for weather transitions (for more natural feel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate|Transition")
	FRichCurve TransitionEasingCurve;

private:
	// ========================================================================
	// Static Singleton Instance
	// ========================================================================

	static TObjectPtr<ABM_ClimateManager> GlobalClimateManagerInstance;

	// ========================================================================
	// Internal Methods
	// ========================================================================

	/**
	 * Update weather transition each frame
	 */
	void UpdateWeatherTransition(float DeltaTime);

	/**
	 * Get target parameter values for a given weather type
	 */
	void GetWeatherParameters(EWeatherType WeatherType, float& OutWetness, float& OutPuddleDepth, 
		float& OutSpecularReflection, float& OutMudAccumulation) const;

	/**
	 * Push current parameter values to the Material Parameter Collection
	 * This is thread-safe
	 */
	void UpdateMaterialParameterCollection();

	/**
	 * Broadcast weather transition complete event
	 */
	void BroadcastWeatherTransitionComplete();
};
