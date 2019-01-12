OBJS   = arglist.o module.o i2c.o asusaura.o compyglow.o main.o
TARGET = testglow
CFLAGS = -Wall -Wextra -ggdb -Og
LDFLAGS = -li2c

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
