CC	= gcc
CFLAGS	= -Wall -pthread
LDFLAGS = 
OBJFILES	= appserver.c Bank.c
TARGET	= appserv

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~