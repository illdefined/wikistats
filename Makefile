SRC = aux.c cwikistats.c hash.c urldecode.c
OBJ = ${SRC:.c=.o}

CFLAGS ?= -Os -Wall -std=c99 -pedantic
PREFIX ?= /usr/local

all: cwikistats

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}:

cwikistats: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}
	@strip $@

install: all
	@echo INSTALL cwikistats
	@install -m 755 cwikistats ${PREFIX}/bin

clean:
	@echo cleaning
	@rm -f cwikistats ${OBJ}

.PHONY: all install clean
