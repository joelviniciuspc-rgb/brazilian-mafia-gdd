# Asset Management & Streaming Architecture

## Overview

This framework manages a 3,000+ asset library across the Brazilian Mafia world map (Rio, SP, Campo Grande) with zero garbage collection overhead and optimal VRAM usage within a 2-3GB budget.

## Core Systems

### 1. **BM_AssetRegistry.cs** (3,000 asset database)

- **ScriptableObject-based master registry** storing metadata for all 3,000+ assets
- **Fast lookup dictionaries** indexed by:
  - Asset ID (unique integer)
  - Category (Buildings, Vehicles, Textures, UI, Audio)
  - Region (Rio, São Paulo, Campo Grande)
- **Memory budgeting**: tracks total allocated memory and enforces hard ceiling
- **BM_AssetLoader**: async Addressables integration with caching
  - `LoadAssetAsync<T>(int id)` – loads by ID, caches result
  - `LoadRegionAsync(string region)` – batch loads all regional assets
  - `PurgeNonPersistentAssets()` – unloads temporary content

### 2. **BM_MapStreamer.cs** (Dynamic world streaming)

- **Grid-based sector system**: 500x500 unit sectors with independent asset lifecycles
- **Proximity-based loading**: loads 2 sectors in each direction around player
- **Regional assignment**: automatically tags sectors (Rio, SP, CG) based on grid position
- **Memory cleanup**: periodic pruning of unused sectors after 5 minutes
- **Events**: `SectorLoaded` and `SectorUnloaded` for gameplay reactions

### 3. **BM_GlobalShaderDirector.cs** (URP environmental effects)

- **Zero per-material overhead**: all effects driven by global shader properties
- **Unified weather system**:
  - `_GlobalWetness` (0–1)
  - `_PuddleDepth` (0–1)
  - `_CampoGrandeMudIntensity` (0–1)
  - `_VerticalWaterStreaks` (0–1)
- **Smooth transitions**: AnimationCurve-driven lerping over configurable duration
- **Regional effects**:
  - `TriggerStorm()` – tropical rain in Rio/SP
  - `ApplyCampoGrandeDust()` – dusty frontier conditions
  - `ClearWeather()` – dry sunny day

### 4. **BM_MapUI.cs & BM_HUDController.cs** (Interactive map & HUD)

- **Real-time player markers**: display Alemão, Marcola, Pampa positions on 2D overlay
- **Safehouse markers**: persist across gameplay sessions
- **Police zone circles**: dynamic visualization of wanted radius based on level
- **Master HUD integration**:
  - Money display (linked to GameManager economy)
  - Wanted level indicator with color coding
  - Mission objectives and vehicle health
  - Minimap toggle

## Performance Targets

| Target | Budget |
|--------|--------|
| **PC VRAM** | 8–16 GB |
| **PS5/Xbox** | 10–12 GB |
| **Mobile** | 2–3 GB |
| **Active sectors** | 3×3 grid (9 sectors) |
| **Frame cost** | Sector updates: <1ms |
| **GC allocations** | 0 (zero allocation updates after initialization) |

## Setup Instructions

### 1. Create Asset Registry

```csharp
var registry = ScriptableObject.CreateInstance<BM_AssetRegistry>();
for (int i = 0; i < 3000; i++)
{
    var entry = new AssetEntry(
        id: i,
        key: $"Assets/Models/Building_{i}",
        category: (AssetCategory)(i % 10),
        region: i % 3 == 0 ? "RIO" : i % 3 == 1 ? "SP" : "CG",
        mem: Random.Range(0.5f, 5f),
        keep: i < 100 // Keep first 100 core assets always loaded
    );
    registry.AddAsset(entry);
}
AssetDatabase.CreateAsset(registry, "Assets/Resources/BM_AssetRegistry.asset");
```

### 2. Configure Addressables

- Label all assets with region tags: `Rio`, `SP`, `CG`
- Use Addressables `BuildPath: "[BuildTarget]/[GroupName]"` for platform-specific streaming
- Enable compression for textures and models

### 3. Assign Shader Properties

Add to your URP Master Material shader:

```glsl
Shader "Custom/BM_URP_Master"
{
    Properties
    {
        _GlobalWetness("Wetness", Range(0, 1)) = 0
        _PuddleDepth("Puddle Depth", Range(0, 1)) = 0
        _CampoGrandeMudIntensity("Mud Intensity", Range(0, 1)) = 0
        _VerticalWaterStreaks("Water Streaks", Range(0, 1)) = 0
    }
    HLSLINCLUDE
    float _GlobalWetness;
    float _PuddleDepth;
    float _CampoGrandeMudIntensity;
    float _VerticalWaterStreaks;
    ENDHLSL
}
```

### 4. Scene Setup

1. Create an empty GameObject: `GameManager`
   - Attach: `BM_GameManager`, `BM_AssetLoader`

2. Create an empty GameObject: `WorldManagement`
   - Attach: `BM_MapStreamer`, `BM_GlobalShaderDirector`

3. Create Canvas for HUD:
   - Attach: `BM_MapUI`, `BM_HUDController`
   - Configure marker prefabs and world bounds

## Usage Example

```csharp
// Transition to rain
BM_GlobalShaderDirector.Instance.SetWeatherState(
    wetness: 0.9f,
    puddles: 0.8f,
    mud: 0.3f,
    streaks: 0.95f
);

// Load a region asynchronously
await BM_AssetLoader.Instance.LoadRegionAsync("RIO");

// Update HUD
BM_HUDController.Instance.SetMissionObjective("Steal the armored truck");
BM_HUDController.Instance.SetVehicleHealth(0.65f);
```

## Optimization Notes

- **No GC allocations in update loop**: all data structures preallocated at startup
- **Material Property Blocks**: vehicle/building damage uses MPB instead of material instances
- **Texture streaming**: only highest-quality 2K PBR textures streamed in active sectors
- **Addressables labels**: enable fine-grained quality-tier selection per region
- **Sector pruning**: automatically unloads sectors unused for >5 minutes

## Mobile Optimization (2–3 GB target)

- Limit active sectors to 1 (player sector only) on mobile
- Use Addressables quality tier: stream 1K textures instead of 2K
- Disable real-time puddles and water streaks on mobile
- Use single-channel normal maps for mobile vehicles
