#include <ipasir.h>

#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <fstream>

int knots;
int colors;
std::string graphFile;

// Liest den Pfad und die Knotenzahl aus der Graphdatei.
void setup(const std::filesystem::path &programPath,
           const std::string &fileName) {
  const std::filesystem::path graphDirectory =
      programPath.parent_path().parent_path() / "graphs";
  graphFile = (graphDirectory / fileName).string();
  colors = 0;

  std::ifstream input(graphFile);
  if (!input) {
    throw std::runtime_error("Graphdatei konnte nicht geöffnet werden: " +
                             graphFile);
  }

  char type;
  std::string format;
  while (input >> type) {
    if (type == 'p') {
      input >> format >> knots;
      return;
    }
    input.ignore(1000, '\n');
  }

  throw std::runtime_error("Keine gueltige p-Zeile in Graphdatei: " +
                           graphFile);
}



namespace {

class Solver {
public:
  // Erstellt den IPASIR-Solver.
  Solver() : handle_(ipasir_init()) {
    if (handle_ == nullptr) {
      throw std::runtime_error("IPASIR-Initialisierung fehlgeschlagen.");
    }
  }

  // Gibt die IPASIR-Ressourcen frei.
  ~Solver() {
    if (handle_ != nullptr) {
      ipasir_release(handle_);
    }
  }

  Solver(const Solver &) = delete;
  Solver &operator=(const Solver &) = delete;

  // Fuegt dem Solver eine abgeschlossene SAT-Klausel hinzu.
  void add_clause(const std::vector<int> &clause) {
    for (const int literal : clause) {
      if (literal == 0) {
        throw std::invalid_argument(
            "Eine Klausel darf kein Null-Literal enthalten.");
      }
      ipasir_add(handle_, literal);
    }
    // IPASIR verwendet 0 als Klauselabschluss; ein leerer Vektor codiert
    // deshalb korrekt eine leere Klausel.
    ipasir_add(handle_, 0);
  }


  // Setzt eine temporaere Annahme fuer den naechsten Solve-Aufruf.
  void assume(const int literal) {
    if (literal == 0) {
      throw std::invalid_argument(
          "Eine Annahme darf kein Null-Literal enthalten.");
    }
    ipasir_assume(handle_, literal);
  }

  // Startet die SAT-Pruefung und liefert den IPASIR-Status zurueck.
  int solve() { return ipasir_solve(handle_); }

  void reset() {
    ipasir_release(handle_);
    handle_ = ipasir_init();
    if (handle_ == nullptr) {
      throw std::runtime_error(
          "IPASIR-Neuinitialisierung fehlgeschlagen.");
    }
  }

  int value(const int literal) const { return ipasir_val(handle_, literal); }

  bool failed(const int literal) const {
    return ipasir_failed(handle_, literal) != 0;
  }

  // Liefert die verwendete IPASIR-Implementierung.
  static const char *signature() { return ipasir_signature(); }

private:
  void *handle_;
};

// Liefert die gemeinsame Solver-Instanz des Programms.
Solver &solver_instance() {
  static Solver solver;
  return solver;
}

// Kodiert eine Exactly-one-Bedingung fuer die Variablen.
void ipasir_XOR(const std::vector<int> &variables) {
  // Mindestens eine Variable muss wahr sein.
  solver_instance().add_clause(variables);

  // Höchstens eine Variable darf wahr sein.
  for (std::size_t first = 0; first < variables.size(); ++first) {
    for (std::size_t second = first + 1; second < variables.size(); ++second) {
      solver_instance().add_clause(
          {-variables[first], -variables[second]});
    }
  }
}

} // namespace


// Fuegt fuer jeden Knoten eine weitere Farbe zur One-hot-Kodierung hinzu.
void addColor() {
  ++colors;
  for (int knot = 0; knot < knots; ++knot) {
    std::vector<int> knotColors;
    knotColors.reserve(colors);
    for (int color = 0; color < colors; ++color) {
      knotColors.push_back(color * knots + knot + 1);
    }
    ipasir_XOR(knotColors);
  }
}

