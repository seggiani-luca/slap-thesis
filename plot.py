#import csv
#import sys
#import subprocess
#import matplotlib.pyplot as plt
#import statistics
#from io import StringIO
#
## Lancia un singolo processo e ne ottiene l'STDOUT
#def run_proc(exp_dir):
#    # lancia il processo
#    p = subprocess.Popen(
#       ["./main.o"],
#        cwd=exp_dir,
#        stdout=subprocess.PIPE,
#        text=True
#    )
#
#    # aspetta la terminazione
#    stdout, _ = p.communicate()
#
#    # restituisci l'STDOUT
#    return stdout
#
## Legge un file CSV e restituisce titoli e dati
#def parse_csv(csv_str):
#    # leggi CSV
#    reader = csv.reader(StringIO(csv_str))
#
#    # leggi prima i titoli
#    headers = next(reader)
#
#    # leggi i dati
#    data = [[] for _ in headers]
#
#    for row in reader:
#        for i, value in enumerate(row):
#            data[i].append(float(value))
#
#    return headers, data
#
## ottieni argomenti
#if(len(sys.argv) < 2):
#    print("Troppi pochi argomenti")
#    exit()
#exp_dir = sys.argv[1]
#
## esegui esperimento
#out = run_proc(exp_dir)
#headers, data = parse_csv(out)
#
## stampa grafico
#plt.figure()
#
## stampa ogni colonna
#for col in range(len(headers)):
#    if col == 0:
#        continue
#    plt.plot(data[0], data[col], label=headers[col])
#
#plt.xlabel(headers[0])
#
#plt.legend()
#plt.show()

import csv
import matplotlib.pyplot as plt

x = []
y1 = []
y2 = []
y3 = []

with open("data.csv", newline="") as f:
    reader = csv.reader(f)

    for row in reader:
        x.append(float(row[0]))
        y1.append(float(row[1]))
        y2.append(float(row[2]))
        y3.append(float(row[3]))

plt.plot(x, y1, label="Grafico 1")
plt.plot(x, y2, label="Grafico 2")
plt.plot(x, y3, label="Grafico 3")

plt.xlabel("Iterazioni")
plt.ylabel("Tempo")
plt.legend()
plt.grid()

plt.show()