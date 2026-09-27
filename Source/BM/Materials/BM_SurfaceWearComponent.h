#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "BM_SurfaceWearComponent.generated.h"

/**
 * UBM_SurfaceWearComponent
 *
 * Handles dynamic wear-like material response for:
 * - vehicle bodies (dust / rust / scratches)
 * - weapons (blood / dirt / scratches)
 * - character meshes (dust / blood splatters / weather damage)
 *
 * The core approach is to drive material parameter masks, which is the UE5-safe production
 * method for dynamic paint / damage representation without modifying generated vertex data.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BRAZILIANMAFIA_API UBM_SurfaceWearComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBM_SurfaceWearComponent();

	UFUNCTION(BlueprintCallable, Category = "Surface Wear")
	void RegisterMesh(UMeshComponent* MeshComponent);

	UFUNCTION(BlueprintCallable, Category = "Surface Wear")
	void ApplySurfaceWear(UMeshComponent* MeshComponent,
		float DustAmount,
		float RustAmount,
		float ScratchAmount,
		float BloodAmount);

	UFUNCTION(BlueprintCallable, Category = "Surface Wear")
	void ApplyWearToBone(USkinnedMeshComponent* SkinnedMesh,
		FName BoneName,
		float DustAmount,
		float RustAmount,
		float ScratchAmount,
		float BloodAmount);

	UFUNCTION(BlueprintCallable, Category = "Surface Wear")
	void ClearWearMask(UMeshComponent* MeshComponent);

	UFUNCTION(BlueprintCallable, Category = "Surface Wear")
	void ClearAllWearMasks();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Wear")
	float DustStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Wear")
	float RustStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Wear")
	float ScratchStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface Wear")
	float BloodStrength;

	UPROPERTY()
	TMap<TWeakObjectPtr<UMeshComponent>, UMaterialInstanceDynamic*> RegisteredDynamicMaterials;

private:
	UMaterialInstanceDynamic* GetOrCreateDynamicMaterial(UMeshComponent* MeshComponent);
	void SetWearParameters(UMaterialInstanceDynamic* DynamicMaterial,
		float DustAmount,
		float RustAmount,
		float ScratchAmount,
		float BloodAmount);
};
