CFLAGS	= -Wall -Wextra -Werror -fpic
LDFLAGS	= -shared

TARGET	= csvlib.so
SOURCES	= $(wildcard src/*.c)
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(TARGET) $(OBJECTS)