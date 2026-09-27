using System;
using UnityEngine;

namespace BrazilianMafia.Player
{
    public enum BM_CharacterType { ALEMAO, MARCOLA, PAMPA }

    /// <summary>Input-agnostic controller. Feed movement/look values from PC or mobile adapters.</summary>
    [RequireComponent(typeof(CharacterController))]
    public sealed class BM_PlayerController : MonoBehaviour
    {
        [Serializable] public sealed class CharacterSlot { public BM_CharacterType type; public Transform target; public Transform cameraAnchor; }
        [SerializeField] private CharacterSlot[] characters;
        [SerializeField] private BM_CharacterType activeCharacter = BM_CharacterType.ALEMAO;
        [SerializeField] private Transform cameraRig;
        [SerializeField] private float walkSpeed = 4f, sprintSpeed = 7f, rotationSpeed = 12f, gravity = -20f, cameraBlendSeconds = .35f;
        private CharacterController controller; private Vector2 moveInput, lookInput; private bool sprintHeld; private float verticalVelocity; private float blendTime; private Transform blendFrom, blendTo; private Camera activeCamera;
        public BM_CharacterType ActiveCharacter => activeCharacter;
        public event Action<BM_CharacterType> CharacterChanged;

        private void Awake() { controller = GetComponent<CharacterController>(); activeCamera = Camera.main; SwitchCharacter(activeCharacter, true); }
        private void Update()
        {
            Vector3 forward = transform.forward * moveInput.y + transform.right * moveInput.x;
            float speed = sprintHeld ? sprintSpeed : walkSpeed;
            if (forward.sqrMagnitude > 1f) forward.Normalize();
            controller.Move(forward * speed * Time.deltaTime);
            if (forward.sqrMagnitude > .001f) transform.rotation = Quaternion.Slerp(transform.rotation, Quaternion.LookRotation(forward), rotationSpeed * Time.deltaTime);
            if (controller.isGrounded && verticalVelocity < 0f) verticalVelocity = -2f;
            verticalVelocity += gravity * Time.deltaTime; controller.Move(Vector3.up * verticalVelocity * Time.deltaTime);
            if (blendTo != null && cameraRig != null) { blendTime += Time.deltaTime; float t = Mathf.Clamp01(blendTime / cameraBlendSeconds); cameraRig.SetPositionAndRotation(Vector3.Lerp(blendFrom.position, blendTo.position, t), Quaternion.Slerp(blendFrom.rotation, blendTo.rotation, t)); if (t >= 1f) blendTo = null; }
        }
        public void SetMoveInput(Vector2 value) => moveInput = Vector2.ClampMagnitude(value, 1f);
        public void SetLookInput(Vector2 value) => lookInput = value;
        public void SetSprint(bool value) => sprintHeld = value;
        public void SwitchCharacter(BM_CharacterType type, bool instant = false)
        {
            CharacterSlot next = Array.Find(characters, c => c != null && c.type == type); if (next == null || next.target == null) return;
            CharacterSlot current = Array.Find(characters, c => c != null && c.type == activeCharacter);
            activeCharacter = type; transform.SetPositionAndRotation(next.target.position, next.target.rotation);
            blendFrom = current?.cameraAnchor ?? next.cameraAnchor; blendTo = next.cameraAnchor; blendTime = instant ? cameraBlendSeconds : 0f;
            if (instant && cameraRig != null) cameraRig.SetPositionAndRotation(next.cameraAnchor.position, next.cameraAnchor.rotation);
            CharacterChanged?.Invoke(type);
        }
    }
}
