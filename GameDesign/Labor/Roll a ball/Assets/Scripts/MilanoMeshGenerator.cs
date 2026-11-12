using System.Collections.Generic;
using UnityEngine;

// Prozeduraler Generator für neoklassizistische Architektur der Galleria Vittorio Emanuele II
public static class MilanoMeshGenerator
{
    // Baut einen echten Rundbogen mit profilierter Laibung und Schlussstein
    public static GameObject CreateArch(string name, float width, float height, float depth, Material stoneMat, Material darkMat)
    {
        GameObject root = new GameObject(name);

        float halfW = width * 0.5f;
        float pillarW = 0.45f;
        float pillarH = height - halfW;

        // Linker & Rechter Pfeiler mit Sockel und Kapitell
        CreatePillar(root.transform, new Vector3(-halfW + pillarW * 0.5f, 0f, 0f), pillarW, pillarH, depth, stoneMat);
        CreatePillar(root.transform, new Vector3(halfW - pillarW * 0.5f, 0f, 0f), pillarW, pillarH, depth, stoneMat);

        // Gebogener Torbogen (Halbring)
        int segments = 16;
        float innerR = halfW - pillarW;
        float outerR = halfW;
        float archY = pillarH;

        for (int i = 0; i < segments; i++)
        {
            float a0 = Mathf.PI * (float)i / segments;
            float a1 = Mathf.PI * (float)(i + 1) / segments;

            Vector3 p0 = new Vector3(Mathf.Cos(a0) * (innerR + outerR) * 0.5f, archY + Mathf.Sin(a0) * (innerR + outerR) * 0.5f, 0f);
            Vector3 p1 = new Vector3(Mathf.Cos(a1) * (innerR + outerR) * 0.5f, archY + Mathf.Sin(a1) * (innerR + outerR) * 0.5f, 0f);

            GameObject seg = GameObject.CreatePrimitive(PrimitiveType.Cube);
            seg.name = $"ArchSeg_{i}";
            Object.Destroy(seg.GetComponent<Collider>());
            seg.transform.SetParent(root.transform, false);
            seg.transform.localPosition = (p0 + p1) * 0.5f;
            seg.transform.right = (p1 - p0).normalized;
            seg.transform.localScale = new Vector3(Vector3.Distance(p0, p1) * 1.05f, pillarW, depth);
            seg.GetComponent<Renderer>().sharedMaterial = stoneMat;
        }

        // Schlussstein (Keystone) an der Spitze
        GameObject keystone = GameObject.CreatePrimitive(PrimitiveType.Cube);
        keystone.name = "Keystone";
        Object.Destroy(keystone.GetComponent<Collider>());
        keystone.transform.SetParent(root.transform, false);
        keystone.transform.localPosition = new Vector3(0f, archY + outerR + 0.1f, 0f);
        keystone.transform.localScale = new Vector3(0.55f, 0.45f, depth * 1.15f);
        keystone.GetComponent<Renderer>().sharedMaterial = stoneMat;

        // Schaufenster / Rückwand im Bogen
        GameObject window = GameObject.CreatePrimitive(PrimitiveType.Cube);
        window.name = "StorefrontWindow";
        Object.Destroy(window.GetComponent<Collider>());
        window.transform.SetParent(root.transform, false);
        window.transform.localPosition = new Vector3(0f, height * 0.45f, 0f);
        window.transform.localScale = new Vector3(width - pillarW * 2f, height * 0.85f, depth * 0.2f);
        window.GetComponent<Renderer>().sharedMaterial = darkMat;

        return root;
    }

    static void CreatePillar(Transform parent, Vector3 localPos, float w, float h, float d, Material mat)
    {
        // Hauptschaft
        GameObject shaft = GameObject.CreatePrimitive(PrimitiveType.Cube);
        shaft.name = "PillarShaft";
        Object.Destroy(shaft.GetComponent<Collider>());
        shaft.transform.SetParent(parent, false);
        shaft.transform.localPosition = localPos + Vector3.up * (h * 0.5f);
        shaft.transform.localScale = new Vector3(w, h, d);
        shaft.GetComponent<Renderer>().sharedMaterial = mat;

        // Sockel (Plinthe)
        GameObject basePlinth = GameObject.CreatePrimitive(PrimitiveType.Cube);
        basePlinth.name = "PillarBase";
        Object.Destroy(basePlinth.GetComponent<Collider>());
        basePlinth.transform.SetParent(parent, false);
        basePlinth.transform.localPosition = localPos + Vector3.up * 0.15f;
        basePlinth.transform.localScale = new Vector3(w * 1.25f, 0.3f, d * 1.25f);
        basePlinth.GetComponent<Renderer>().sharedMaterial = mat;

        // Kapitell
        GameObject capital = GameObject.CreatePrimitive(PrimitiveType.Cube);
        capital.name = "PillarCapital";
        Object.Destroy(capital.GetComponent<Collider>());
        capital.transform.SetParent(parent, false);
        capital.transform.localPosition = localPos + Vector3.up * (h - 0.15f);
        capital.transform.localScale = new Vector3(w * 1.25f, 0.3f, d * 1.25f);
        capital.GetComponent<Renderer>().sharedMaterial = mat;
    }

    // Baut das gusseiserne Glas-Tonnengewölbe mit Rippen
    public static GameObject CreateGlassBarrelVault(string name, float width, float length, float height, Material ironMat, Material glassMat)
    {
        GameObject root = new GameObject(name);
        int ribs = 10;
        int archSegments = 14;
        float radius = width * 0.5f;

        for (int r = 0; r <= ribs; r++)
        {
            float z = -length * 0.5f + length * ((float)r / ribs);
            for (int s = 0; s < archSegments; s++)
            {
                float a0 = Mathf.PI * (float)s / archSegments;
                float a1 = Mathf.PI * (float)(s + 1) / archSegments;

                Vector3 p0 = new Vector3(Mathf.Cos(a0) * radius, height + Mathf.Sin(a0) * radius, z);
                Vector3 p1 = new Vector3(Mathf.Cos(a1) * radius, height + Mathf.Sin(a1) * radius, z);

                GameObject strut = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
                strut.name = $"VaultRib_{r}_{s}";
                Object.Destroy(strut.GetComponent<Collider>());
                strut.transform.SetParent(root.transform, false);
                strut.transform.localPosition = (p0 + p1) * 0.5f;
                strut.transform.up = (p1 - p0).normalized;
                strut.transform.localScale = new Vector3(0.08f, Vector3.Distance(p0, p1) * 0.5f, 0.08f);
                strut.GetComponent<Renderer>().sharedMaterial = ironMat;
            }
        }

        // Längsträger (Kämpfer / First)
        for (int a = 0; a <= archSegments; a += 2)
        {
            float angle = Mathf.PI * (float)a / archSegments;
            Vector3 startP = new Vector3(Mathf.Cos(angle) * radius, height + Mathf.Sin(angle) * radius, -length * 0.5f);
            Vector3 endP = new Vector3(Mathf.Cos(angle) * radius, height + Mathf.Sin(angle) * radius, length * 0.5f);

            GameObject beam = GameObject.CreatePrimitive(PrimitiveType.Cube);
            beam.name = $"LongitudinalBeam_{a}";
            Object.Destroy(beam.GetComponent<Collider>());
            beam.transform.SetParent(root.transform, false);
            beam.transform.localPosition = (startP + endP) * 0.5f;
            beam.transform.localScale = new Vector3(0.12f, 0.12f, length);
            beam.GetComponent<Renderer>().sharedMaterial = ironMat;
        }

        return root;
    }
}

