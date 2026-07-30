CC = clang
CFLAGS = -O2

TARGET = stride
SRC = stride.c

CSV = csv.txt
PLOT = plot.py

S ?= 16
ITERS ?= 1000000

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC)
	@$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	@./$(TARGET) $(S) $(ITERS) > $(CSV)
	@python3 $(PLOT)

clean:
	@rm -f $(TARGET) $(CSV)