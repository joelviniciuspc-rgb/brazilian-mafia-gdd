using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.AddressableAssets;
using UnityEngine.ResourceManagement.AsyncOperations;
using UnityEngine.U2D;

namespace BrazilianMafia.AssetManagement
{
    /// <summary>
    /// Asset type classification for streaming prioritization and memory budgeting.
    /// </summary>
    public enum AssetCategory : byte
    {
        BuildingMesh = 0,
        VehicleModel = 1,
        WeaponModel = 2,
        PedestrianMesh = 3,
        PropObject = 4,
        TextureAlbedo = 5,
        TextureNormal = 6,
        TextureRoughness = 7,
        UIAtlas = 8,
        AudioClip = 9
    }

    /// <summary>
    /// Serialized registry entry for a single asset.
    /// </summary>
    [System.Serializable]
    public sealed class AssetEntry
    {
        public int id;
        public string addressableKey;
        public AssetCategory category;
        public string regionTag; // "RIO", "SP", "CG"
        public float memoryMB;
        public bool keepLoaded;

        public AssetEntry() { }
        public AssetEntry(int id, string key, AssetCategory cat, string region, float mem, bool keep)
        {
            this.id = id;
            this.addressableKey = key;
            this.category = cat;
            this.regionTag = region;
            this.memoryMB = mem;
            this.keepLoaded = keep;
        }
    }

    /// <summary>
    /// Master registry: indexed 3000+ assets mapped by category, region, and memory budget.
    /// Exposed as a ScriptableObject for authoring and runtime reference.
    /// </summary>
    [CreateAssetMenu(fileName = "BM_AssetRegistry", menuName = "BrazilianMafia/Asset Registry")]
    public sealed class BM_AssetRegistry : ScriptableObject
    {
        [SerializeField] private List<AssetEntry> allAssets = new();
        [SerializeField, Range(512, 4096)] private float totalMemoryBudgetMB = 2048f;

        private Dictionary<int, AssetEntry> idLookup;
        private Dictionary<AssetCategory, List<AssetEntry>> categoryLookup;
        private Dictionary<string, List<AssetEntry>> regionLookup;

        public float TotalBudget => totalMemoryBudgetMB;
        public int AssetCount => allAssets.Count;
        public IReadOnlyList<AssetEntry> AllAssets => allAssets.AsReadOnly();

        private void OnEnable() => RebuildIndices();

        /// <summary>
        /// Rebuild all lookup dictionaries for fast runtime access.
        /// </summary>
        private void RebuildIndices()
        {
            idLookup = new Dictionary<int, AssetEntry>(allAssets.Count);
            categoryLookup = new Dictionary<AssetCategory, List<AssetEntry>>();
            regionLookup = new Dictionary<string, List<AssetEntry>>();

            foreach (var cat in Enum.GetValues(typeof(AssetCategory)).Cast<AssetCategory>())
                categoryLookup[cat] = new List<AssetEntry>();

            foreach (var asset in allAssets)
            {
                idLookup[asset.id] = asset;
                if (categoryLookup.ContainsKey(asset.category))
                    categoryLookup[asset.category].Add(asset);
                if (!regionLookup.ContainsKey(asset.regionTag))
                    regionLookup[asset.regionTag] = new List<AssetEntry>();
                regionLookup[asset.regionTag].Add(asset);
            }
        }

        /// <summary>
        /// Get a single asset by its unique ID.
        /// </summary>
        public bool TryGetAsset(int id, out AssetEntry asset) => idLookup.TryGetValue(id, out asset);

        /// <summary>
        /// Get all assets in a specific category.
        /// </summary>
        public IReadOnlyList<AssetEntry> GetAssetsByCategory(AssetCategory category)
        {
            return categoryLookup.TryGetValue(category, out var list) ? list.AsReadOnly() : new List<AssetEntry>().AsReadOnly();
        }

        /// <summary>
        /// Get all assets in a specific region (Rio, SP, CG).
        /// </summary>
        public IReadOnlyList<AssetEntry> GetAssetsByRegion(string region)
        {
            return regionLookup.TryGetValue(region, out var list) ? list.AsReadOnly() : new List<AssetEntry>().AsReadOnly();
        }

        /// <summary>
        /// Calculate total memory for a given set of assets.
        /// </summary>
        public float CalculateTotalMemory(IEnumerable<AssetEntry> assets) => assets.Sum(a => a.memoryMB);

#if UNITY_EDITOR
        /// <summary>
        /// Add an asset entry to the registry (editor only).
        /// </summary>
        public void AddAsset(AssetEntry entry)
        {
            if (allAssets.Any(a => a.id == entry.id)) return;
            allAssets.Add(entry);
            RebuildIndices();
            UnityEditor.EditorUtility.SetDirty(this);
        }

