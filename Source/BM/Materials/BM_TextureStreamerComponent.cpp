#include "BM_TextureStreamerComponent.h"

UBM_TextureStreamerComponent::UBM_TextureStreamerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	TextureProfile = EBM_TextureProfile::PCConsole;
	bEnableMipStreaming = true;
	MobileMipBias = 2;
	ConsoleMipBias = 0;
}

void UBM_TextureStreamerComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateRuntimeMipBudget();
}

void UBM_TextureStreamerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UBM_TextureStreamerComponent::SetTextureProfile(EBM_TextureProfile NewProfile)
{
	TextureProfile = NewProfile;
	UpdateRuntimeMipBudget();
}

void UBM_TextureStreamerComponent::RegisterTexture(UTexture2D* Texture, ETextureGroup TextureGroup)
{
	if (Texture == nullptr)
	{
		return;
	}

	if (!RegisteredTextures.Contains(Texture))
	{
		RegisteredTextures.Add(Texture);
	}

	ConfigureTextureForPlatform(Texture, TextureGroup);
}

void UBM_TextureStreamerComponent::ConfigureTextureForPlatform(UTexture2D* Texture, ETextureGroup Group)
{
	if (Texture == nullptr)
	{
		return;
	}

	// Always assign the right texture group.
	Texture->TextureGroup = Group;

	if (TextureProfile == EBM_TextureProfile::Mobile)
	{
		// Mobile target: aggressively reduce memory. Keep lower mip footprint, lower quality as needed.
		if (Texture->GetSizeX() > 1024 || Texture->GetSizeY() > 1024)
		{
			Texture->SetMipBias(MobileMipBias);
		}
		else
		{
			Texture->SetMipBias(1);
		}

		Texture->CompressionSettings = TC_Default;
		Texture->MipGenSettings = TMGS_SimpleAverage;
		Texture->bForceMipLevelsToBeResident = false;
		Texture->SetForceMipLevelsToBeResident(0.0f, false);
	}
	else
	{
		// PC / Console target: preserve high fidelity.
		Texture->SetMipBias(ConsoleMipBias);
		Texture->CompressionSettings = TC_Default;
		Texture->MipGenSettings = TMGS_SimpleAverage;
		Texture->bForceMipLevelsToBeResident = true;
	}
}

void UBM_TextureStreamerComponent::ApplyMobileBudgetSettings()
{
	for (UTexture2D* Texture : RegisteredTextures)
	{
		if (Texture == nullptr)
		{
			continue;
		}

		const bool bIsNormalMap = Texture->SRGB == false;
		const bool bIsRMA = Texture->SRGB == false;

		if (bIsNormalMap)
		{
			Texture->TextureGroup = TEXTUREGROUP_WorldNormalMap;
		}
		else if (bIsRMA)
		{
			Texture->TextureGroup = TEXTUREGROUP_World;
		}
		else
		{
			Texture->TextureGroup = TEXTUREGROUP_World;
		}

		Texture->SetMipBias(MobileMipBias);
		Texture->MipGenSettings = TMGS_SimpleAverage;
	}
}

void UBM_TextureStreamerComponent::UpdateRuntimeMipBudget()
{
	for (UTexture2D* Texture : RegisteredTextures)
	{
		if (Texture == nullptr)
		{
			continue;
		}

		ConfigureTextureForPlatform(Texture, Texture->TextureGroup);
	}
}

void UBM_TextureStreamerComponent::ClearRegisteredTextures()
{
	RegisteredTextures.Empty();
}

void UBM_TextureStreamerComponent::UpdateTextureEntry(UTexture2D* Texture, ETextureGroup Group)
{
	if (Texture == nullptr)
	{
		return;
	}

	Texture->TextureGroup = Group;
	ConfigureTextureForPlatform(Texture, Group);
}
