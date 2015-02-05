
.PHONY : clean

CC ?= gcc

SRC= \
src/atf2png.c

CFLAG=-O2 -Wall -g

all :
	$(CC) -o atf2png $(SRC) $(CFLAG)

clean :
	rm -f atf2png.exe