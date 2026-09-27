using System;
using UnityEngine;

namespace BrazilianMafia.World
{
    public enum BM_WeatherType { Clear, Drizzle, Rain, HeavyRain, Storm, Dust }
    /// <summary>URP equivalent of an Unreal MPC climate driver using global shader properties.</summary>
    public sealed class BM_WeatherDirector : MonoBehaviour
    {
        [SerializeField] private BM_WeatherType current = BM_WeatherType.Clear; [SerializeField] private float transitionSeconds = 5f;
        [SerializeField, Range(0, 1)] private float wetness, puddles, verticalStreaks; [SerializeField] private AnimationCurve transition = AnimationCurve.EaseInOut(0, 0, 1, 1);
        private BM_WeatherType target; private float startWetness, startPuddles, startStreaks, elapsed;
        private static readonly int Wet = Shader.PropertyToID("_GlobalWetness"), Puddle = Shader.PropertyToID("_PuddleDepth"), Streak = Shader.PropertyToID("_VerticalWaterStreaks");
        private void OnEnable() { target = current; ApplyGlobals(); }
        private void Update() { if (current == target) return; elapsed += Time.deltaTime; float t = transition.Evaluate(Mathf.Clamp01(elapsed / Mathf.Max(.01f, transitionSeconds))); var v = Values(target); wetness = Mathf.Lerp(startWetness, v.wet, t); puddles = Mathf.Lerp(startPuddles, v.puddle, t); verticalStreaks = Mathf.Lerp(startStreaks, v.streak, t); ApplyGlobals(); if (t >= 1f) current = target; }
        public void TransitionTo(BM_WeatherType weather, float seconds = 5f) { startWetness = wetness; startPuddles = puddles; startStreaks = verticalStreaks; target = weather; transitionSeconds = seconds; elapsed = 0f; }
        private void ApplyGlobals() { Shader.SetGlobalFloat(Wet, wetness); Shader.SetGlobalFloat(Puddle, puddles); Shader.SetGlobalFloat(Streak, verticalStreaks); }
        private static (float wet, float puddle, float streak) Values(BM_WeatherType w) => w switch { BM_WeatherType.Drizzle => (.35f, .15f, .4f), BM_WeatherType.Rain => (.7f, .5f, .75f), BM_WeatherType.HeavyRain => (.95f, .85f, 1f), BM_WeatherType.Storm => (1f, 1f, 1f), BM_WeatherType.Dust => (0f, 0f, 0f), _ => (0f, 0f, 0f) };
    }
}
