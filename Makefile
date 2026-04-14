CC=			cc
CPPFLAGS=	
CFLAGS=		-std=c99 -g3 -O0 -Wall -Werror -Wextra -pedantic
LIBS=	
LDFLAGS=	
TARGET=		chess
SRC=		main.c chess.c	
INC=		
OBJ=		${SRC:.c=.o}

all: ${TARGET}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -c $<

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LIBS} ${LDFLAGS}

clean:
	rm -f ${TARGET} ${OBJ}

run: ${TARGET}
	./$<

.PHONY: all clean run
