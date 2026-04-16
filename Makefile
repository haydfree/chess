CC=		cc
CPPFLAGS=	
DBGFLAGS=	-g -fsanitize=address -fno-omit-frame-pointer
CFLAGS=		-O0 -std=c99 -Wall -Werror -Wextra -pedantic ${DBGFLAGS}
LIBS=		-lraylib -lm -lasan
LDFLAGS=	${LIBS} -fsanitize=address -fno-omit-frame-pointer
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
