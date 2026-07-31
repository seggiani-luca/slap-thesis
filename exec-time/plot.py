import csv
import matplotlib.pyplot as plt
import sys

# Ottieni argomenti
if(len(sys.argv) < 2):
    print("Troppi pochi argomenti")
    exit()
csv_file = sys.argv[1]

# Numero di iterazioni per ciascun test
iters = []

# Tempi misurati per accessi ad array collegati 
link_times = []

# Tempi misurati per accessi ad array casuali 
rand_times = []

# Tempi misurati per accessi ad array sa+rv 
sarv_times = []

# Legge i dati dal file CSV.
# Ogni riga è nel formato:
#     ITERS, link_time, random_time
with open("data.csv", newline="") as f:
    reader = csv.reader(f)
    for row in reader:
        iters.append(int(row[0]))
        link_times.append(int(row[1]))
        rand_times.append(int(row[2]))
        sarv_times.append(int(row[3]))

# Grafico STRIDE vs RANDOM
# Linee verticali
plt.figure()

plt.xticks(iters)
plt.grid(axis="x", alpha=0.3)

# Visualizza i dati come grafico a dispersione
plt.plot(iters, link_times, label="Collegati")
plt.scatter(iters, rand_times, label="Casuali")

# Etichette degli assi
plt.xlabel("Numero iterazioni")
plt.ylabel("Tempo di esecuzione (ns)")
plt.title("Stride vs Random")

# Mostra la legenda e il grafico
plt.legend()
plt.show()

# Grafico SARV vs RANDOM
plt.figure()

plt.xticks(iters)
plt.grid(axis="x", alpha=0.3)

plt.plot(iters, sarv_times, label="SA+RV")
plt.scatter(iters, rand_times, label="Random")

plt.xlabel("Numero iterazioni")
plt.ylabel("Tempo di esecuzione (ns)")
plt.title("SA+RV vs Random")

plt.legend()
plt.show()