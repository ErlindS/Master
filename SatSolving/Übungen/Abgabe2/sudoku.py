
try:
    from .ipasir_helper import (
        ipasir_add,
        ipasir_init,
        ipasir_release,
        ipasir_solve,
        ipasir_val,
    )
except ImportError:
    from ipasir_helper import (
        ipasir_add,
        ipasir_init,
        ipasir_release,
        ipasir_solve,
        ipasir_val,
    )
from itertools import combinations
from math import isqrt
from pathlib import Path

import argparse

# Solver initialisieren
solver = ipasir_init()


def ipasir_XOR(vars):
    #Da fast alle Sudoku Regeln ein XOR brauchen, 
    # dient diese Helper Funktion als XOR.
    # Nimmt eine Liste an Nummern an und wandelt sie
    # in IPASIR Regeln um:

    # Mindestens eine Variable muss wahr sein
    for a in vars:
        ipasir_add(solver, a)
    ipasir_add(solver, 0)

    # Höchstens eine Variable darf wahr sein
    for a, b in combinations(vars, 2):
        ipasir_add(solver, -a)
        ipasir_add(solver, -b)
        ipasir_add(solver, 0)


def defineSpace(order):
    #Grundwerte definieren auf Basis des Sudoku-Umfangs (3 -> 9*9 Feld)
    
    global rowLength, cellAmount, variableAmount, numberSpace, blockSize

    rowLength = order*order
    numberSpace = order*order
    cellAmount = rowLength * rowLength
    variableAmount = cellAmount * rowLength
    blockSize = isqrt(rowLength)


def readSudokuFile(link):
    #Nimmt ein .sudoku File an und wandelt es in eine Liste um

    with open(link, "r") as file:
        lines = file.readlines()

    # Erste Zeile lesen und Sudoku-Größe festlegen
    order = int(lines[0])
    defineSpace(order)

    # Alle Sudoku-Werte in eine Liste umwandeln
    sudoku = []
    for line in lines[1:]:
        sudoku.extend(map(int, line.split()))

    return sudoku


def printResult():
    #Gibt das Ergebnis im Termina aus

    if result == 10:
        print("SAT")

        printSudoku()

        #print Wahrheitswerte jeder Variabel
        #anzahl = variableAmount
        #for var in range(1, anzahl + 1):
        #    value = ipasir_val(solver, var)
        #    print(f"x{var} = {'True' if value > 0 else 'False'}")
            
    elif result == 20:
        print("UNSAT")
    else:
        print("UNKNOWN")


def defineRowRules():
    #Füge Regel hinzu:
    # In jeder Zeile muss und darf (XOR) jede
    # Zahl einmal vorkommen


    #Für jede Zeile...
    for currRow in range(0, variableAmount, rowLength * numberSpace):
       #für jede Zahl im numberSpace...
        for currNumber in range(0, numberSpace):
            #... definiere XOR für diese Zahl in der Zeile
            vars = []
            for i in range(1 + currRow + currNumber, (rowLength * numberSpace) + currRow+ currNumber, numberSpace):
                vars.append(i)
            ipasir_XOR(vars)


def defineColumnRules():
    #Füge Regel hinzu:
    # In jeder Spalte muss und darf (XOR) jede
    # Zahl einmal vorkommen


    #für jede Zahl im numberSpace
    for currNumber in range(1, numberSpace +1):

        #für jede Spalte (durch jede Zelle in erster Zeile iterieren)
        for currCol in range(rowLength):
            vars = []
            #für jede Zelle in dieser Spalte
            for row in range(rowLength):
                var = (
                    #füge die Nummer in der Zelle zum XOR
                    row * rowLength * numberSpace
                    + currCol * numberSpace
                    + currNumber
                )
                vars.append(var)
            ipasir_XOR(vars)



def defineBlockRules():
    #Füge Regel hinzu:
    # In jedem Block muss und darf (XOR) jede
    # Zahl einmal vorkommen

    #für jede Zahl im numberSpace...
    for currNumber in range (1, numberSpace + 1):
        
        #für jeden Blockanfang...
        #(Zeile)
        for blockRow in range(0, rowLength, blockSize):
            #(Spalte)
            for blockCol in range(0, rowLength, blockSize):
                
                vars = []
                # Alle Zellen des Blocks durchlaufen
                for row in range(blockRow, blockRow + blockSize):
                    for col in range(blockCol, blockCol + blockSize):
                        #füge die Nummer in der Zelle zum XOR
                        var = (
                            row * rowLength * numberSpace
                            + col * numberSpace
                            + currNumber
                        )

                        vars.append(var)

                ipasir_XOR(vars)

        


def defineCellRules():
    #Da SAT nur binär kann, wird der Zahlenraum
    # per One-Hot definiert. 
    # Bsp: ein 9*9 Sudoku hat pro Zelle 9 Variabeln,
    # Davon ist nur eine Wahr

    #für jede Zelle im Sudoku...
    for currStart in range(0, cellAmount * numberSpace, numberSpace):
        
        #...definiere XOR für den Zahlenbereich dieser Zelle
        #also: es darf und muss nur eine Zahl pro Zelle stehen
        vars = []
        for i in range(1 + currStart, numberSpace + currStart + 1):
            vars.append(i)
        ipasir_XOR(vars)


def conciderSudokuChallenge(link=None):
    #Wandelt die Werte aus gegebenen .sudoku file
    # in IPASIR Regeln um

    sudoku = []
    if link:
        sudoku = readSudokuFile(link)
    if not sudoku:
        return
    
    for index, value in enumerate(sudoku):
       #Wenn die Zelle nicht leer (0) ist
       if value != 0:
            #füge sie als Bedingung hinzu
            ipasir_add(solver, index*numberSpace + value)
            ipasir_add(solver, 0)





def printSudoku():

    #Setup für Trennstriche:
    # Breite der horizontalen Trennlinie berechnen
    lineLength = rowLength * 2 + (rowLength - 1) // blockSize * 3

    #für jede Reihe...
    for row in range(rowLength):
        #checke für Trennstrich
        if row > 0 and row % blockSize == 0:
            print("-" * lineLength)

        #für jede Spalte...
        for col in range(rowLength):
            number = 0

            #für jede Zahl in numberSpace...
            for n in range(1, numberSpace + 1):
                #...Wenn diese Zahl die Zelle belegt, merke sie
                var = row * rowLength * numberSpace + col * numberSpace + n
                value = ipasir_val(solver, var)

                if value > 0:
                    number = n
                    break
            
            #gebe die Zahl in der Zelle aus
            #Wenn keine Zahl gefunden wurde, schreibe "."
            print(f"{number if number != 0 else '.':>{len(str(rowLength)) + 1}}", end=" ")


            #checke Vertikale Trennlinie
            if (col + 1) % blockSize == 0 and col < rowLength - 1:
                print("|", end=" ")


        #Setze Absatz nach jeder Zeile
        print(" Zeile ", row+1)


def main():
    global result

    parser = argparse.ArgumentParser()
    parser.add_argument("--input")
    args = parser.parse_args()
    try:
        defineSpace(3)
        input_path = (
            Path(__file__).resolve().parent / "sudokus" / args.input
            if args.input
            else None
        )
        
        
        conciderSudokuChallenge(input_path)
        defineCellRules()
        defineBlockRules()
        defineRowRules()
        defineColumnRules()

        result = ipasir_solve(solver)
        printResult()
    finally:
        ipasir_release(solver)


if __name__ == "__main__":
    main()
