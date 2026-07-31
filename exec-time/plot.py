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

# Legge i dati dal file CSV.
# Ogni riga è nel formato:
#     ITERS, link_time, random_time
with open("data.csv", newline="") as f:
    reader = csv.reader(f)
    for row in reader:
        iters.append(int(row[0]))
        link_times.append(int(row[1]))
        rand_times.append(int(row[2]))

# Visualizza i dati come grafico a dispersione
plt.scatter(iters, link_times, label="Collegati")
plt.scatter(iters, rand_times, label="Casuali")

# Etichette degli assi
plt.xlabel("Numero iterazioni")
plt.ylabel("Tempo di esecuzione")

# Mostra la legenda e il grafico
plt.legend()
plt.show()
