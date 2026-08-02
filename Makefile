.PHONY: format

# compilatore e flags
CC := clang
CFLAGS := -O3 -mcpu=apple-m2 #-g
LFLAGS := -ldl

# script per il plotting
PLOT := python3 plot.py

# utility
#UTILS := utils/kperf.c#utils/utils.c 

# compila un esperimento o un test
comp-%:
	@echo "Compilo esperimento" $* "..."
	@$(CC) $(CFLAGS) $(UTILS) $*/main.c -o $*/main.o $(LFLAGS)

# fa il plotting di un esperimento o un test
plot-%:
	@echo "Stampo esperimento" $* "..."
	@$(PLOT) $*

# formatta tutto il sorgente
format:
	@echo "Formatting sources..."
	@find . \( -name '*.c' -o -name '*.h' \) -exec clang-format -i {} \;

run:
	for ((i = 10; i <= 1000; i += 10)); do \
		sudo ./exec-time/main.o $$i; \
	done > data.csv
	python3 plot.py