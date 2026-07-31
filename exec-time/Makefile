.PHONY: all run clean

# compilatore
CC = clang
CFLAGS = -O3 -mcpu=apple-m2

# codice 
SRC = stride.c
PLOT = plot.py

# oggetti
TARGET = stride.o
CSV = data.csv 

all: $(TARGET) run plot

$(TARGET): $(SRC)
	@$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	@./$(TARGET) $(S) $(ITERS) > $(CSV)

plot: $(CSV)
	@python3 $(PLOT) $(CSV)

clean:
	@rm -f $(TARGET) $(CSV)
