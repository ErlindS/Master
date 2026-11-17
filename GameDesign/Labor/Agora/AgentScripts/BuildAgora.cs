using System.Linq;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;
using UnityEngine.Rendering;

// Legt die Daten-Assets und die Szene Assets/Scenes/Agora.unity an.
// Ausführen: unity command script run --file AgentScripts/BuildAgora.cs --entry BuildAgora.All
public static class BuildAgora
{
    const string DataDir = "Assets/Data";
    const string ZoneDir = "Assets/Data/Zones";
    const string ScenePath = "Assets/Scenes/Agora.unity";

    public static string All()
    {
        if (!AssetDatabase.IsValidFolder(DataDir)) AssetDatabase.CreateFolder("Assets", "Data");
        if (!AssetDatabase.IsValidFolder(ZoneDir)) AssetDatabase.CreateFolder(DataDir, "Zones");

        var wohnen = Zone("Wohnen", ZoneKind.Wohnen, "Wohnen", 30, 0f, 2, 3, 0f,
            new Color(0.45f, 0.75f, 0.35f), new Color(0.95f, 0.88f, 0.72f),
            "Hier ziehen Bewohner ein, wenn es Arbeit gibt. Zufriedene Bewohner lassen das Haus wachsen.");
        var industrie = Zone("Industrie", ZoneKind.Industrie, "Industrie", 60, 0f, 4, 1, 0f,
            new Color(0.85f, 0.7f, 0.25f), new Color(0.6f, 0.45f, 0.35f),
            "Arbeitsplätze. Jede Schicht erzeugt Waren und kostet die Firma Lohn.");
        var agora = Zone("Agora", ZoneKind.Agora, "Agora", 80, 0f, 0, 1, 0f,
            new Color(0.8f, 0.78f, 0.7f), new Color(0.9f, 0.85f, 0.75f),
            "Marktplatz. Nur hier werden Waren gehandelt; jeder Verkauf bringt Steuern.");
        var institution = Zone("Institution", ZoneKind.Institution, "Institution", 120, 1.5f, 0, 1, 9f,
            new Color(0.35f, 0.55f, 0.85f), new Color(0.92f, 0.92f, 0.95f),
            "Tempel/Rathaus. Kostet Unterhalt, macht Bewohner im Umkreis zufriedener.");

        string sysPath = DataDir + "/FreierMarkt.asset";
        var sys = AssetDatabase.LoadAssetAtPath<EconomySystem>(sysPath);
        if (sys == null)
        {
            sys = ScriptableObject.CreateInstance<EconomySystem>();
            sys.displayName = "Freier Markt";
            sys.description = "Preise entstehen aus Angebot und Nachfrage. Der Staat erhebt nur eine Marktsteuer.";
            AssetDatabase.CreateAsset(sys, sysPath);
        }
        AssetDatabase.SaveAssets();

        // ---- Szene ----
        var scene = EditorSceneManager.NewScene(NewSceneSetup.EmptyScene, NewSceneMode.Single);

        var lightGo = new GameObject("Sonne");
        var light = lightGo.AddComponent<Light>();
        light.type = LightType.Directional;
        light.intensity = 1.2f;
        light.shadows = LightShadows.Soft;
        light.color = new Color(1f, 0.96f, 0.88f);
        lightGo.transform.rotation = Quaternion.Euler(50f, -35f, 0f);

        RenderSettings.ambientMode = AmbientMode.Trilight;
        RenderSettings.ambientSkyColor = new Color(0.65f, 0.72f, 0.85f);
        RenderSettings.ambientEquatorColor = new Color(0.55f, 0.55f, 0.5f);
        RenderSettings.ambientGroundColor = new Color(0.3f, 0.28f, 0.25f);

        var camGo = new GameObject("Main Camera");
        camGo.tag = "MainCamera";
        var cam = camGo.AddComponent<Camera>();
        cam.clearFlags = CameraClearFlags.SolidColor;
        cam.backgroundColor = new Color(0.55f, 0.7f, 0.85f);
        cam.farClipPlane = 300f;
        camGo.AddComponent<AudioListener>();
        var cityCam = camGo.AddComponent<CityCamera>();
        cityCam.distance = 34f;
        cityCam.pitch = 58f;
        cityCam.focus = new Vector3(-3f, 0f, 0f); // etwas nach links, das Menü verdeckt den linken Rand
        camGo.transform.position = new Vector3(0, 25, -18);
        camGo.transform.rotation = Quaternion.Euler(cityCam.pitch, 0, 0);

        var root = new GameObject("Agora");
        var economy = root.AddComponent<Economy>();
        economy.rules = sys;
        root.AddComponent<CityGrid>();
        var tool = root.AddComponent<BuildTool>();
        tool.zones = new[] { wohnen, industrie, agora, institution };
        root.AddComponent<CityUI>();

        EditorSceneManager.SaveScene(scene, ScenePath);

        // In die Build-Liste aufnehmen (vorne), bestehende Szenen behalten
        var scenes = EditorBuildSettings.scenes.Where(s => s.path != ScenePath).ToList();
        scenes.Insert(0, new EditorBuildSettingsScene(ScenePath, true));
        EditorBuildSettings.scenes = scenes.ToArray();

        return "ok: " + ScenePath;
    }

    static ZoneDefinition Zone(string file, ZoneKind kind, string name, int cost, float upkeep,
        int capacity, int maxLevel, float radius, Color ground, Color building, string desc)
    {
        string path = $"{ZoneDir}/{file}.asset";
        var z = AssetDatabase.LoadAssetAtPath<ZoneDefinition>(path);
        bool isNew = z == null;
        if (isNew) z = ScriptableObject.CreateInstance<ZoneDefinition>();
        z.kind = kind;
        z.displayName = name;
        z.buildCost = cost;
        z.upkeepPerTick = upkeep;
        z.capacityPerLevel = capacity;
        z.maxLevel = maxLevel;
        z.influenceRadius = radius;
        z.groundColor = ground;
        z.buildingColor = building;
        z.description = desc;
        if (isNew) AssetDatabase.CreateAsset(z, path);
        else EditorUtility.SetDirty(z);
        return z;
    }
}
