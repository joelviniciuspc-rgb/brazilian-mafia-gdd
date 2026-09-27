using UnityEngine;

namespace BrazilianMafia.Vehicles
{
    /// <summary>WheelCollider vehicle with low-rider tuning and motorcycle Grau support.</summary>
    public sealed class BM_VehicleController : MonoBehaviour
    {
        [System.Serializable] public struct Wheel { public WheelCollider collider; public Transform visual; public bool steer; public bool drive; }
        [SerializeField] private Rigidbody body; [SerializeField] private Wheel[] wheels; [SerializeField] private bool motorcycle;
        [SerializeField] private float motorTorque = 1500f, brakeTorque = 3500f, steeringAngle = 32f, wheelieTorque = 180f;
        [SerializeField] private Transform damageMesh; [SerializeField] private Renderer[] renderers; [SerializeField] private float mudIntensity, rustAmount; [SerializeField] private Texture2D mudMask, rustMask, shatteredGlass;
        private MaterialPropertyBlock block; private bool wheelie;
        private static readonly int Mud = Shader.PropertyToID("_MudSpatterIntensity"), Rust = Shader.PropertyToID("_RustMask"), Damage = Shader.PropertyToID("_DamageMask"), Glass = Shader.PropertyToID("_WindshieldShatter");
        private void Awake() { body ??= GetComponent<Rigidbody>(); block = new MaterialPropertyBlock(); }
        private void FixedUpdate()
        {
            float throttle = Input.GetAxisRaw("Vertical"), steer = Input.GetAxisRaw("Horizontal"); bool brake = Input.GetKey(KeyCode.Space);
            foreach (Wheel w in wheels) { if (w.collider == null) continue; if (w.steer) w.collider.steerAngle = steer * steeringAngle; if (w.drive) w.collider.motorTorque = throttle * motorTorque; w.collider.brakeTorque = brake ? brakeTorque : 0f; if (w.visual) { w.collider.GetWorldPose(out var p, out var r); w.visual.SetPositionAndRotation(p, r); } }
            if (motorcycle && wheelie) body.AddRelativeTorque(Vector3.right * wheelieTorque, ForceMode.Force);
        }
        public void SetGrau(bool enabled) { wheelie = motorcycle && enabled; body.centerOfMass = wheelie ? new Vector3(0f, .15f, -.25f) : Vector3.zero; SetRearFriction(wheelie ? .65f : 1f); }
        private void SetRearFriction(float stiffness) { foreach (Wheel w in wheels) if (w.collider != null && !w.steer) { WheelFrictionCurve f = w.collider.sidewaysFriction; f.stiffness = stiffness; w.collider.sidewaysFriction = f; } }
        private void OnCollisionEnter(Collision collision) { float impact = collision.relativeVelocity.magnitude / 20f; SetDamage(Mathf.Clamp01(impact)); if (impact > 1.5f) SetWindshieldDamage(Mathf.Clamp01(impact - 1.5f)); }
        public void SetDamage(float value) { foreach (Renderer r in renderers) { if (!r) continue; r.GetPropertyBlock(block); block.SetFloat(Damage, Mathf.Clamp01(value)); block.SetFloat(Rust, rustAmount); block.SetFloat(Mud, mudIntensity); if (mudMask) block.SetTexture("_MudMask", mudMask); if (rustMask) block.SetTexture("_RustTexture", rustMask); r.SetPropertyBlock(block); } }
        public void SetWindshieldDamage(float value) { foreach (Renderer r in renderers) if (r) { r.GetPropertyBlock(block); block.SetFloat(Glass, value); if (shatteredGlass) block.SetTexture("_ShatteredGlass", shatteredGlass); r.SetPropertyBlock(block); } }
        public void SetMudIntensity(float value) { mudIntensity = Mathf.Clamp01(value); SetDamage(0f); }
    }
}
