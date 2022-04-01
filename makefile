CC	= gcc
CFLAGS	= -Wall
LDFLAGS = 
OBJFILES	= appserver.o Bank.o 
TARGET	= appserv

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~