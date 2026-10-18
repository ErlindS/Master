/*
 * Kompilieren (Beispiel mit CaDiCaL, das IPASIR implementiert):
 *   gcc pythagoras_ipasir.c -I<cadical>/src <cadical>/build/libcadical.a -lstdc++ -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ipasir.h"

static void klausel(void *s, int a, int b, int c) {
    ipasir_add(s, a);
    ipasir_add(s, b);
    ipasir_add(s, c);
    ipasir_add(s, 0);
}

int main(int argc, char **argv) {
    int n = argc > 1 ? atoi(argv[1]) : 30;

    void *solver = ipasir_init();               /* Solver anlegen */

    int anzahl = 0;
    for (int a = 1; a <= n; a++) {
        for (int b = a + 1; b <= n; b++) {
            long c2 = (long)a * a + (long)b * b;
            int c = (int)sqrtl((long double)c2);
            if (c > n) break;
            if ((long)c * c != c2) continue;
            klausel(solver,  a,  b,  c);        /* nicht alle blau */
            klausel(solver, -a, -b, -c);        /* nicht alle rot  */
            anzahl++;
        }
    }
    printf("n = %d: %d Tripel, %d Klauseln\n", n, anzahl, 2 * anzahl);

    /* 3. Lösen: 10 = SAT, 20 = UNSAT */
    int ergebnis = ipasir_solve(solver);

    if (ergebnis == 10) {
        printf("SAT Färbung gefunden\n");
        if (n <= 50) {
            for (int i = 1; i <= n; i++)
                printf("%d: %s\n", i, ipasir_val(solver, i) > 0 ? "rot" : "blau");
        }
    } else if (ergebnis == 20) {
        printf("UNSAT keine Färbung möglich\n");
    }

    ipasir_release(solver);                     /* 5. Aufräumen */
    return 0;
}