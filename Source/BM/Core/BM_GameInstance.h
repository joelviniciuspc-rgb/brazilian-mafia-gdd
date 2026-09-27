#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Containers/Deque.h"
#include "BM_GameInstance.generated.h"

class ABM_PlayerController;
class ABM_ClimateManager;
class ABM_WorldDirector;
class UBM_SaveGameData;

/**
 * EGameState
 * Enumeration representing the high-level game state
 */
UENUM(BlueprintType)
enum class EGameState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	MainMenu = 1 UMETA(DisplayName = "Main Menu"),
	Loading = 2 UMETA(DisplayName = "Loading"),
	Playing = 3 UMETA(DisplayName = "Playing"),
	Paused = 4 UMETA(DisplayName = "Paused"),
	Cutscene = 5 UMETA(DisplayName = "Cutscene"),
	ShuttingDown = 6 UMETA(DisplayName = "Shutting Down")
};

/**
 * FPlayerSaveData
 * Structure for persisting player progress across sessions
 */
USTRUCT(BlueprintType)
struct FPlayerSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float TotalPlayTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int64 CurrentMoney;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 CurrentMissionIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FVector LastPlayerLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString LastMapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 WantedLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TMap<FString, float> FactionReputations;
};

/**
 * UBM_GameInstance
 * 
 * Global game instance managing:
 * - Runtime initialization and shutdown
 * - Save game persistence across map streaming
 * - Asynchronous memory purging for platform scaling (PC/Console/Mobile)
 * - Cross-Save and Cross-Play network authentication
 * - Global gameplay state and configuration
 * - Asset streaming and memory optimization
 */
UCLASS()
class BRAZILIANMAFIA_API UBM_GameInstance : public UGameInstance
{
	GENERATED_BODY()

	friend class ABM_PlayerController;
	friend class ABM_ClimateManager;
	friend class ABM_WorldDirector;


public:
	UBM_GameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	// ========================================================================
	// Singleton Access
	// ========================================================================

	/**
	 * Get the global BM GameInstance.
	 * Safe to call from any thread (returns cached instance)
	 */
	USTATIC(BlueprintCallable, Category = "GameInstance")
	static UBM_GameInstance* GetBMGameInstance();

	// ========================================================================
	// Game State Management
	// ========================================================================

	/**
	 * Set the current game state
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState")
	void SetGameState(EGameState NewState);

	/**
	 * Get the current game state
	 */
	UFUNCTION(BlueprintCallable, Category = "GameState")
	EGameState GetGameState() const { return CurrentGameState; }

	// ========================================================================
	// Save Game System (Persistence)
	// ========================================================================

	/**
	 * Save the current game state to disk.
	 * Runs asynchronously to avoid frame hitches.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGame(const FString& SaveSlotName = TEXT("AutoSave"));

	/**
	 * Load a previously saved game.
	 * Triggers level streaming and asset loading.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadGame(const FString& SaveSlotName = TEXT("AutoSave"));

	/**
	 * Get the current player save data
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	FPlayerSaveData GetPlayerSaveData() const { return CurrentPlayerSaveData; }

	/**
	 * Set player save data (used after loading or updates)
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SetPlayerSaveData(const FPlayerSaveData& NewData);

	// ========================================================================
	// Memory Management (Platform Scaling)
	// ========================================================================

	/**
	 * Purge unused memory asynchronously.
	 * Called periodically to maintain target memory budgets.
	 * - PC: 8-16GB target
	 * - Console: 10-12GB target
	 * - Mobile: 2-3GB target
	 */
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void AsyncMemoryPurge(float TargetMemoryMB = 2048.0f);

	/**
	 * Get current memory usage in MB
	 */
	UFUNCTION(BlueprintCallable, Category = "Memory")
	float GetCurrentMemoryUsageMB() const;

	/**
	 * Get the target memory budget for the current platform
	 */
	UFUNCTION(BlueprintCallable, Category = "Memory")
	float GetPlatformMemoryBudgetMB() const;

	// ========================================================================
	// Network & Authentication
	// ========================================================================

	/**
	 * Authenticate player account (Cross-Save/Cross-Play)
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Auth")
	void AuthenticatePlayer(const FString& UserId, const FString& AuthToken);

	/**
	 * Get the authenticated player ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Auth")
	FString GetAuthenticatedPlayerId() const { return AuthenticatedPlayerId; }

	/**
	 * Check if player is authenticated
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Auth")
	bool IsPlayerAuthenticated() const { return !AuthenticatedPlayerId.IsEmpty(); }

	/**
	 * Sync player data to cloud (Cross-Save)
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Sync")
	void SyncPlayerDataToCloud();

	/**
	 * Load player data from cloud (Cross-Save)
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Sync")
	void LoadPlayerDataFromCloud();

	// ========================================================================
	// Global Managers Access
	// ========================================================================

	/**
	 * Get the climate manager (weather system)
	 */
	UFUNCTION(BlueprintCallable, Category = "Managers")
	ABM_ClimateManager* GetClimateManager() const { return CachedClimateManager; }

	/**
	 * Get the world director (global AI controller)
	 */
	UFUNCTION(BlueprintCallable, Category = "Managers")
	ABM_WorldDirector* GetWorldDirector() const { return CachedWorldDirector; }

	/**
	 * Get the player controller
	 */
	UFUNCTION(BlueprintCallable, Category = "Managers")
	ABM_PlayerController* GetBMPlayerController() const;

	// ========================================================================
	// Delegates & Events
	// ========================================================================

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, EGameState);
	FOnGameStateChanged OnGameStateChanged;

	DECLARE_MULTICAST_DELEGATE(FOnGameSaved);
	FOnGameSaved OnGameSaved;

	DECLARE_MULTICAST_DELEGATE(FOnGameLoaded);
	FOnGameLoaded OnGameLoaded;

	DECLARE_MULTICAST_DELEGATE(FOnPlayerAuthenticated);
	FOnPlayerAuthenticated OnPlayerAuthenticated;

	// ========================================================================
	// Platform Detection
	// ========================================================================

	/**
	 * Get the current platform type
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform")
	FString GetCurrentPlatformName() const;

	/**
	 * Check if running on Windows/PC
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform")
	bool IsRunningOnPC() const;

	/**
	 * Check if running on Console (PS5/Xbox)
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform")
	bool IsRunningOnConsole() const;

	/**
	 * Check if running on Mobile (Android/iOS)
	 */
	UFUNCTION(BlueprintCallable, Category = "Platform")
	bool IsRunningOnMobile() const;

protected:
	// ========================================================================
	// Game State
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EGameState CurrentGameState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FPlayerSaveData CurrentPlayerSaveData;

	// ========================================================================
	// Network
	// ========================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	FString AuthenticatedPlayerId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	FString AuthToken;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	bool bIsAuthenticated;

	// ========================================================================
	// Cached References
	// ========================================================================

	TObjectPtr<ABM_ClimateManager> CachedClimateManager;
	TObjectPtr<ABM_WorldDirector> CachedWorldDirector;

	// ========================================================================
	// Memory
	// ========================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|PC")
	float PCMemoryBudgetMB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Console")
	float ConsoleMemoryBudgetMB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory|Mobile")
	float MobileMemoryBudgetMB;

	float LastAsyncPurgeTime;

	// ========================================================================
	// Internal Methods
	// ========================================================================

private:
	void CacheGlobalManagers();
	void OnMapLoaded();
	void OnMapUnloaded();
};
