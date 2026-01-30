TARGET = app

CC = gcc
AS = gcc
CFLAGS = -Wall 
LDFLAGS = -lwiringPi

C_SRCS = main.c secuencias_propias.c secuencias.c 
S_SRCS = calculoLectura.s

OBJS = $(C_SRCS:.c=.o) $(S_SRCS:.s=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean	