        /// <summary>
        /// Validate the registry for duplicate IDs and memory budget violations.
        /// </summary>
        public void ValidateRegistry(out List<string> errors)
        {
            errors = new List<string>();
            var ids = new HashSet<int>();
            float totalMem = 0f;

            foreach (var asset in allAssets)
            {
                if (ids.Contains(asset.id))
                    errors.Add($"Duplicate asset ID: {asset.id}");
                ids.Add(asset.id);
                totalMem += asset.memoryMB;

                if (string.IsNullOrEmpty(asset.addressableKey))
                    errors.Add($"Asset {asset.id} missing Addressable key");
            }

            if (totalMem > totalMemoryBudgetMB * 1.5f)
                errors.Add($"Total registered memory ({totalMem}MB) exceeds 150% of budget ({totalMemoryBudgetMB}MB)");
        }
#endif
    }

    /// <summary>
    /// Manages asynchronous loading/unloading and caching of assets indexed in the registry.
    /// Supports region-based streaming and memory budgeting.
    /// </summary>
    public sealed class BM_AssetLoader : MonoBehaviour
    {
        [SerializeField] private BM_AssetRegistry registry;
        [SerializeField, Range(0.5f, 5f)] private float loadTimeoutSeconds = 3f;

        private Dictionary<int, AsyncOperationHandle<UnityEngine.Object>> activeLoads;
        private Dictionary<int, UnityEngine.Object> cachedAssets;
        private HashSet<int> markedForUnload;

        public static BM_AssetLoader Instance { get; private set; }
        public event Action<int, UnityEngine.Object> AssetLoaded;
        public event Action<int> AssetUnloaded;

        private void Awake()
        {
            if (Instance != null && Instance != this) { Destroy(gameObject); return; }
            Instance = this;
            DontDestroyOnLoad(gameObject);
            activeLoads = new Dictionary<int, AsyncOperationHandle<UnityEngine.Object>>();
            cachedAssets = new Dictionary<int, UnityEngine.Object>();
            markedForUnload = new HashSet<int>();
        }

        /// <summary>
        /// Asynchronously load a single asset by ID. Caches the result.
        /// </summary>
        public async Task<T> LoadAssetAsync<T>(int assetId) where T : UnityEngine.Object
        {
            if (registry == null) return null;
            if (!registry.TryGetAsset(assetId, out var entry)) return null;
            if (cachedAssets.TryGetValue(assetId, out var cached)) return cached as T;

            var handle = Addressables.LoadAssetAsync<T>(entry.addressableKey);
            activeLoads[assetId] = handle.Convert<UnityEngine.Object>();

            try
            {
                var result = await handle.Task;
                cachedAssets[assetId] = result;
                AssetLoaded?.Invoke(assetId, result);
                return result;
            }
            finally
            {
                activeLoads.Remove(assetId);
            }
        }

        /// <summary>
        /// Batch load all assets in a region for streaming.
        /// </summary>
        public async Task<List<UnityEngine.Object>> LoadRegionAsync(string region)
        {
            if (registry == null) return new List<UnityEngine.Object>();
            var assets = registry.GetAssetsByRegion(region);
            var tasks = assets.Where(a => !cachedAssets.ContainsKey(a.id))
                .Select(a => LoadAssetAsync<UnityEngine.Object>(a.id))
                .ToList();

            if (tasks.Count == 0) return new List<UnityEngine.Object>(cachedAssets.Values);
            await Task.WhenAll(tasks);
            return tasks.Select(t => t.Result).Where(o => o != null).ToList();
        }

        /// <summary>
        /// Unload a single cached asset and release its memory.
        /// </summary>
        public void UnloadAsset(int assetId)
        {
            if (activeLoads.TryGetValue(assetId, out var handle))
                Addressables.Release(handle);
            cachedAssets.Remove(assetId);
            AssetUnloaded?.Invoke(assetId);
        }

        /// <summary>
        /// Unload all assets not marked as keepLoaded.
        /// </summary>
        public void PurgeNonPersistentAssets()
        {
            var toRemove = new List<int>();
            foreach (var kvp in cachedAssets)
            {
                if (registry.TryGetAsset(kvp.Key, out var entry) && !entry.keepLoaded)
                    toRemove.Add(kvp.Key);
            }
            foreach (int id in toRemove) UnloadAsset(id);
        }

        private void OnDestroy()
        {
            foreach (var handle in activeLoads.Values) Addressables.Release(handle);
            activeLoads.Clear();
            cachedAssets.Clear();
            if (Instance == this) Instance = null;
        }
    }
}
