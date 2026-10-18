/*
 * graph_coloring.c - Inkrementelle Graphfaerbung mit IPASIR
 *
 * Vorlesung: Practical SAT Solving (Prof. Dr. Carsten Sinz, HKA)
 * Uebungsblatt 1, Aufgabe 1
 *
 * Kompilierung (Beispiel mit CaDiCaL):
 *   gcc -O3 graph_coloring.c -I. -I<cadical>/src <cadical>/build/libcadical.a -lstdc++ -lm -o graph_coloring
 *
 * Aufruf:
 *   ./graph_coloring <graph.col>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipasir.h"

typedef struct {
    int u;
    int v;
} Edge;

/* Variablen-Mapping: var(v, c) fuer Knoten v (1..n) und Farbe c (1..k_max) */
static inline int var_idx(int v, int c, int k_max) {
    return (v - 1) * k_max + c;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Verwendung: %s <dimacs-graph-datei.col>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Fehler beim Oeffnen der Datei");
        return 1;
    }

    int n = 0;          /* Anzahl Knoten */
    int m = 0;          /* Anzahl Kanten */
    Edge *edges = NULL;
    int edge_count = 0;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'c' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        if (line[0] == 'p') {
            char format[32];
            if (sscanf(line, "p %s %d %d", format, &n, &m) >= 3) {
                edges = (Edge *)malloc(sizeof(Edge) * m);
            }
        } else if (line[0] == 'e') {
            int u, v;
            if (sscanf(line, "e %d %d", &u, &v) == 2) {
                if (edge_count < m) {
                    edges[edge_count].u = u;
                    edges[edge_count].v = v;
                    edge_count++;
                }
            }
        }
    }
    fclose(file);

    printf("Graph geladen: %s (|V| = %d, |E| = %d)\n", filename, n, edge_count);

    if (n == 0) {
        printf("Leerer Graph: 0 Farben benoetigt.\n");
        free(edges);
        return 0;
    }

    int k_max = n; /* Obergrenze fuer Farben */
    void *solver = ipasir_init();
    printf("IPASIR Solver initialisiert: %s\n", ipasir_signature());

    /* Hilfsvariablen-Basis: hinter allen x_{v,c} */
    int base_activation = n * k_max + 1;

    int chromatic_number = -1;

    /* Inkrementelle Suche ueber k = 1, 2, ... */
    for (int k = 1; k <= k_max; k++) {
        /* 1. Kantenklauseln fuer die neue Farbe k hinzufuegen:
              Fuer jede Kante (u, v): not(x_{u,k}) or not(x_{v,k})
              Diese Klauseln gelten dauerhaft fuer Farbe k. */
        for (int i = 0; i < edge_count; i++) {
            ipasir_add(solver, -var_idx(edges[i].u, k, k_max));
            ipasir_add(solver, -var_idx(edges[i].v, k, k_max));
            ipasir_add(solver, 0);
        }

        /* 2. At-Least-One (ALO) Klauseln fuer Farbe 1..k unter Verwendung
              eines Aktivierungs-Literals act_k:
              Fuer jeden Knoten v: (x_{v,1} or ... or x_{v,k} or not(act_k)) */
        int act_k = base_activation + k;
        for (int v = 1; v <= n; v++) {
            for (int c = 1; c <= k; c++) {
                ipasir_add(solver, var_idx(v, c, k_max));
            }
            ipasir_add(solver, -act_k);
            ipasir_add(solver, 0);
        }

        /* 3. Assumption fuer die aktuelle Farbanzahl k aktivieren */
        ipasir_assume(solver, act_k);

        /* 4. Loesen */
        int res = ipasir_solve(solver);
        if (res == 10) { /* SAT: k Farben reichen aus */
            chromatic_number = k;
            printf("\n========================================\n");
            printf("Ergebnis: Gueltige Faerbung gefunden!\n");
            printf("Benoetigte Farben (chromatische Zahl chi(G)): %d\n", chromatic_number);
            printf("========================================\n");
            printf("Knotenfaerbung:\n");
            for (int v = 1; v <= n; v++) {
                int assigned_color = 0;
                for (int c = 1; c <= k; c++) {
                    if (ipasir_val(solver, var_idx(v, c, k_max)) > 0) {
                        assigned_color = c;
                        break;
                    }
                }
                printf("  Knoten %3d -> Farbe %d\n", v, assigned_color);
            }
            break;
        } else if (res == 20) { /* UNSAT: k Farben reichen nicht aus */
            printf("k = %d: UNSAT (keine gueltige %d-Faerbung)\n", k, k);
        } else {
            fprintf(stderr, "Unerwarteter Solver-Status: %d\n", res);
            break;
        }
    }

    ipasir_release(solver);
    free(edges);
    return 0;
}

