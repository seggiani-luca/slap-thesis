import csv
import sys
import subprocess
import matplotlib.pyplot as plt
import statistics
from io import StringIO

# Lancia un singolo processo e ne ottiene l'STDOUT
def run_proc(exp_dir):
    # lancia il processo
    p = subprocess.Popen(
       ["./main.o"],
        cwd=exp_dir,
        stdout=subprocess.PIPE,
        text=True
    )

    # aspetta la terminazione
    stdout, _ = p.communicate()

    # restituisci l'STDOUT
    return stdout

# Legge un file CSV e restituisce titoli e dati
def parse_csv(csv_str):
    # leggi CSV
    reader = csv.reader(StringIO(csv_str))

    # leggi prima i titoli
    headers = next(reader)

    # leggi i dati
    data = [[] for _ in headers]

    for row in reader:
        for i, value in enumerate(row):
            data[i].append(float(value))

    return headers, data

# ottieni argomenti
if(len(sys.argv) < 3):
    print("Troppi pochi argomenti")
    exit()
exp_dir = sys.argv[1]
tries = int(sys.argv[2])

# effettua tries tentativi
headers = []
results = []
for _ in range(tries):
    out = run_proc(exp_dir)
    headers, data = parse_csv(out)
    results.append(data)

# inizializza array di colonne mediane
data = []
cols = len(headers)

# scorri ogni colonna
for col in range(cols):
    # ottieni il numero di righe della colonna
    rows = len(results[0][col])

    # inizializza colonna di mediane
    medians = []

    # scorri ogni riga
    for row in range(rows):
        # contenitore di tutti a quella riga
        values = []

        # popola il contenitore
        for tri in range(tries):
            values.append(results[tri][col][row])

        # imposta la colonna di mediane a quella riga
        medians.append(statistics.median(values))

    # imposta la colonna di mediane nell'array delle colonne
    data.append(medians)

# stampa grafico
plt.figure()

# stampa ogni colonna
for col in range(cols):
    if col == 0:
        continue
    plt.plot(data[0], data[col], label=headers[col])

plt.xlabel(headers[0])

plt.legend()
plt.show()
