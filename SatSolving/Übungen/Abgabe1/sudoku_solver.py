#!/usr/bin/env python3
"""
sudoku_solver.py - Loeser fuer verallgemeinerte Sudoku-Puzzles per SAT-Codierung

Vorlesung: Practical SAT Solving (Prof. Dr. Carsten Sinz, HKA)
Uebungsblatt 1, Aufgabe 2

Ein verallgemeinertes Sudoku der Ordnung n hat die Dimension n^2 x n^2 mit
n^2 Unterbloecken der Groesse n x n. Zahlenbereich: 1 .. n^2.
Standard-Sudoku (9x9) entspricht Ordnung n = 3.

Eingabeformat (wie in sudokus.zip): erste Zahl = Ordnung n, danach die
n^2 x n^2 Felder zeilenweise, 0 = leeres Feld.

Aufruf:
    python3 sudoku_solver.py <puzzle.sudoku> [--solver PFAD] [--cnf DATEI] [--quiet]

Codierung:
  * Variablen x(r,c,v) werden nur fuer leere Zellen und nur fuer Werte v
    angelegt, die nicht schon durch eine Vorgabe in Zeile, Spalte oder Block
    ausgeschlossen sind (Vorverarbeitung entspricht Unit-Propagation der
    Vorgaben).
  * Fuer jede Zelle, und fuer jede Kombination (Zeile, Wert), (Spalte, Wert),
    (Block, Wert) mit noch fehlendem Wert: "genau eins" =
    at-least-one (eine Klausel) + at-most-one.
  * At-most-one: paarweise fuer kleine Gruppen, sonst Sequential-Counter-
    Codierung (Sinz 2005) mit O(m) statt O(m^2) Klauseln.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import time

PAIRWISE_LIMIT = 6  # bis zu dieser Gruppengroesse paarweises AMO


class GeneralizedSudokuSolver:
    def __init__(self, order, grid):
        self.order = order
        self.N = order * order
        self.grid = grid              # Liste von Zeilen, 0 = leer
        self.var_of = {}              # (r, c, v) -> Variablen-ID
        self.lit_info = [None]        # Variablen-ID -> (r, c, v) (nur Hauptvariablen)
        self.num_vars = 0
        self.clauses = []
        self.trivially_unsat = False

    # ------------------------------------------------------------------ Hilfen
    def block(self, r, c):
        return (r // self.order) * self.order + c // self.order

    def new_var(self):
        self.num_vars += 1
        return self.num_vars

    # ----------------------------------------------------------- Kardinalitaet
    def at_most_one(self, lits):
        m = len(lits)
        if m <= 1:
            return
        if m <= PAIRWISE_LIMIT:
            for i in range(m):
                for j in range(i + 1, m):
                    self.clauses.append([-lits[i], -lits[j]])
            return
        # Sequential Counter: s_i <=> "eines von x_1..x_i ist wahr"
        s = [self.new_var() for _ in range(m - 1)]
        self.clauses.append([-lits[0], s[0]])
        for i in range(1, m - 1):
            self.clauses.append([-lits[i], s[i]])
            self.clauses.append([-s[i - 1], s[i]])
            self.clauses.append([-lits[i], -s[i - 1]])
        self.clauses.append([-lits[m - 1], -s[m - 2]])

    def exactly_one(self, lits):
        if not lits:
            self.trivially_unsat = True   # Wert/Zelle ohne Kandidat
            self.clauses.append([])
            return
        self.clauses.append(list(lits))
        self.at_most_one(lits)

    # ---------------------------------------------------------------- Codierung
    def encode(self):
        N, g = self.N, self.grid
        all_vals = set(range(1, N + 1))
        row_used = [set() for _ in range(N)]
        col_used = [set() for _ in range(N)]
        blk_used = [set() for _ in range(N)]
        for r in range(N):
            for c in range(N):
                v = g[r][c]
                if v:
                    b = self.block(r, c)
                    # doppelte Vorgabe in einer Einheit => unloesbar
                    if v in row_used[r] or v in col_used[c] or v in blk_used[b]:
                        self.trivially_unsat = True
                    row_used[r].add(v)
                    col_used[c].add(v)
                    blk_used[b].add(v)

        # Gruppen: (Typ, Index, Wert) -> Literale
        row_grp, col_grp, blk_grp = {}, {}, {}
        for r in range(N):
            for c in range(N):
                if g[r][c]:
                    continue
                b = self.block(r, c)
                cand = all_vals - row_used[r] - col_used[c] - blk_used[b]
                cell_lits = []
                for v in sorted(cand):
                    x = self.new_var()
                    self.var_of[(r, c, v)] = x
                    self.lit_info.append((r, c, v))
                    cell_lits.append(x)
                    row_grp.setdefault((r, v), []).append(x)
                    col_grp.setdefault((c, v), []).append(x)
                    blk_grp.setdefault((b, v), []).append(x)
                self.exactly_one(cell_lits)
        self.num_main_vars = len(self.lit_info) - 1

        # Jeder fehlende Wert muss in jeder Zeile/Spalte/Block genau einmal vorkommen
        for idx in range(N):
            for v in all_vals - row_used[idx]:
                self.exactly_one(row_grp.get((idx, v), []))
            for v in all_vals - col_used[idx]:
                self.exactly_one(col_grp.get((idx, v), []))
            for v in all_vals - blk_used[idx]:
                self.exactly_one(blk_grp.get((idx, v), []))

    def write_dimacs(self, path):
        with open(path, "w") as f:
            f.write(f"c Generalized Sudoku Order {self.order} ({self.N}x{self.N})\n")
            f.write(f"p cnf {self.num_vars} {len(self.clauses)}\n")
            for cl in self.clauses:
                f.write(" ".join(map(str, cl)) + " 0\n")

    # -------------------------------------------------------------------- Loesen
    def solve(self, solver_path, cnf_path=None):
        """Gibt geloestes Gitter zurueck oder None (UNSAT)."""
        tmp = None
        if cnf_path is None:
            fd, tmp = tempfile.mkstemp(suffix=".cnf")
            os.close(fd)
            cnf_path = tmp
        try:
            self.write_dimacs(cnf_path)
            proc = subprocess.run([solver_path, "-q", cnf_path],
                                  capture_output=True, text=True)
        finally:
            if tmp:
                os.remove(tmp)

        status, true_vars = None, set()
        for line in proc.stdout.splitlines():
            if line.startswith("s "):
                status = line[2:].strip()
            elif line.startswith("v "):
                true_vars.update(int(t) for t in line[2:].split() if int(t) > 0)
        if status == "UNSATISFIABLE":
            return None
        if status != "SATISFIABLE":
            raise RuntimeError(f"Solver-Fehler (exit {proc.returncode}):\n{proc.stderr}")

        sol = [row[:] for row in self.grid]
        for x in range(1, self.num_main_vars + 1):
            if x in true_vars:
                r, c, v = self.lit_info[x]
                sol[r][c] = v
        return sol

    # ------------------------------------------------------------------ Pruefen
    def is_valid_solution(self, sol):
        N, n = self.N, self.order
        full = set(range(1, N + 1))
        for i in range(N):
            if set(sol[i]) != full or {sol[r][i] for r in range(N)} != full:
                return False
        for br in range(n):
            for bc in range(n):
                vals = {sol[br * n + dr][bc * n + dc] for dr in range(n) for dc in range(n)}
                if vals != full:
                    return False
        return all(self.grid[r][c] in (0, sol[r][c]) for r in range(N) for c in range(N))

    def print_grid(self, grid):
        width = len(str(self.N))
        line_sep = "-+-".join(["-" * ((width + 1) * self.order - 1)] * self.order)
        for r in range(self.N):
            if r > 0 and r % self.order == 0:
                print(line_sep)
            cells = []
            for c in range(self.N):
                if c > 0 and c % self.order == 0:
                    cells.append("|")
                val = grid[r][c]
                cells.append(f"{(val if val else '.'):>{width}}")
            print(" ".join(cells))


def read_puzzle(path):
    with open(path) as f:
        tokens = f.read().split()
    order = int(tokens[0])
    N = order * order
    nums = list(map(int, tokens[1:1 + N * N]))
    if len(nums) != N * N:
        raise ValueError(f"{path}: erwartet {N*N} Felder, gefunden {len(nums)}")
    return order, [nums[r * N:(r + 1) * N] for r in range(N)]


def default_solver():
    found = shutil.which("cadical")
    if found:
        return found
    here = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(
        here, "../../../TheorieEffizienterAlgorithmen/SatSolving/Labor/cadical/build/cadical"))


def main():
    ap = argparse.ArgumentParser(description="Verallgemeinerter Sudoku-Loeser per SAT")
    ap.add_argument("puzzle", help="Sudoku-Datei (erste Zahl = Ordnung n)")
    ap.add_argument("--solver", default=os.environ.get("SAT_SOLVER", default_solver()),
                    help="SAT-Solver-Binary (DIMACS, Ausgabe 's'/'v'), Standard: CaDiCaL")
    ap.add_argument("--cnf", help="CNF zusaetzlich in diese Datei schreiben")
    ap.add_argument("--quiet", action="store_true", help="Gitter nicht ausgeben")
    args = ap.parse_args()

    order, grid = read_puzzle(args.puzzle)
    s = GeneralizedSudokuSolver(order, grid)
    t0 = time.perf_counter()
    s.encode()
    t1 = time.perf_counter()
    empty = sum(v == 0 for row in grid for v in row)
    print(f"{os.path.basename(args.puzzle)}: Ordnung {order} ({s.N}x{s.N}), "
          f"{empty} leere Felder")
    print(f"CNF: {s.num_vars} Variablen, {len(s.clauses)} Klauseln "
          f"(Codierung {t1 - t0:.2f} s)")

    if s.trivially_unsat:
        print("Ergebnis: UNLOESBAR (Widerspruch bereits in den Vorgaben)")
        return 20
    sol = s.solve(args.solver, args.cnf)
    t2 = time.perf_counter()
    if sol is None:
        print(f"Ergebnis: UNLOESBAR (Solver: UNSAT, {t2 - t1:.2f} s)")
        return 20
    ok = s.is_valid_solution(sol)
    print(f"Ergebnis: geloest (Solver {t2 - t1:.2f} s), Pruefung: {'OK' if ok else 'FEHLER'}")
    if not args.quiet:
        s.print_grid(sol)
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
