CC = gcc
CFLAGS = -Wall -Wextra -g
SOURCES = utils.c fstiva.c fcoada.c functii.c main.c
EXE = tema2

build:
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXE)
clean:
	rm -f $(EXE)