using System.Collections;
using System.Collections.Generic;
using UnityEngine;

// Ein Gebäude auf einer Zelle eines Feldes: Haus, Werkstatt, Markt oder Tempel.
// Häuser haben Bewohner und wachsen in Stufen (Stockwerke), Werkstätten haben Arbeiter.
public class Building : MonoBehaviour
{
    public CityField Field { get; private set; }
    public Cell Cell { get; private set; }
    public int Level { get; private set; }
    public int Workers { get; private set; }
    public readonly List<Resident> Residents = new List<Resident>();

    public ZoneKind Kind => kind;
    public int Capacity => capacityPerLevel * Level;
    public bool HasFreeJob => kind == ZoneKind.Industrie && Workers < Capacity;

    private CityGrid grid;
    private ZoneKind kind;
    private int capacityPerLevel;
    private Color color;
    private GameObject model;

    public void Init(CityGrid owner, CityField field, Cell cell, int level)
    {
        grid = owner;
        Field = field;
        Cell = cell;
        kind = field.Zone.kind;
        capacityPerLevel = field.Zone.capacityPerLevel;
        color = field.Zone.buildingColor;
        // leicht zufällig gedreht/gefärbt, damit es nicht wie ein Raster aussieht
        transform.rotation = Quaternion.Euler(0f, Random.Range(0, 4) * 90f, 0f);
        color = Color.Lerp(color, color * Random.Range(0.8f, 1.05f), 0.6f);
        color.a = 1f;
        SetLevel(level);
    }

    public void SetLevel(int level)
    {
        level = Mathf.Max(0, level);
        if (level == Level && model != null) return;
        int old = Level;
        Level = level;
        BuildModel(level > old);

        if (kind == ZoneKind.Wohnen)
        {
            while (Residents.Count < Capacity) Residents.Add(grid.SpawnResident(this));
            while (Residents.Count > Capacity)
            {
                Resident r = Residents[Residents.Count - 1];
                Residents.RemoveAt(Residents.Count - 1);
                r.MoveOut();
            }
        }
    }

    // Abriss: Bewohner ziehen weg, Arbeiter verlieren den Job
    public void Remove()
    {
        foreach (Resident r in Residents.ToArray()) r.MoveOut();
        Residents.Clear();
        if (kind == ZoneKind.Industrie) grid.FireAllWorkers(this);
        if (Cell != null && Cell.building == this) Cell.building = null;
        Field.Buildings.Remove(this);
        Destroy(gameObject);
    }

    public bool TryHire()
    {
        if (!HasFreeJob) return false;
        Workers++;
        return true;
    }

    public void Fire() => Workers = Mathf.Max(0, Workers - 1);

    public float AverageSatisfaction()
    {
        if (Residents.Count == 0) return 0.5f;
        float sum = 0f;
        foreach (Resident r in Residents) sum += r.Satisfaction;
        return sum / Residents.Count;
    }

    // ---- Aussehen aus einfachen Formen ----

    void BuildModel(bool animate)
    {
        if (model != null) Destroy(model);
        model = new GameObject("Modell");
        model.transform.SetParent(transform, false);
        float s = grid.cellSize;

        switch (kind)
        {
            case ZoneKind.Wohnen:
                // Haus: jedes Stockwerk ein Block, oben ein Dach
                float floor = 0.45f;
                AddBox(Vector3.zero, new Vector3(s * 0.6f, floor * Level, s * 0.55f), color);
                AddBox(new Vector3(0f, floor * Level, 0f), new Vector3(s * 0.68f, 0.12f, s * 0.62f), new Color(0.65f, 0.3f, 0.22f));
                break;

            case ZoneKind.Industrie:
                AddBox(Vector3.zero, new Vector3(s * 0.8f, 0.6f, s * 0.6f), color);
                AddBox(new Vector3(s * 0.25f, 0f, s * 0.15f), new Vector3(0.18f, 1.3f, 0.18f), color * 0.6f);
                break;

            case ZoneKind.Agora:
                AddBox(Vector3.zero, new Vector3(s * 0.9f, 0.06f, s * 0.9f), color);
                for (int i = 0; i < 4; i++)
                {
                    float px = (i % 2 == 0 ? -1 : 1) * s * 0.38f;
                    float pz = (i < 2 ? -1 : 1) * s * 0.38f;
                    AddBox(new Vector3(px, 0f, pz), new Vector3(0.1f, 0.9f, 0.1f), new Color(0.95f, 0.95f, 0.92f));
                }
                AddBox(new Vector3(-s * 0.15f, 0f, 0f), new Vector3(0.3f, 0.25f, 0.22f), new Color(0.75f, 0.45f, 0.2f));
                AddBox(new Vector3(s * 0.15f, 0f, 0.1f), new Vector3(0.3f, 0.25f, 0.22f), new Color(0.6f, 0.3f, 0.2f));
                break;

            case ZoneKind.Institution:
                AddBox(Vector3.zero, new Vector3(s * 0.85f, 0.12f, s * 0.65f), color);
                for (int i = 0; i < 4; i++)
                {
                    float px = Mathf.Lerp(-s * 0.33f, s * 0.33f, i / 3f);
                    AddBox(new Vector3(px, 0.12f, s * 0.24f), new Vector3(0.09f, 0.8f, 0.09f), color);
                    AddBox(new Vector3(px, 0.12f, -s * 0.24f), new Vector3(0.09f, 0.8f, 0.09f), color);
                }
                AddBox(new Vector3(0f, 0.92f, 0f), new Vector3(s * 0.9f, 0.15f, s * 0.7f), color * 0.85f);
                break;
        }

        if (animate) StartCoroutine(Rise(model.transform));
    }

    void AddBox(Vector3 localPos, Vector3 size, Color c)
    {
        c.a = 1f;
        GameObject box = GameObject.CreatePrimitive(PrimitiveType.Cube);
        Destroy(box.GetComponent<Collider>());
        box.transform.SetParent(model.transform, false);
        box.transform.localPosition = localPos + new Vector3(0f, size.y * 0.5f + 0.04f, 0f);
        box.transform.localScale = size;
        box.GetComponent<Renderer>().sharedMaterial = CityMaterials.Get(c);
    }

    IEnumerator Rise(Transform t)
    {
        const float duration = 0.5f;
        for (float time = 0f; time < duration && t != null; time += Time.deltaTime)
        {
            float k = time / duration;
            t.localScale = new Vector3(1f, 1f - (1f - k) * (1f - k), 1f);
            yield return null;
        }
        if (t != null) t.localScale = Vector3.one;
    }
}

// Kleiner Cache, damit pro Farbe nur ein Material entsteht.
public static class CityMaterials
{
    static readonly Dictionary<Color, Material> cache = new Dictionary<Color, Material>();
    static Shader shader;

    public static Material Get(Color color)
    {
        if (cache.TryGetValue(color, out Material m) && m != null) return m;
        if (shader == null)
            shader = Shader.Find("Universal Render Pipeline/Lit") ?? Shader.Find("Standard");
        m = new Material(shader);
        m.SetColor("_BaseColor", color);
        m.color = color;
        cache[color] = m;
        return m;
    }
}
