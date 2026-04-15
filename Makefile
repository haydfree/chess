CC=			cc
CPPFLAGS=	
CFLAGS=		-std=c99 -g3 -O0 -Wall -Werror -Wextra -pedantic
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

.PHONY: all clean run
