# Unity Migration Notes

The Unreal-oriented prototype has been migrated into a Unity URP-oriented C# foundation.

## New runtime modules

- `BM_GameManager`: persistent singleton, money, wanted level, authentication seam, async unused-asset purge.
- `BM_PlayerController`: movement, sprint and smooth physical-character switching.
- `BM_VehicleController`: WheelCollider driving, low-rider/motorcycle tuning, Grau wheelie, damage and PBR mask hooks.
- `BM_WeatherDirector`: URP global shader-property equivalent of an MPC climate driver.

## Setup

1. Create a Unity 2022.3 LTS or Unity 6 URP project.
2. Copy `Unity/Assets/Scripts/BM` into `Assets/Scripts/BM`.
3. Add a `CharacterController` to the player and configure character slots.
4. Add `Rigidbody`, `WheelCollider` objects and wheel visuals to vehicles.
5. Add shader properties `_GlobalWetness`, `_PuddleDepth`, `_VerticalWaterStreaks`, `_MudSpatterIntensity`, `_DamageMask`, `_RustMask` and `_WindshieldShatter` to URP master materials.
6. Replace the authentication stub with the selected backend (Unity Gaming Services, EOS, Steam, or platform services).
7. Use Addressables for production map chunks; `Resources.UnloadUnusedAssets` is retained as a safe baseline fallback.

## Performance constraints

- Never mutate UnityEngine objects from worker threads; async tasks return to the main thread before touching scene state.
- Prefer Addressables labels per region and quality tier for the 2–3 GB mobile target.
- Use `MaterialPropertyBlock` rather than creating a material instance per vehicle.
- Use additive scene streaming or Addressables for Rio, São Paulo and Campo Grande sectors.
- Keep physics and wheel updates in `FixedUpdate` and cap active vehicle simulation by distance.
