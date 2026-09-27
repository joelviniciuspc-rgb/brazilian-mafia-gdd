using System;
using System.Collections;
using System.Threading;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.SceneManagement;

namespace BrazilianMafia.Core
{
    /// <summary>Persistent runtime executive for economy, wanted state, authentication and memory maintenance.</summary>
    public sealed class BM_GameManager : MonoBehaviour
    {
        public static BM_GameManager Instance { get; private set; }

        [Header("Runtime state")]
        [SerializeField] private long playerMoney;
        [SerializeField, Range(0, 5)] private int wantedLevel;
        [SerializeField] private string authenticatedUserId;
        public long PlayerMoney => Interlocked.Read(ref playerMoney);
        public int WantedLevel { get { lock (stateLock) return wantedLevel; } }
        public bool IsAuthenticated { get { lock (stateLock) return !string.IsNullOrEmpty(authenticatedUserId); } }

        public event Action<long> MoneyChanged;
        public event Action<int> WantedLevelChanged;
        public event Action<bool> AuthenticationChanged;

        private readonly object stateLock = new object();
        private CancellationTokenSource lifetimeCts;
        private Coroutine purgeRoutine;

        [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
        private static void Bootstrap() { if (Instance == null) new GameObject(nameof(BM_GameManager)).AddComponent<BM_GameManager>(); }

        private void Awake()
        {
            if (Instance != null && Instance != this) { Destroy(gameObject); return; }
            Instance = this;
            DontDestroyOnLoad(gameObject);
            lifetimeCts = new CancellationTokenSource();
            SceneManager.sceneLoaded += OnSceneLoaded;
        }

        private void Start() { purgeRoutine = StartCoroutine(MemoryMaintenance()); }

        public void AddMoney(long amount)
        {
            long value = Interlocked.Add(ref playerMoney, amount);
            MoneyChanged?.Invoke(value);
        }

        public void TriggerCrime(int severity)
        {
            lock (stateLock) wantedLevel = Mathf.Clamp(wantedLevel + Mathf.Max(1, severity), 0, 5);
            WantedLevelChanged?.Invoke(WantedLevel);
        }

        public void ClearWanted(int amount = 1)
        {
            lock (stateLock) wantedLevel = Mathf.Clamp(wantedLevel - Mathf.Max(1, amount), 0, 5);
            WantedLevelChanged?.Invoke(WantedLevel);
        }

        /// <summary>Replace with the platform backend (EOS, Steam, PlayFab, etc.).</summary>
        public async Task<bool> AuthenticateAsync(string userId, string token, CancellationToken cancellationToken = default)
        {
            await Task.Delay(1, cancellationToken).ConfigureAwait(false);
            bool valid = !string.IsNullOrWhiteSpace(userId) && !string.IsNullOrWhiteSpace(token);
            lock (stateLock) authenticatedUserId = valid ? userId : null;
            MainThread(() => AuthenticationChanged?.Invoke(valid));
            return valid;
        }

        public async Task PurgeUnusedAssetsAsync(CancellationToken cancellationToken = default)
        {
            await AwaitOperation(ResourceUnloadOperation(), cancellationToken).ConfigureAwait(false);
        }

        private IEnumerator ResourceUnloadOperation()
        {
            yield return Resources.UnloadUnusedAssets();
            GC.Collect();
        }

        private async Task AwaitOperation(IEnumerator operation, CancellationToken token)
        {
            var completion = new TaskCompletionSource<bool>();
            MainThread(() => StartCoroutine(RunOperation(operation, completion)));
            using (token.Register(() => completion.TrySetCanceled(token))) await completion.Task.ConfigureAwait(false);
        }

        private IEnumerator RunOperation(IEnumerator operation, TaskCompletionSource<bool> completion)
        {
            yield return operation;
            completion.TrySetResult(true);
        }

        private IEnumerator MemoryMaintenance()
        {
            var wait = new WaitForSecondsRealtime(30f);
            while (true) { yield return wait; if (Application.isFocused) _ = PurgeUnusedAssetsAsync(lifetimeCts.Token); }
        }

        private void OnSceneLoaded(Scene scene, LoadSceneMode mode) { Resources.UnloadUnusedAssets(); }
        private void MainThread(Action action) { if (this != null) action(); }
        private void OnDestroy() { SceneManager.sceneLoaded -= OnSceneLoaded; lifetimeCts?.Cancel(); lifetimeCts?.Dispose(); if (Instance == this) Instance = null; }
    }
}
