#include "BM/Core/BM_GameInstance.h"

#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/Platform.h"
#include "HAL/PlatformProcess.h"
#include "Misc/MemoryStats.h"
#include "Async/Async.h"
#include "BM/Climate/BM_ClimateManager.h"
#include "BM/World/BM_WorldDirector.h"
#include "BM/Character/BM_PlayerController.h"

static TObjectPtr<UBM_GameInstance> GlobalGameInstance = nullptr;

UBM_GameInstance::UBM_GameInstance()
	: CurrentGameState(EGameState::None)
	, bIsAuthenticated(false)
	, PCMemoryBudgetMB(12288.0f)
	, ConsoleMemoryBudgetMB(10240.0f)
	, MobileMemoryBudgetMB(2048.0f)
	, LastAsyncPurgeTime(0.0f)
{
}

void UBM_GameInstance::Init()
{
	Super::Init();

	// Register singleton
	if (GlobalGameInstance == nullptr)
	{
		GlobalGameInstance = this;
	}

	// Initialize player save data
	CurrentPlayerSaveData.PlayerName = TEXT("Player1");
	CurrentPlayerSaveData.TotalPlayTime = 0.0f;
	CurrentPlayerSaveData.CurrentMoney = 5000;
	CurrentPlayerSaveData.CurrentMissionIndex = 0;
	CurrentPlayerSaveData.LastMapName = TEXT("SP_MainMenu");
	CurrentPlayerSaveData.WantedLevel = 0;

	SetGameState(EGameState::MainMenu);

	UE_LOG(LogTemp, Warning, TEXT("[BM_GameInstance] Initialized on platform: %s"), *GetCurrentPlatformName());
}

void UBM_GameInstance::Shutdown()
{
	SetGameState(EGameState::ShuttingDown);

	// Save any unsaved progress
	SaveGame(TEXT("AutoSave"));

	// Clean up references
	CachedClimateManager = nullptr;
	CachedWorldDirector = nullptr;

	if (GlobalGameInstance == this)
	{
		GlobalGameInstance = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("[BM_GameInstance] Shutdown complete"));

	Super::Shutdown();
}

UBM_GameInstance* UBM_GameInstance::GetBMGameInstance()
{
	return GlobalGameInstance;
}

void UBM_GameInstance::SetGameState(EGameState NewState)
{
	if (CurrentGameState == NewState)
	{
		return;
	}

	EGameState OldState = CurrentGameState;
	CurrentGameState = NewState;

	UE_LOG(LogTemp, Log, TEXT("[BM_GameInstance] Game state changed: %d -> %d"), (int32)OldState, (int32)NewState);

	OnGameStateChanged.Broadcast(NewState);
}

void UBM_GameInstance::SaveGame(const FString& SaveSlotName)
{
	if (CurrentGameState == EGameState::ShuttingDown)
	{
		return;
	}

	// Update play time
	if (UWorld* World = GetWorld())
	{
		CurrentPlayerSaveData.TotalPlayTime += World->DeltaTimeSeconds;
	}

	// Asynchronously save to disk
	AsyncTask(ENamedThreads::BackgroundThread, [this, SaveSlotName]()
	{
		// Simulate save operation (in real game, use UGameplayStatics::SaveGameToSlot)
		FString SavePath = FPaths::ProjectSavedDir() / SaveSlotName + TEXT(".bmsave");
		UE_LOG(LogTemp, Log, TEXT("[BM_GameInstance] Saving game to: %s"), *SavePath);
	});

	OnGameSaved.Broadcast();
}

void UBM_GameInstance::LoadGame(const FString& SaveSlotName)
{
	FString SavePath = FPaths::ProjectSavedDir() / SaveSlotName + TEXT(".bmsave");
	UE_LOG(LogTemp, Log, TEXT("[BM_GameInstance] Loading game from: %s"), *SavePath);

	// In a real implementation, load from disk and restore player data
	SetGameState(EGameState::Loading);
	// ... load logic ...
	SetGameState(EGameState::Playing);

	OnGameLoaded.Broadcast();
}

void UBM_GameInstance::SetPlayerSaveData(const FPlayerSaveData& NewData)
{
	CurrentPlayerSaveData = NewData;
	UE_LOG(LogTemp, Log, TEXT("[BM_GameInstance] Player save data updated: %s (Money: %lld)"), 
		*NewData.PlayerName, NewData.CurrentMoney);
}

