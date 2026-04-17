CC=		cc
CPPFLAGS=	
DBGFLAGS=	-g
CFLAGS=		-O0 -std=c99 -Wall -Werror -Wextra -pedantic ${DBGFLAGS}
LIBS=		-lraylib -lm
LDFLAGS=	${LIBS}
TARGET=		chess
SRC=		main.c chess.c	
OBJ=		${SRC:.c=.o}

all: ${TARGET}

${OBJ}: %.o: %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c $< -o $@

${TARGET}: ${OBJ}
	${CC} -o $@ $^ ${LDFLAGS}

clean:
	rm -f ${TARGET} ${OBJ}

run: ${TARGET}
	./$<

valgrind: ${TARGET}
	valgrind --tool=callgrind --instr-atstart=no ./$<

.PHONY: all clean run
