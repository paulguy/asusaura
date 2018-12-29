OBJS   = asusaura.o main.o
TARGET = testaura
CFLAGS = -Wall -Wextra -ggdb -Og
LDFLAGS = -li2c

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
