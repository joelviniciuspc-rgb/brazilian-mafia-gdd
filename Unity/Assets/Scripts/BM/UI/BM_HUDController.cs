using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.UI;
#if ENABLE_UI_TOOLKIT
using UnityEngine.UIElements;
#endif

namespace BrazilianMafia.UI
{
    /// <summary>
    /// Interactive 2D map overlay displaying real-time player positions, safehouse markers,
    /// and active police response zones. Reads from BM_PlayerController character positions.
    /// </summary>
    public sealed class BM_MapUI : MonoBehaviour
    {
        [Header("References")]
        [SerializeField] private Canvas mapCanvas;
        [SerializeField] private RectTransform mapPanelRect;
        [SerializeField] private Image mapBackgroundImage;

        [Header("Markers")]
        [SerializeField] private Image playerMarkerPrefab;
        [SerializeField] private Image safehouseMarkerPrefab;
        [SerializeField] private Image policeZoneMarkerPrefab;

        [Header("Configuration")]
        [SerializeField, Range(10, 1000)] private float worldToUIScale = 100f;
        [SerializeField] private Vector2 mapWorldMin = new Vector2(-2500, -2500);
        [SerializeField] private Vector2 mapWorldMax = new Vector2(2500, 2500);
        [SerializeField, Range(0.5f, 60f)] private float updateInterval = 0.2f;

        private List<(RectTransform marker, int characterId)> playerMarkers = new();
        private List<(RectTransform marker, Vector3 position)> safehouseMarkers = new();
        private List<(RectTransform marker, Vector3 center, float radius)> policeZones = new();
        private float lastUpdateTime;

        public static BM_MapUI Instance { get; private set; }

        private void Awake()
        {
            if (Instance != null && Instance != this) { Destroy(gameObject); return; }
            Instance = this;
        }

        private void OnEnable() { if (mapCanvas != null) mapCanvas.gameObject.SetActive(true); }
        private void OnDisable() { if (mapCanvas != null) mapCanvas.gameObject.SetActive(false); }

        private void Update()
        {
            if (Time.realtimeSinceStartup - lastUpdateTime < updateInterval) return;
            lastUpdateTime = Time.realtimeSinceStartup;
            UpdateMarkerPositions();
        }

        /// <summary>
        /// Register a character (Alemão, Marcola, Pampa) on the map.
        /// </summary>
        public void RegisterCharacterMarker(int characterId, Transform characterTransform, Color markerColor)
        {
            if (playerMarkerPrefab == null) return;
            var instance = Instantiate(playerMarkerPrefab, mapPanelRect);
            instance.color = markerColor;
            var rect = instance.GetComponent<RectTransform>();
            playerMarkers.Add((rect, characterId));
        }

        /// <summary>
        /// Add a safehouse/landmark marker to the map.
        /// </summary>
        public void AddSafehouseMarker(Vector3 worldPosition, Color markerColor, string labelText = "")
        {
            if (safehouseMarkerPrefab == null) return;
            var instance = Instantiate(safehouseMarkerPrefab, mapPanelRect);
            instance.color = markerColor;
            safehouseMarkers.Add((instance.GetComponent<RectTransform>(), worldPosition));
            if (!string.IsNullOrEmpty(labelText) && instance.TryGetComponent<Text>(out var text))
                text.text = labelText;
        }

        /// <summary>
        /// Draw a police response zone circle at a given world position.
        /// </summary>
        public void AddPoliceZone(Vector3 worldCenter, float radius, int wantedLevel)
        {
            if (policeZoneMarkerPrefab == null) return;
            var instance = Instantiate(policeZoneMarkerPrefab, mapPanelRect);
            instance.color = wantedLevel switch
            {
                1 => Color.yellow,
                2 => new Color(1, 0.5f, 0),
                3 => Color.red,
                4 => new Color(0.5f, 0, 0.5f),
                5 => Color.black,
                _ => Color.white
            };
            policeZones.Add((instance.GetComponent<RectTransform>(), worldCenter, radius));
        }

        /// <summary>
        /// Clear all police zone markers from the map.
        /// </summary>
        public void ClearPoliceZones()
        {
            foreach (var (marker, _, _) in policeZones)
                Destroy(marker.gameObject);
            policeZones.Clear();
        }

