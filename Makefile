.PHONY: format

# compilatore e flags
CC := clang
CFLAGS := -O3
LFLAGS := -ldl

# script per il plotting
PLOT := python3 plot.py
TRIES := 100

# utility
UTILS := utils/kperf.c utils/utils.c

# compila un esperimento o un test
comp-%:
	@echo "Compilo esperimento" $* "..."
	@$(CC) $(CFLAGS) $(UTILS) $*/main.c -o $*/main.o $(LFLAGS)

# fa il plotting di un esperimento o un test
plot-%:
	@echo "Stampo esperimento" $* "..."
	@$(PLOT) $* $(TRIES)

# formatta tutto il sorgente
format:
	@echo "Formatting sources..."
	@find . \( -name '*.c' -o -name '*.h' \) -exec clang-format -i {} \;