// Kodiert alle moeglichen Farben fuer jeden Knoten.
void addAllColors() {
  for (int knot = 0; knot < knots; ++knot) {
    std::vector<int> knotColors;
    knotColors.reserve(knots);
    for (int color = 0; color < knots; ++color) {
      knotColors.push_back(color * knots + knot + 1);
    }
    ipasir_XOR(knotColors);
  }
}

// Schaltet alle Farben oberhalb der aktuellen Grenze per Annahme ab.
void limitColors() {
  for (int knot = 0; knot < knots; ++knot) {
    for (int color = colors; color < knots; ++color) {
      const int variable = color * knots + knot + 1;
      solver_instance().assume(-variable);
    }
  }
}

// Verhindert, dass benachbarte Knoten dieselbe neue Farbe verwenden.
void XORlastColor() {
  std::ifstream input(graphFile);
  char type;
  int from;
  int to;
  const int lastColor = colors - 1;

  while (input >> type) {
    if (type == 'e' && input >> from >> to) {
      const int fromVariable = lastColor * knots + from;
      const int toVariable = lastColor * knots + to;
      solver_instance().add_clause({-fromVariable, -toVariable});
    } else {
      input.ignore(1000, '\n');
    }
  }
}

// Gibt die im SAT-Modell gewählte Farbe für jeden Knoten aus.
void printColors() {
  for (int knot = 0; knot < knots; ++knot) {
    int assignedColor = 0;
    for (int color = 0; color < colors; ++color) {
      const int variable = color * knots + knot + 1;
      if (solver_instance().value(variable) > 0) {
        assignedColor = color + 1;
        break;
      }
    }

    if (assignedColor == 0) {
      throw std::runtime_error(
          "Das SAT-Modell enthaelt keine Farbe fuer Knoten " +
          std::to_string(knot + 1) + ".");
    }

    std::cout << "Knoten " << knot + 1 << ": Farbe " << assignedColor
              << '\n';
  }
}

int main(int argc, char **argv) {
  try {
    if (argc != 2) {
      throw std::invalid_argument(
          "Verwendung: graphColor <DIMACS-Graph-Datei>");
    }

    std::cout << "Inkrementelles SAT-Programm gestartet.\n";
    std::cout << "IPASIR: " << Solver::signature() << '\n';
    
    //definiert das graph-File und die Anzahl der Knoten
    const std::filesystem::path programPath =
        std::filesystem::absolute(argc > 0 ? argv[0] : "graphColorSkeleton");
        setup(programPath, argv[1]);


    //Fügt Bedingung hinzu, dass jeder Knoten genau (XOR) 1 Farbe haben darf
    //(Eine Farbe aus Anzahl Farbe = knots)
    addAllColors();


    Solver &solver = solver_instance();
    for (colors = 1; colors <= knots; ++colors) {
      //Berechne den Graphen für immer eine Farbe mehr.
      //Für jede neue Farbe: diese darf zwischen zwei Knoten nicht die selbe sein:
      XORlastColor();
      //Aus addAllColors sind pro Knoten noch nicht bedachte Farben möglich.
      //Schließe diese mit assume() aus:
      limitColors();

      std::cout << "Prüfe die Färbbarkeit mit " << colors
                << " Farben.\n";
      const int result = solver.solve();
      if (result == 10) {
        std::cout << "SAT mit " << colors << " Farben.\n";
        printColors();
        return 0;
      }
      if (result != 20) {
        std::cerr << "UNKNOWN (IPASIR-Ergebnis " << result << ").\n";
        return 2;
      }
    }






    std::cerr << "Keine Färbung mit höchstens " << knots
              << " Farben gefunden.\n";
    return 1;
  } catch (const std::exception &error) {
    std::cerr << "Fehler: " << error.what() << '\n';
    return 2;
  }
}
