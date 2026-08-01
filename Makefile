.PHONY: format

# compilatore e flags
CC := clang
CFLAGS := -O3

# script per il plotting
PLOT := python plot.py
TRIES := 100

# compila un esperimento o un test
comp-%:
	@echo "Compilo esperimento" $* "..."
	@$(CC) $(CFLAGS) $*/main.c -o $*/main.o

# fa il plotting di un esperimento o un test
plot-%:
	@echo "Stampo esperimento" $* "..."
	@$(PLOT) $* $(TRIES)

# formatta tutto il sorgente
format:
	@echo "Formatting sources..."
	@find . \( -name '*.c' -o -name '*.h' \) -exec clang-format -i {} \;
