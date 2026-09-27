using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.AddressableAssets;

namespace BrazilianMafia.World
{
    /// <summary>
    /// Grid-based sector for dynamic map streaming.
    /// Each sector is 500x500 units and independently manages its loaded assets.
    /// </summary>
    [System.Serializable]
    public sealed class MapSector
    {
        public int x, z; // Grid coordinates
        public Vector3 center => new Vector3(x * 500f, 0, z * 500f);
        public Bounds bounds => new Bounds(center, Vector3.one * 500f);
        public string regionTag;
        public List<int> assetIds = new();
        public bool isLoaded;
        public float lastAccessTime;
    }

    /// <summary>
    /// Manages dynamic streaming of the massive world map across Rio, SP, and Campo Grande.
    /// Divides the map into 500x500 unit grid sectors and loads/unloads based on player proximity.
    /// </summary>
    public sealed class BM_MapStreamer : MonoBehaviour
    {
        [SerializeField] private BM_AssetRegistry.BM_AssetRegistry registry;
        [SerializeField] private Transform playerTransform;
        [SerializeField, Range(1, 4)] private int loadRadius = 2; // Load 2 sectors in each direction
        [SerializeField, Range(0.1f, 10f)] private float updateIntervalSeconds = 0.5f;
        [SerializeField, Range(10, 300)] private float memoryCleanupIntervalSeconds = 60f;

        private Dictionary<Vector2Int, MapSector> sectorGrid = new();
        private HashSet<Vector2Int> activeLoadedSectors = new();
        private float lastUpdateTime, lastCleanupTime;
        private Vector2Int lastPlayerSector = Vector2Int.zero;

        public event Action<MapSector> SectorLoaded;
        public event Action<MapSector> SectorUnloaded;
        public IReadOnlyDictionary<Vector2Int, MapSector> Sectors => sectorGrid;

        private void Awake() { if (playerTransform == null) playerTransform = transform; }

        private void Update()
        {
            if (Time.realtimeSinceStartup - lastUpdateTime < updateIntervalSeconds) return;
            lastUpdateTime = Time.realtimeSinceStartup;

            Vector2Int playerSector = GetSectorCoords(playerTransform.position);
            if (playerSector != lastPlayerSector) UpdateStreamingZone(playerSector);
            lastPlayerSector = playerSector;

            if (Time.realtimeSinceStartup - lastCleanupTime > memoryCleanupIntervalSeconds)
            {
                lastCleanupTime = Time.realtimeSinceStartup;
                PruneUnusedSectors();
            }
        }

        /// <summary>
        /// Get grid sector coordinates from world position.
        /// </summary>
        private Vector2Int GetSectorCoords(Vector3 worldPos) => new Vector2Int(Mathf.FloorToInt(worldPos.x / 500f), Mathf.FloorToInt(worldPos.z / 500f));

        /// <summary>
        /// Update the active loading zone around the player.
        /// </summary>
        private async void UpdateStreamingZone(Vector2Int playerSector)
        {
            var newActiveSet = new HashSet<Vector2Int>();
            for (int x = -loadRadius; x <= loadRadius; x++)
            {
                for (int z = -loadRadius; z <= loadRadius; z++)
                {
                    newActiveSet.Add(playerSector + new Vector2Int(x, z));
                }
            }

            var toLoad = newActiveSet.Except(activeLoadedSectors).ToList();
            var toUnload = activeLoadedSectors.Except(newActiveSet).ToList();

            foreach (var coord in toLoad) _ = LoadSectorAsync(coord);
            foreach (var coord in toUnload) UnloadSector(coord);

            activeLoadedSectors = newActiveSet;
        }

        /// <summary>
        /// Asynchronously load all assets in a sector.
        /// </summary>
        private async Task LoadSectorAsync(Vector2Int coord)
        {
            if (activeLoadedSectors.Contains(coord) && sectorGrid.TryGetValue(coord, out var sector) && sector.isLoaded) return;
            if (!sectorGrid.TryGetValue(coord, out sector)) sector = CreateSector(coord);

            sector.isLoaded = false;
            var loader = BrazilianMafia.AssetManagement.BM_AssetLoader.Instance;
            if (loader == null) return;

            try
            {
                var tasks = sector.assetIds.Select(id => loader.LoadAssetAsync<UnityEngine.Object>(id)).ToList();
                await Task.WhenAll(tasks);
                sector.isLoaded = true;
                sector.lastAccessTime = Time.realtimeSinceStartup;
                SectorLoaded?.Invoke(sector);
                Debug.Log($"Sector {coord} loaded. Assets: {sector.assetIds.Count}");
            }
            catch (Exception ex)
            {
                Debug.LogError($"Failed to load sector {coord}: {ex.Message}");
            }
        }

        /// <summary>
        /// Unload a sector and release its assets.
        /// </summary>
        private void UnloadSector(Vector2Int coord)
        {
            if (!sectorGrid.TryGetValue(coord, out var sector)) return;
            var loader = BrazilianMafia.AssetManagement.BM_AssetLoader.Instance;
            if (loader != null)
            {
                foreach (int assetId in sector.assetIds) loader.UnloadAsset(assetId);
            }
            sector.isLoaded = false;
            SectorUnloaded?.Invoke(sector);
            Debug.Log($"Sector {coord} unloaded");
        }

        /// <summary>
        /// Create a new sector and populate it with region-appropriate assets.
        /// </summary>
        private MapSector CreateSector(Vector2Int coord)
        {
            if (sectorGrid.TryGetValue(coord, out var existing)) return existing;
            var sector = new MapSector { x = coord.x, z = coord.y };
            DetermineSectorRegion(sector);
            PopulateSectorAssets(sector);
            sectorGrid[coord] = sector;
            return sector;
        }

        /// <summary>
        /// Assign region tag to sector based on grid position.
        /// </summary>
        private void DetermineSectorRegion(MapSector sector)
        {
            if (sector.x < -5) sector.regionTag = "RIO";
            else if (sector.x > 5) sector.regionTag = "CG";
            else sector.regionTag = "SP";
        }

        /// <summary>
        /// Populate sector with region-appropriate assets from registry.
        /// </summary>
        private void PopulateSectorAssets(MapSector sector)
        {
            if (registry == null) return;
            var regionalAssets = registry.GetAssetsByRegion(sector.regionTag);
            sector.assetIds = regionalAssets.Take(50).Select(a => a.id).ToList();
        }

        /// <summary>
        /// Periodically clean up sectors that haven't been accessed recently.
        /// </summary>
        private void PruneUnusedSectors()
        {
            var toRemove = new List<Vector2Int>();
            float threshold = Time.realtimeSinceStartup - 300f; // 5 minutes
            foreach (var kvp in sectorGrid)
            {
                if (!activeLoadedSectors.Contains(kvp.Key) && kvp.Value.lastAccessTime < threshold)
                    toRemove.Add(kvp.Key);
            }
            foreach (var coord in toRemove) sectorGrid.Remove(coord);
        }
    }
}