void UBM_GameInstance::AsyncMemoryPurge(float TargetMemoryMB)
{
	if (UWorld* World = GetWorld())
	{
		if (World->TimeSinceCreation - LastAsyncPurgeTime < 30.0f)
		{
			return; // Throttle purges to every 30 seconds
		}
	}

	AsyncTask(ENamedThreads::BackgroundThread, [this, TargetMemoryMB]()
	{
		float CurrentMemory = GetCurrentMemoryUsageMB();

		if (CurrentMemory > TargetMemoryMB)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BM_GameInstance] Memory purge triggered. Current: %.1f MB, Target: %.1f MB"),
				CurrentMemory, TargetMemoryMB);

			// Force garbage collection
			GC.CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		}
	});

	if (UWorld* World = GetWorld())
	{
		LastAsyncPurgeTime = World->TimeSinceCreation;
	}
}

float UBM_GameInstance::GetCurrentMemoryUsageMB() const
{
	// Platform-specific memory query
#if PLATFORM_WINDOWS
	float ProcessMemoryMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
#elif PLATFORM_PS5 || PLATFORM_XBOXONE
	float ProcessMemoryMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
#elif PLATFORM_ANDROID || PLATFORM_IOS
	float ProcessMemoryMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
#else
	float ProcessMemoryMB = 0.0f;
#endif

	return ProcessMemoryMB;
}

float UBM_GameInstance::GetPlatformMemoryBudgetMB() const
{
	if (IsRunningOnPC())
	{
		return PCMemoryBudgetMB;
	}
	else if (IsRunningOnConsole())
	{
		return ConsoleMemoryBudgetMB;
	}
	else if (IsRunningOnMobile())
	{
		return MobileMemoryBudgetMB;
	}

	return PCMemoryBudgetMB;
}

void UBM_GameInstance::AuthenticatePlayer(const FString& UserId, const FString& InAuthToken)
{
	AuthenticatedPlayerId = UserId;
	AuthToken = InAuthToken;
	bIsAuthenticated = !UserId.IsEmpty();

	UE_LOG(LogTemp, Log, TEXT("[BM_GameInstance] Player authenticated: %s"), *UserId);

	OnPlayerAuthenticated.Broadcast();
}

void UBM_GameInstance::SyncPlayerDataToCloud()
{
	if (!IsPlayerAuthenticated())
	{
		UE_LOG(LogTemp, Error, TEXT("[BM_GameInstance] Cannot sync: Player not authenticated"));
		return;
	}

	AsyncTask(ENamedThreads::BackgroundThread, [this]()
	{
		UE_LOG(LogTemp, Log, TEXT("[BM_GameInstance] Syncing player data to cloud for user: %s"), *AuthenticatedPlayerId);
		// Implement cloud sync logic here
	});
}

void UBM_GameInstance::LoadPlayerDataFromCloud()
{
	if (!IsPlayerAuthenticated())
	{
		UE_LOG(LogTemp, Error, TEXT("[BM_GameInstance] Cannot load: Player not authenticated"));
		return;
	}

	AsyncTask(ENamedThreads::BackgroundThread, [this]()
	{
		UE_LOG(LogTemp, Log, TEXT("[BM_GameInstance] Loading player data from cloud for user: %s"), *AuthenticatedPlayerId);
		// Implement cloud load logic here
	});
}

ABM_PlayerController* UBM_GameInstance::GetBMPlayerController() const
{
	if (UWorld* World = GetWorld())
	{
		return Cast<ABM_PlayerController>(GEngine->GetFirstLocalPlayerController(World));
	}
	return nullptr;
}

FString UBM_GameInstance::GetCurrentPlatformName() const
{
#if PLATFORM_WINDOWS
	return TEXT("Windows");
#elif PLATFORM_PS5
	return TEXT("PlayStation 5");
#elif PLATFORM_XBOXONE || PLATFORM_XSX
	return TEXT("Xbox Series X/S");
#elif PLATFORM_ANDROID
	return TEXT("Android");
#elif PLATFORM_IOS
	return TEXT("iOS");
#else
	return TEXT("Unknown");
#endif
}

bool UBM_GameInstance::IsRunningOnPC() const
{
#if PLATFORM_WINDOWS
	return true;
#else
	return false;
#endif
}

bool UBM_GameInstance::IsRunningOnConsole() const
{
#if PLATFORM_PS5 || PLATFORM_XBOXONE || PLATFORM_XSX
	return true;
#else
	return false;
#endif
}

bool UBM_GameInstance::IsRunningOnMobile() const
{
#if PLATFORM_ANDROID || PLATFORM_IOS
	return true;
#else
	return false;
#endif
}
