#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Texture2D.h"
#include "BM_TextureStreamerComponent.generated.h"

UENUM(BlueprintType)
enum class EBM_TextureProfile : uint8
{
	PCConsole,
	Mobile
};

/**
 * UBM_TextureStreamerComponent
 *
 * Handles runtime texture memory reduction for open-world streaming.
 * This is a practical manager for modern UE5 projects targeting PC/Console and Mobile.
 *
 * Responsibilities:
 * - Track texture groups and mip bias at runtime
 * - Adjust streaming behavior per platform profile
 * - Preserve 4K/8K for PC/Console while reducing memory for Mobile
 * - Keep memory under the 2-3GB budget target for mobile builds
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRAZILIANMAFIA_API UBM_TextureStreamerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBM_TextureStreamerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
	void SetTextureProfile(EBM_TextureProfile NewProfile);

	UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
	EBM_TextureProfile GetTextureProfile() const { return TextureProfile; }

	/** Registers a texture for runtime streaming control. */
	UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
	void RegisterTexture(UTexture2D* Texture, ETextureGroup TextureGroup = TEXTUREGROUP_World);

	/** Applies platform-specific streaming and mip settings. */
	UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
	void ConfigureTextureForPlatform(UTexture2D* Texture, ETextureGroup Group = TEXTUREGROUP_World);

	/** Applies aggressive downscaling on Mobile to fit the 2-3GB memory budget. */
	UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
	void ApplyMobileBudgetSettings();

	/** Rebuilds MIP usage for the registered textures. */
	UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
	void UpdateRuntimeMipBudget();

	/** Empties the registered texture list. */
	UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
	void ClearRegisteredTextures();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Streaming")
	EBM_TextureProfile TextureProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Streaming")
	bool bEnableMipStreaming;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Streaming")
	int32 MobileMipBias;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture Streaming")
	int32 ConsoleMipBias;

	/** Registered textures that this manager actively controls. */
	UPROPERTY()
	TArray<UTexture2D*> RegisteredTextures;

private:
	void UpdateTextureEntry(UTexture2D* Texture, ETextureGroup Group);
};