        /// <summary>
        /// Update marker positions based on world coordinates.
        /// </summary>
        private void UpdateMarkerPositions()
        {
            if (mapPanelRect == null) return;
            var rect = mapPanelRect.rect;

            // Update player markers
            foreach (var (marker, charId) in playerMarkers)
            {
                var controller = BrazilianMafia.Player.BM_PlayerController.GetControllerForCharacter(charId);
                if (controller != null) UpdateMarkerPosition(marker, controller.transform.position, rect);
            }

            // Update safehouse markers
            foreach (var (marker, worldPos) in safehouseMarkers)
                UpdateMarkerPosition(marker, worldPos, rect);

            // Update police zone circles
            foreach (var (marker, center, radius) in policeZones)
            {
                var uiPos = WorldToUIPosition(center, rect);
                marker.anchoredPosition = uiPos;
                marker.sizeDelta = new Vector2(radius * 2 / worldToUIScale, radius * 2 / worldToUIScale);
            }
        }

        /// <summary>
        /// Convert world position to UI canvas position.
        /// </summary>
        private Vector2 WorldToUIPosition(Vector3 worldPos, Rect canvasRect)
        {
            float x = Mathf.InverseLerp(mapWorldMin.x, mapWorldMax.x, worldPos.x) * canvasRect.width - canvasRect.width * 0.5f;
            float y = Mathf.InverseLerp(mapWorldMin.y, mapWorldMax.y, worldPos.z) * canvasRect.height - canvasRect.height * 0.5f;
            return new Vector2(x, y);
        }

        /// <summary>
        /// Update a single marker's UI position.
        /// </summary>
        private void UpdateMarkerPosition(RectTransform marker, Vector3 worldPos, Rect canvasRect)
        {
            marker.anchoredPosition = WorldToUIPosition(worldPos, canvasRect);
        }

        private void OnDestroy() { if (Instance == this) Instance = null; }
    }

    /// <summary>
    /// Master HUD controller managing all UI panels: minimap, wanted level, vehicle health, mission objectives.
    /// </summary>
    public sealed class BM_HUDController : MonoBehaviour
    {
        [Header("UI Elements")]
        [SerializeField] private Image wantedLevelIndicator;
        [SerializeField] private Text moneyDisplay, missionObjectiveText, vehicleHealthText;
        [SerializeField] private CanvasGroup minimapCanvasGroup;
        [SerializeField] private RectTransform wantedRadialMeter;

        [Header("Configuration")]
        [SerializeField] private Color[] wantedLevelColors = new Color[6] { Color.green, Color.yellow, new Color(1, 0.5f, 0), Color.red, new Color(0.5f, 0, 0.5f), Color.black };

        public static BM_HUDController Instance { get; private set; }

        private void Awake()
        {
            if (Instance != null && Instance != this) { Destroy(gameObject); return; }
            Instance = this;
            BrazilianMafia.Core.BM_GameManager.Instance.MoneyChanged += OnMoneyChanged;
            BrazilianMafia.Core.BM_GameManager.Instance.WantedLevelChanged += OnWantedLevelChanged;
        }

        private void OnMoneyChanged(long newMoney)
        {
            if (moneyDisplay != null)
                moneyDisplay.text = $"${newMoney:N0}";
        }

        private void OnWantedLevelChanged(int wantedLevel)
        {
            if (wantedLevelIndicator != null)
            {
                wantedLevelIndicator.color = wantedLevel < wantedLevelColors.Length ? wantedLevelColors[wantedLevel] : Color.red;
                wantedLevelIndicator.fillAmount = wantedLevel / 5f;
            }
            if (wantedRadialMeter != null)
            {
                wantedRadialMeter.localRotation = Quaternion.Euler(0, 0, -wantedLevel * 60);
            }
        }

        public void SetMissionObjective(string objective) { if (missionObjectiveText != null) missionObjectiveText.text = objective; }
        public void SetVehicleHealth(float healthPercent) { if (vehicleHealthText != null) vehicleHealthText.text = $"Health: {healthPercent:P0}"; }
        public void ToggleMinimap(bool show) { if (minimapCanvasGroup != null) minimapCanvasGroup.alpha = show ? 1f : 0f; }

        private void OnDestroy()
        {
            if (BrazilianMafia.Core.BM_GameManager.Instance != null)
            {
                BrazilianMafia.Core.BM_GameManager.Instance.MoneyChanged -= OnMoneyChanged;
                BrazilianMafia.Core.BM_GameManager.Instance.WantedLevelChanged -= OnWantedLevelChanged;
            }
            if (Instance == this) Instance = null;
        }
    }
}
