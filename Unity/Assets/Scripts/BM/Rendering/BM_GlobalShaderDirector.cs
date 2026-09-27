using System.Collections.Generic;
using UnityEngine;

namespace BrazilianMafia.Rendering
{
    /// <summary>
    /// Master global shader property director for URP.
    /// Drives environmental effects across thousands of objects simultaneously without per-object material mutations.
    /// </summary>
    public sealed class BM_GlobalShaderDirector : MonoBehaviour
    {
        [Header("Weather State")]
        [SerializeField, Range(0, 1)] private float globalWetness;
        [SerializeField, Range(0, 1)] private float puddleDepth;
        [SerializeField, Range(0, 1)] private float campoGrandeMudIntensity;
        [SerializeField, Range(0, 1)] private float verticalWaterStreaks;

        [Header("Environmental Colors")]
        [SerializeField] private Color favelaWallColor = new Color(0.6f, 0.4f, 0.2f);
        [SerializeField] private Color asphaltWetnessColor = Color.gray;
        [SerializeField] private Color skyboxOverlay = Color.white;

        [Header("Dynamic Transitions")]
        [SerializeField, Range(0.1f, 10f)] private float transitionSpeed = 2f;
        [SerializeField] private AnimationCurve weatherCurve = AnimationCurve.EaseInOut(0, 0, 1, 1);

        private float targetWetness, targetPuddles, targetMud, targetStreaks;
        private float transitionElapsed;
        private bool transitioning;

        private static readonly int
            WetnessID = Shader.PropertyToID("_GlobalWetness"),
            PuddleID = Shader.PropertyToID("_PuddleDepth"),
            MudID = Shader.PropertyToID("_CampoGrandeMudIntensity"),
            StreaksID = Shader.PropertyToID("_VerticalWaterStreaks"),
            FavelaColorID = Shader.PropertyToID("_FavelaWallTint"),
            AsphaltColorID = Shader.PropertyToID("_AsphaltWetnessColor"),
            SkyOverlayID = Shader.PropertyToID("_SkyboxOverlay");

        public static BM_GlobalShaderDirector Instance { get; private set; }

        private void Awake()
        {
            if (Instance != null && Instance != this) { Destroy(gameObject); return; }
            Instance = this;
            DontDestroyOnLoad(gameObject);
            ApplyGlobals();
        }

        private void Update()
        {
            if (!transitioning) return;
            transitionElapsed += Time.deltaTime;
            float t = weatherCurve.Evaluate(Mathf.Clamp01(transitionElapsed / transitionSpeed));

            globalWetness = Mathf.Lerp(globalWetness, targetWetness, t);
            puddleDepth = Mathf.Lerp(puddleDepth, targetPuddles, t);
            campoGrandeMudIntensity = Mathf.Lerp(campoGrandeMudIntensity, targetMud, t);
            verticalWaterStreaks = Mathf.Lerp(verticalWaterStreaks, targetStreaks, t);

            ApplyGlobals();

            if (t >= 1f)
            {
                globalWetness = targetWetness;
                puddleDepth = targetPuddles;
                campoGrandeMudIntensity = targetMud;
                verticalWaterStreaks = targetStreaks;
                transitioning = false;
            }
        }

        /// <summary>
        /// Transition to a new weather state over transitionSpeed seconds.
        /// </summary>
        public void SetWeatherState(float wetness, float puddles, float mud, float streaks)
        {
            targetWetness = Mathf.Clamp01(wetness);
            targetPuddles = Mathf.Clamp01(puddles);
            targetMud = Mathf.Clamp01(mud);
            targetStreaks = Mathf.Clamp01(streaks);
            transitionElapsed = 0f;
            transitioning = true;
        }

        /// <summary>
        /// Apply all global shader properties instantly.
        /// </summary>
        public void ApplyGlobals()
        {
            Shader.SetGlobalFloat(WetnessID, globalWetness);
            Shader.SetGlobalFloat(PuddleID, puddleDepth);
            Shader.SetGlobalFloat(MudID, campoGrandeMudIntensity);
            Shader.SetGlobalFloat(StreaksID, verticalWaterStreaks);
            Shader.SetGlobalColor(FavelaColorID, favelaWallColor);
            Shader.SetGlobalColor(AsphaltColorID, asphaltWetnessColor);
            Shader.SetGlobalColor(SkyOverlayID, skyboxOverlay);
        }

        /// <summary>
        /// Simulate a tropical storm in Rio or SP.
        /// </summary>
        public void TriggerStorm(float duration = 30f)
        {
            SetWeatherState(1f, 1f, 0.2f, 1f);
            Invoke(nameof(ClearWeather), duration);
        }

        /// <summary>
        /// Clear weather back to dry conditions.
        /// </summary>
        public void ClearWeather() => SetWeatherState(0f, 0f, 0f, 0f);

        /// <summary>
        /// Apply Campo Grande dust storm effect.
        /// </summary>
        public void ApplyCampoGrandeDust(float intensity = 0.8f)
        {
            SetWeatherState(0.1f, 0.1f, intensity, 0.3f);
            skyboxOverlay = new Color(1f, 0.9f, 0.7f, 0.4f);
        }

        private void OnDestroy() { if (Instance == this) Instance = null; }
    }
}
