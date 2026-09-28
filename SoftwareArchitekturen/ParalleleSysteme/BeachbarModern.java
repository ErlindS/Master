import java.util.LinkedList;
import java.util.Queue;
import java.util.Random;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class BeachbarModern {

    /*
     * EXECUTOR SERVICE VERGLEICH (FixedThreadPool vs. CachedThreadPool):
     *
     * 1. FixedThreadPool(3): Beschränkt die Anzahl parallel aktiver Threads strikt
     * auf 3. Das entspricht
     * exakt unseren 3 physischen Mixern (Alex, Sam, Luca) und verhindert
     * Ressourcenüberlastung.
     *
     * 2. CachedThreadPool: Erzeugt bei Bedarf dynamisch neue Threads und baut
     * unbenutzte nach 60s Idle-Zeit ab.
     * Würden wir Einzelbestellungen direkt als Runnables ans Pool übergeben, würde
     * ein CachedThreadPool bei
     * hohem Gästeandrang beliebig viele Mixer-Threads gleichzeitig erzeugen – die
     * physische Begrenzung auf
     * 3 Mixer ginge dadurch verloren.
     */

    // Kompaktes Record für die Studierenden (Java 16+)
    record Student(int id, int anzahlCocktails) {
    }

    // Der gemeinsame Monitor mit ReentrantLock & Condition
    static class Theke {
        private final Queue<Student> warteschlange = new LinkedList<>();
        private final Lock lock = new ReentrantLock();
        private final Condition hatStudenten = lock.newCondition();
        private boolean alleAngekommen = false;

        public void reiheEin(Student student) {
            lock.lock();
            try {
                warteschlange.add(student);
                System.out.printf("[%s] Studierende:r %2d reiht sich ein (%d Cocktail(s)). [Warteschlange: %d]%n",
                        getTimestamp(), student.id(), student.anzahlCocktails(), warteschlange.size());

                // Signalisiert wartenden Mixern, dass ein neuer Student da ist (entspricht
                // notifyAll)
                hatStudenten.signalAll();
            } finally {
                lock.unlock(); // Garantierte Freigabe des Locks im finally-Block
            }
        }

        public Student nimmNaechstenStudenten() throws InterruptedException {
            lock.lock();
            try {
                // Warten mittels Condition.await() statt wait()
                while (warteschlange.isEmpty() && !alleAngekommen) {
                    hatStudenten.await();
                }

                if (warteschlange.isEmpty() && alleAngekommen) {
                    return null; // Feierabend-Signal
                }

                return warteschlange.poll();
            } finally {
                lock.unlock();
            }
        }

        public void signalisiereAlleAngekommen() {
            lock.lock();
            try {
                this.alleAngekommen = true;
                hatStudenten.signalAll(); // Alle schlafenden Mixer wecken zum Schichtende
            } finally {
                lock.unlock();
            }
        }
    }

    private static long startZeit;

    private static String getTimestamp() {
        long elapsedSec = (System.currentTimeMillis() - startZeit) / 1000;
        return String.format("%02d:%02d", elapsedSec / 60, elapsedSec % 60);
    }

    public static void main(String[] args) {
        startZeit = System.currentTimeMillis();
        System.out.println("=== Strand-Oase Modernisiertes Management gestartet ===");

        Theke theke = new Theke();
        Random random = new Random();

        // 1. ThreadPool mit genau 3 Threads für die 3 Mixer (Alex, Sam, Luca)
        ExecutorService mixerPool = Executors.newFixedThreadPool(3);

        // 2. Single-Thread-Executor für die Generierung der ankommenden Studierenden
        ExecutorService ankunftsPool = Executors.newSingleThreadExecutor();

        // Starten der 3 Mixer-Schichten über Lambdas
        String[] mixerNamen = { "Alex", "Sam", "Luca" };
        for (String name : mixerNamen) {
            mixerPool.submit(() -> starteMixerSchicht(name, theke, random));
        }

        // Ankunfts-Schleife als kompaktes Lambda
        ankunftsPool.submit(() -> {
            try {
                for (int i = 1; i <= 40; i++) {
                    int cocktails = 1 + random.nextInt(3);
                    theke.reiheEin(new Student(i, cocktails));

                    if (i < 40) {
                        Thread.sleep(2000); // 2 Sekunden Simulation = 2 Minuten Echtzeit
                    }
                }
                theke.signalisiereAlleAngekommen();
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        });

        // Einleitung des geordneten Herunterfahrens
        ankunftsPool.shutdown();
        mixerPool.shutdown();

        try {
            // Warten, bis alle Mix-Aufgaben abgearbeitet sind
            if (mixerPool.awaitTermination(10, TimeUnit.MINUTES)) {
                System.out.println("=== Simulation beendet: Alle 40 Studierenden wurden erfolgreich bedient! ===");
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }

    // Die eigentliche Schicht-Logik eines Mixers
    private static void starteMixerSchicht(String name, Theke theke, Random random) {
        try {
            while (true) {
                Student student = theke.nimmNaechstenStudenten();

                if (student == null) {
                    System.out.printf("[%s] Mixer %-4s: Feierabend! Keine weiteren Bestellungen.%n",
                            getTimestamp(), name);
                    break;
                }

                System.out.printf("[%s] Mixer %-4s übernimmt Studierende:n %2d (%d Cocktail(s)).%n",
                        getTimestamp(), name, student.id(), student.anzahlCocktails());

                for (int i = 1; i <= student.anzahlCocktails(); i++) {
                    int mixDauerMs = 2000 + random.nextInt(2001); // 2.0 bis 4.0 Sekunden
                    Thread.sleep(mixDauerMs);

                    System.out.printf("[%s] Mixer %-4s: Cocktail %d/%d für Studierende:n %2d fertig (Dauer: %.1fs).%n",
                            getTimestamp(), name, i, student.anzahlCocktails(), student.id(), mixDauerMs / 1000.0);
                }

                System.out.printf("[%s] Mixer %-4s: Bestellung von Studierende:m %2d VOLLSTÄNDIG bedient!%n",
                        getTimestamp(), name, student.id());
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            System.out.println("Mixer " + name + " wurde unterbrochen.");
        }
    }
}