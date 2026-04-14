CC=			cc
CPPFLAGS=	
CFLAGS=		-std=c99 -g3 -O0 -Wall -Werror -Wextra -pedantic
LIBS=	
LDFLAGS=	
TARGET=		chess_engine
SRC=		board.c  evaluation.c  game.c  main.c  memory_debug.c  move_generation.c  play.c  test.c  uint64.c
INC=		chess_engine.h memory_debug.h
OBJ=		${SRC:.c=.o}

all: ${TARGET}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -c $<

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LIBS} ${LIDFLAGS}

clean:
	rm -f ${TARGET} ${OBJ}

run: ${TARGET}
	./$<

.PHONY: all clean run
