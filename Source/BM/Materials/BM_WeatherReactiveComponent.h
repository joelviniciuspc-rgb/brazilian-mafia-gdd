#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "IBM_WeatherReactive.h"
#include "BM_WeatherReactiveComponent.generated.h"

UENUM(BlueprintType)
enum class EBM_SurfaceType : uint8
{
	Asphalt,
	Concrete,
	Brick,
	Metal,
	Wood,
	Soil,
	Unknown
};

/**
 * UBM_WeatherReactiveComponent
 *
 * Pushes weather state into an MPC to drive wetness, reflection, puddles, and running water.
 * This is designed for open-world environments such as São Paulo, Rio, and Campo Grande.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRAZILIANMAFIA_API UBM_WeatherReactiveComponent : public UActorComponent, public IBM_WeatherReactive
{
	GENERATED_BODY()

public:
	UBM_WeatherReactiveComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// -------------------------------------------------------------------------
	// IBM_WeatherReactive implementation
	// -------------------------------------------------------------------------
	virtual void SetWeatherState(float RainIntensity, float Wetness, float WetReflections) override;
	virtual void ApplySurfaceWeatherResponse(float DeltaTime) override;
	virtual void SetPuddleAccumulation(float PuddleAmount) override;
	virtual void SetRunningWaterMask(float FlowMaskValue) override;

	// -------------------------------------------------------------------------
	// Surface setup
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetSurfaceType(EBM_SurfaceType NewSurfaceType);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	EBM_SurfaceType GetSurfaceType() const { return CurrentSurfaceType; }

	// -------------------------------------------------------------------------
	// Material collection setup
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeatherParameterCollection(UMaterialParameterCollection* InCollection);

	UFUNCTION(BlueprintCallable, Category = "Weather")
	UMaterialParameterCollection* GetWeatherParameterCollection() const { return WeatherParameterCollection; }

protected:
	/** Parameter collection used by all reactive materials. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather|MPC")
	UMaterialParameterCollection* WeatherParameterCollection;

	/** Current material surface type. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	EBM_SurfaceType CurrentSurfaceType;

	/** Director-AI driven weather state. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
	float RainIntensity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
	float Wetness;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
	float WetReflections;

	/** Localized puddle creation for asphalt. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
	float PuddleAccumulation;

	/** Running water mask for concrete / brick. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather")
	float RunningWaterMask;

private:
	/** Updates all runtime parameters in the material collection instance. */
	void UpdateMaterialParameterCollection();
};
