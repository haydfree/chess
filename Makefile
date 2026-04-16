CC=			cc
CPPFLAGS=	
CFLAGS=		-std=c99 -g -O2 -Wall -Werror -Wextra -pedantic -fno-omit-frame-pointer
LIBS=		-lraylib -lm
LDFLAGS=	
TARGET=		chess
SRC=		main.c chess.c	
OBJ=		${SRC:.c=.o}

all: ${TARGET}

${OBJ}: %.o: %.c
	${CC} ${CPPFLAGS} ${CFLAGS} -c $< -o $@

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LIBS} ${LDFLAGS}

clean:
	rm -f ${TARGET} ${OBJ}

run: ${TARGET}
	./$<

valgrind: ${TARGET}
	valgrind --tool=callgrind --instr-atstart=no ./$<

.PHONY: all clean run
