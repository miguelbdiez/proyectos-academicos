CC = gcc
CFLAGS = -Wall -Wextra -O2
# En Solaris (olivo) hacian falta estas dos: en Linux no.
#LIBS = -lsocket -lnsl
LIBS =

PROGS = servidor cliente

.PHONY: all clean kill limpiarBasura

all: $(PROGS)

servidor: servidor.o
	$(CC) $(CFLAGS) -o $@ servidor.o $(LIBS)

cliente: cliente.o
	$(CC) $(CFLAGS) -o $@ cliente.o $(LIBS)

clean:
	rm -f *.o $(PROGS)

kill:
	pkill -f ./servidor

limpiarBasura:
	rm -f *[*\]*.txt *.log
