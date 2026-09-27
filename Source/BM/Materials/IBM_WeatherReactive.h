#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IBM_WeatherReactive.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UBM_WeatherReactive : public UInterface
{
	GENERATED_BODY()
};

class IBM_WeatherReactive
{
	GENERATED_BODY()

public:
	/**
	 * Called by the global Director-AI to push weather state into the Material Parameter Collection.
	 * The material system uses these values to drive puddles, wetness, and wet reflections.
	 */
	virtual void SetWeatherState(float RainIntensity, float Wetness, float WetReflections) = 0;

	/**
	 * Updates local surface response based on the current weather and material type.
	 * Example:
	 * - Asphalt: puddle accumulation and darkened gloss
	 * - Concrete / Brick: running water streaks and wet mask
	 */
	virtual void ApplySurfaceWeatherResponse(float DeltaTime) = 0;

	/**
	 * Allows material surfaces to absorb water in a localized way.
	 */
	virtual void SetPuddleAccumulation(float PuddleAmount) = 0;

	/**
	 * Allows water to appear as a running flow across concrete / brick surfaces.
	 */
	virtual void SetRunningWaterMask(float FlowMaskValue) = 0;
};
