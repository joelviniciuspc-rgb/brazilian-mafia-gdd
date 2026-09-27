#include "BM_SurfaceWearComponent.h"

UBM_SurfaceWearComponent::UBM_SurfaceWearComponent()
{
	DustStrength = 1.0f;
	RustStrength = 1.0f;
	ScratchStrength = 1.0f;
	BloodStrength = 1.0f;
}

void UBM_SurfaceWearComponent::RegisterMesh(UMeshComponent* MeshComponent)
{
	if (MeshComponent == nullptr)
	{	return;
	}

	GetOrCreateDynamicMaterial(MeshComponent);
}

void UBM_SurfaceWearComponent::ApplySurfaceWear(UMeshComponent* MeshComponent,
	float DustAmount,
	float RustAmount,
	float ScratchAmount,
	float BloodAmount)
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = GetOrCreateDynamicMaterial(MeshComponent);
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	SetWearParameters(DynamicMaterial, DustAmount, RustAmount, ScratchAmount, BloodAmount);
}

void UBM_SurfaceWearComponent::ApplyWearToBone(USkinnedMeshComponent* SkinnedMesh,
	FName BoneName,
	float DustAmount,
	float RustAmount,
	float ScratchAmount,
	float BloodAmount)
{
	if (SkinnedMesh == nullptr)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = GetOrCreateDynamicMaterial(SkinnedMesh);
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	// This produces a skin-local material response by driving mask parameters.
	// For a production implementation, these masks can be split by bone region and serialized to the material.
	SetWearParameters(DynamicMaterial,
		DustAmount * DustStrength,
		RustAmount * RustStrength,
		ScratchAmount * ScratchStrength,
		BloodAmount * BloodStrength);

	if (BoneName != NAME_None)
	{
		DynamicMaterial->SetScalarParameterValue(FName(TEXT("BoneWearMask")), 1.0f);
		DynamicMaterial->SetScalarParameterValue(FName(*BoneName.ToString() + TEXT("_Dust")), DustAmount);
		DynamicMaterial->SetScalarParameterValue(FName(*BoneName.ToString() + TEXT("_Rust")), RustAmount);
		DynamicMaterial->SetScalarParameterValue(FName(*BoneName.ToString() + TEXT("_Scratch")), ScratchAmount);
		DynamicMaterial->SetScalarParameterValue(FName(*BoneName.ToString() + TEXT("_Blood")), BloodAmount);
	}
}

void UBM_SurfaceWearComponent::ClearWearMask(UMeshComponent* MeshComponent)
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = RegisteredDynamicMaterials.FindRef(TWeakObjectPtr<UMeshComponent>(MeshComponent));
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	SetWearParameters(DynamicMaterial, 0.0f, 0.0f, 0.0f, 0.0f);
}

void UBM_SurfaceWearComponent::ClearAllWearMasks()
{
	for (auto& Pair : RegisteredDynamicMaterials)
	{
		if (UMaterialInstanceDynamic* DynamicMaterial = Pair.Value)
		{
			SetWearParameters(DynamicMaterial, 0.0f, 0.0f, 0.0f, 0.0f);
		}
	}
}

UMaterialInstanceDynamic* UBM_SurfaceWearComponent::GetOrCreateDynamicMaterial(UMeshComponent* MeshComponent)
{
	if (MeshComponent == nullptr)
	{
		return nullptr;
	}

	TWeakObjectPtr<UMeshComponent> WeakMesh(MeshComponent);
	UMaterialInstanceDynamic** Existing = RegisteredDynamicMaterials.Find(WeakMesh);
	if (Existing != nullptr && *Existing != nullptr)
	{
		return *Existing;
	}

	UMaterialInterface* BaseMaterial = MeshComponent->GetMaterial(0);
	UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (DynamicMaterial == nullptr)
	{
		return nullptr;
	}

	RegisteredDynamicMaterials.Add(WeakMesh, DynamicMaterial);
	SetWearParameters(DynamicMaterial, 0.0f, 0.0f, 0.0f, 0.0f);

	return DynamicMaterial;
}

void UBM_SurfaceWearComponent::SetWearParameters(UMaterialInstanceDynamic* DynamicMaterial,
	float DustAmount,
	float RustAmount,
	float ScratchAmount,
	float BloodAmount)
{
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	DynamicMaterial->SetScalarParameterValue(TEXT("DustMask"), FMath::Clamp(DustAmount, 0.0f, 1.0f));
	DynamicMaterial->SetScalarParameterValue(TEXT("RustMask"), FMath::Clamp(RustAmount, 0.0f, 1.0f));
	DynamicMaterial->SetScalarParameterValue(TEXT("ScratchMask"), FMath::Clamp(ScratchAmount, 0.0f, 1.0f));
	DynamicMaterial->SetScalarParameterValue(TEXT("BloodMask"), FMath::Clamp(BloodAmount, 0.0f, 1.0f));
}
