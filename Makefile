CC = gcc

ODIR = ./bin

src = $(wildcard src/*.c)
obj = $(src:.c=.o)

libfunc.a: $(src)
	$(CC) $(src) -o $(ODIR)/bf2c

.PHONY: clean

clean:
	rm $(ODIR)/bf2c
