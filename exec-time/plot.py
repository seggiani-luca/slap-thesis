import csv
import matplotlib.pyplot as plt

# inizializza vettori dei tempi
iters = []
link_times = []
rand_times = []

# popola vettori dei tempi dal csv
with open("csv.txt", newline="") as f:
    reader = csv.reader(f)
    for row in reader:
        iters.append(int(row[0]))
        link_times.append(int(row[1]))
        rand_times.append(int(row[2]))

plt.scatter(iters, link_times, label="Collegati")
plt.scatter(iters, rand_times, label="Casuali")
plt.xlabel("Numero iterazioni")
plt.ylabel("Tempo di esecuzione")
plt.legend()
plt.show()
