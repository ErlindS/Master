import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;

public class BeachbarSimulation {

    // Repräsentiert eine:n Studierende:n mit ID und Anzahl bestellter Cocktails
    static class Student {
        private final int id;
        private final int anzahlCocktails;

        public Student(int id, int anzahlCocktails) {
            this.id = id;
            this.anzahlCocktails = anzahlCocktails;
        }

        public int getId() {
            return id;
        }

        public int getAnzahlCocktails() {
            return anzahlCocktails;
        }
    }

    // Der gemeinsame Monitor für die Synchronisation
    static class Theke {
        private final Queue<Student> warteschlange = new LinkedList<>();
        private boolean alleAngekommen = false;

        // Erzeuger-Methode: Studierende:r reiht sich an der Theke ein
        public synchronized void reiheEin(Student student) {
            warteschlange.add(student);
            System.out.printf("[%s] Studierende:r %2d reiht sich ein (%d Cocktail(s)). [Warteschlange: %d]%n",
                    getTimestamp(), student.getId(), student.getAnzahlCocktails(), warteschlange.size());
            
            // Wecke wartende Mixer-Threads auf
            notifyAll();
        }

        // Verbraucher-Methode: Mixer holt den/die nächste:n Studierende:n
        public synchronized Student nimmNaechstenStudenten() throws InterruptedException {
            // Solange die Schlange leer ist und noch Studierende nachkommen -> Warten
            while (warteschlange.isEmpty() && !alleAngekommen) {
                wait();
            }

            // Wenn alle bedient wurden und niemand mehr wartet -> Signal zum Beenden
            if (warteschlange.isEmpty() && alleAngekommen) {
                return null;
            }

            return warteschlange.poll();
        }

        // Signalisiert, dass alle 40 Studierenden angekommen sind
        public synchronized void signalisiereAlleAngekommen() {
            this.alleAngekommen = true;
            // Alle eventuell wartenden Mixer wecken, damit sie die Schleife beenden können
            notifyAll();
        }
    }

    // Mixer-Thread (Alex, Sam, Luca)
    static class Mixer implements Runnable {
        private final String name;
        private final Theke theke;
        private final Random random = new Random();

        public Mixer(String name, Theke theke) {
            this.name = name;
            this.theke = theke;
        }

        @Override
        public void run() {
            try {
                while (true) {
                    Student student = theke.nimmNaechstenStudenten();
                    
                    // Abbruchbedingung: Keine Studierenden mehr da und alle angekommen
                    if (student == null) {
                        System.out.printf("[%s] Mixer %-4s: Feierabend! Keine weiteren Bestellungen.%n",
                                getTimestamp(), name);
                        break;
                    }

                    System.out.printf("[%s] Mixer %-4s übernimmt Studierende:n %2d (%d Cocktail(s)).%n",
                            getTimestamp(), name, student.getId(), student.getAnzahlCocktails());

                    // Mixer ist für die gesamte Dauer dieser Bestellung belegt
                    for (int i = 1; i <= student.getAnzahlCocktails(); i++) {
                        // Zufällige Mixdauer pro Cocktail: 2 bis 4 Sekunden (2000ms bis 4000ms)
                        int mixDauerMs = 2000 + random.nextInt(2001);
                        Thread.sleep(mixDauerMs);

                        System.out.printf("[%s] Mixer %-4s: Cocktail %d/%d für Studierende:n %2d fertig (Dauer: %.1fs).%n",
                                getTimestamp(), name, i, student.getAnzahlCocktails(), student.getId(), mixDauerMs / 1000.0);
                    }

                    System.out.printf("[%s] Mixer %-4s: Bestellung von Studierende:m %2d VOLLSTÄNDIG bedient!%n",
                            getTimestamp(), name, student.getId());
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.out.println("Mixer " + name + " wurde unterbrochen.");
            }
        }
    }

    // Zeitmessung für die Konsolenausgabe
    private static long startZeit;

    private static String getTimestamp() {
        long elapsedSec = (System.currentTimeMillis() - startZeit) / 1000;
        return String.format("%02d:%02d", elapsedSec / 60, elapsedSec % 60);
    }

    public static void main(String[] args) {
        startZeit = System.currentTimeMillis();
        System.out.println("=== Strand-Oase Simulation gestartet ===");

        Theke theke = new Theke();
        Random random = new Random();

        // 3 Mixer-Threads erstellen und starten
        Thread mixerAlex = new Thread(new Mixer("Alex", theke));
        Thread mixerSam  = new Thread(new Mixer("Sam",  theke));
        Thread mixerLuca = new Thread(new Mixer("Luca", theke));

        mixerAlex.start();
        mixerSam.start();
        mixerLuca.start();

        // Ankunft-Thread: Simuliert das Eintreffen der 40 Studierenden alle 2 Sekunden
        Thread ankunftsThread = new Thread(() -> {
            try {
                for (int i = 1; i <= 40; i++) {
                    int anzahlCocktails = 1 + random.nextInt(3); // 1 bis 3 Cocktails
                    theke.reiheEin(new Student(i, anzahlCocktails));

                    // Alle 2 Sekunden (entspricht 2 Minuten in der Simulation) kommt der/die Nächste
                    if (i < 40) {
                        Thread.sleep(2000);
                    }
                }
                theke.signalisiereAlleAngekommen();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        ankunftsThread.start();

        // Warten, bis alle Threads fertig sind
        try {
            ankunftsThread.join();
            mixerAlex.join();
            mixerSam.join();
            mixerLuca.join();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }

        System.out.println("=== Simulation beendet: Alle 40 Studierenden wurden erfolgreich bedient! ===");
    }
}