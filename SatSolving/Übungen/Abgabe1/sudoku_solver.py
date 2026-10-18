#!/usr/bin/env python3
"""
sudoku_solver.py - Loeser fuer verallgemeinerte Sudoku-Puzzles per SAT-Codierung

Vorlesung: Practical SAT Solving (Prof. Dr. Carsten Sinz, HKA)
Uebungsblatt 1, Aufgabe 2

Ein verallgemeinertes Sudoku der Ordnung n hat die Dimension n^2 x n^2 mit
n^2 Unterbloecken der Groesse n x n. Zahlenbereich: 1 .. n^2.
Standard-Sudoku (9x9) entspricht Ordnung n = 3.
"""

import sys
import os
import subprocess
import math

class GeneralizedSudokuSolver:
    def __init__(self, order=3):
        self.order = order
        self.N = order * order  # Feldgroesse N x N
        self.num_vars = self.N ** 3
        self.clauses = []

    def var(self, r, c, v):
        """1-basierte Variablen-ID fuer Zelle (r, c) mit Wert v in 1..N"""
        return (r - 1) * (self.N * self.N) + (c - 1) * self.N + v

    def decode_var(self, var_id):
        """Dekodiert Variablen-ID zurueck zu (r, c, v)"""
        var_id -= 1
        v = (var_id % self.N) + 1
        var_id //= self.N
        c = (var_id % self.N) + 1
        r = (var_id // self.N) + 1
        return r, c, v

    def encode_rules(self):
        """Erzeugt alle SAT-Klauseln fuer die Sudoku-Regeln"""
        self.clauses = []
        N = self.N
        n = self.order

        # 1. Zellen-Bedingung: Jede Zelle (r, c) enthaelt genau eine Zahl 1..N
        for r in range(1, N + 1):
            for c in range(1, N + 1):
                # Mindestens eine Zahl (ALO)
                self.clauses.append([self.var(r, c, v) for v in range(1, N + 1)])
                # Hoechstens eine Zahl (AMO)
                for v1 in range(1, N + 1):
                    for v2 in range(v1 + 1, N + 1):
                        self.clauses.append([-self.var(r, c, v1), -self.var(r, c, v2)])

        # 2. Zeilen-Bedingung: Jede Zahl v kommt in jeder Zeile r genau einmal vor
        for r in range(1, N + 1):
            for v in range(1, N + 1):
                # Mindestens einmal (ALO)
                self.clauses.append([self.var(r, c, v) for c in range(1, N + 1)])
                # Hoechstens einmal (AMO)
                for c1 in range(1, N + 1):
                    for c2 in range(c1 + 1, N + 1):
                        self.clauses.append([-self.var(r, c1, v), -self.var(r, c2, v)])

        # 3. Spalten-Bedingung: Jede Zahl v kommt in jeder Spalte c genau einmal vor
        for c in range(1, N + 1):
            for v in range(1, N + 1):
                # Mindestens einmal (ALO)
                self.clauses.append([self.var(r, c, v) for r in range(1, N + 1)])
                # Hoechstens einmal (AMO)
                for r1 in range(1, N + 1):
                    for r2 in range(r1 + 1, N + 1):
                        self.clauses.append([-self.var(r1, c, v), -self.var(r2, c, v)])

        # 4. Block-Bedingung: Jede Zahl v kommt in jedem Unterblock genau einmal vor
        for br in range(n):
            for bc in range(n):
                block_cells = [
                    (br * n + dr, bc * n + dc)
                    for dr in range(1, n + 1)
                    for dc in range(1, n + 1)
                ]
                for v in range(1, N + 1):
                    # Mindestens einmal (ALO)
                    self.clauses.append([self.var(r, c, v) for (r, c) in block_cells])
                    # Hoechstens einmal (AMO)
                    for i in range(len(block_cells)):
                        for j in range(i + 1, len(block_cells)):
                            r1, c1 = block_cells[i]
                            r2, c2 = block_cells[j]
                            self.clauses.append([-self.var(r1, c1, v), -self.var(r2, c2, v)])

    def add_clues(self, grid):
        """Fuegt vorgegebene Zahlen als Einheitsklauseln hinzu"""
        for r in range(1, self.N + 1):
            for c in range(1, self.N + 1):
                val = grid[r - 1][c - 1]
                if val != 0:
                    self.clauses.append([self.var(r, c, val)])

    def to_dimacs(self, output_path):
        """Exportiert die Formel ins DIMACS CNF Format"""
        with open(output_path, "w") as f:
            f.write(f"c Generalized Sudoku Order {self.order} ({self.N}x{self.N})\n")
            f.write(f"p cnf {self.num_vars} {len(self.clauses)}\n")
            for cl in self.clauses:
                f.write(" ".join(map(str, cl)) + " 0\n")

    def print_grid(self, grid):
        """Gibt das Gitter formatiert auf der Konsole aus"""
        width = len(str(self.N))
        line_sep = "+".join(["-" * ((width + 1) * self.order + 1)] * self.order)
        for r in range(self.N):
            if r > 0 and r % self.order == 0:
                print(line_sep)
            row_str = []
            for c in range(self.N):
                if c > 0 and c % self.order == 0:
                    row_str.append("|")
                val = grid[r][c]
                val_s = str(val) if val != 0 else "."
                row_str.append(f"{val_s:>{width}}")
            print(" ".join(row_str))


def test_example():
    # Beispiel eines einfachen 9x9 Sudokus (Ordnung 3)
    sudoku = GeneralizedSudokuSolver(order=3)
    sudoku.encode_rules()
    print(f"Sudoku Ordnung 3 generiert: {sudoku.num_vars} Variablen, {len(sudoku.clauses)} Klauseln.")

if __name__ == "__main__":
    test_example()

