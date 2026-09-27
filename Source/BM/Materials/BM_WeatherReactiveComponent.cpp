#include "BM_WeatherReactiveComponent.h"

#include "Engine/World.h"
#include "Materials/MaterialParameterCollectionInstance.h"

UBM_WeatherReactiveComponent::UBM_WeatherReactiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentSurfaceType = EBM_SurfaceType::Asphalt;
	WeatherParameterCollection = nullptr;

	RainIntensity = 0.0f;
	Wetness = 0.0f;
	WetReflections = 0.0f;
	PuddleAccumulation = 0.0f;
	RunningWaterMask = 0.0f;
}

void UBM_WeatherReactiveComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateMaterialParameterCollection();
}

void UBM_WeatherReactiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ApplySurfaceWeatherResponse(DeltaTime);
}

void UBM_WeatherReactiveComponent::SetWeatherState(float InRainIntensity, float InWetness, float InWetReflections)
{
	RainIntensity = FMath::Clamp(InRainIntensity, 0.0f, 1.0f);
	Wetness = FMath::Clamp(InWetness, 0.0f, 1.0f);
	WetReflections = FMath::Clamp(InWetReflections, 0.0f, 1.0f);

	ApplySurfaceWeatherResponse(0.0f);
	UpdateMaterialParameterCollection();
}

void UBM_WeatherReactiveComponent::ApplySurfaceWeatherResponse(float DeltaTime)
{
	const float RainContribution = RainIntensity * Wetness;

	switch (CurrentSurfaceType)
	{
		case EBM_SurfaceType::Asphalt:
		{
			// Asphalt accumulates water in puddles. This keeps roads darker and more reflective.
			PuddleAccumulation = FMath::Clamp((RainContribution * 1.25f) + (Wetness * 0.5f), 0.0f, 1.0f);
			RunningWaterMask = 0.0f;
			break;
		}

		case EBM_SurfaceType::Concrete:
		case EBM_SurfaceType::Brick:
		{
			// Concrete and brick surfaces show running water and wet streaks.
			RunningWaterMask = FMath::Clamp((RainContribution * 1.15f) + (Wetness * 0.8f), 0.0f, 1.0f);
			PuddleAccumulation = 0.15f * RainContribution;
			break;
		}

		case EBM_SurfaceType::Metal:
		{
			PuddleAccumulation = FMath::Clamp(RainContribution * 0.5f, 0.0f, 0.8f);
			RunningWaterMask = FMath::Clamp(Wetness * 0.7f, 0.0f, 1.0f);
			break;
		}

		default:
		{
			PuddleAccumulation = 0.0f;
			RunningWaterMask = 0.0f;
			break;
		}
	}

	UpdateMaterialParameterCollection();
}

void UBM_WeatherReactiveComponent::SetPuddleAccumulation(float PuddleAmount)
{
	PuddleAccumulation = FMath::Clamp(PuddleAmount, 0.0f, 1.0f);
	UpdateMaterialParameterCollection();
}

void UBM_WeatherReactiveComponent::SetRunningWaterMask(float FlowMaskValue)
{
	RunningWaterMask = FMath::Clamp(FlowMaskValue, 0.0f, 1.0f);
	UpdateMaterialParameterCollection();
}

void UBM_WeatherReactiveComponent::SetSurfaceType(EBM_SurfaceType NewSurfaceType)
{
	CurrentSurfaceType = NewSurfaceType;
	ApplySurfaceWeatherResponse(0.0f);
}

void UBM_WeatherReactiveComponent::SetWeatherParameterCollection(UMaterialParameterCollection* InCollection)
{
	WeatherParameterCollection = InCollection;
	UpdateMaterialParameterCollection();
}

void UBM_WeatherReactiveComponent::UpdateMaterialParameterCollection()
{
	if (WeatherParameterCollection == nullptr)
	{
		return;
	}

	UMaterialParameterCollectionInstance* CollectionInstance = GetWorld()->GetParameterCollectionInstance(WeatherParameterCollection);
	if (CollectionInstance == nullptr)
	{
		return;
	}

	// These parameters must exist in your Material Parameter Collection asset.
	CollectionInstance->SetScalarParameterValue(TEXT("RainIntensity"), RainIntensity);
	CollectionInstance->SetScalarParameterValue(TEXT("Wetness"), Wetness);
	CollectionInstance->SetScalarParameterValue(TEXT("WetReflections"), WetReflections);
	CollectionInstance->SetScalarParameterValue(TEXT("PuddleAccumulation"), PuddleAccumulation);
	CollectionInstance->SetScalarParameterValue(TEXT("RunningWaterMask"), RunningWaterMask);
	CollectionInstance->SetScalarParameterValue(TEXT("SurfaceWetnessBlend"), FMath::Clamp(Wetness + PuddleAccumulation, 0.0f, 1.0f));
}
