SRC = cwikistats.c hash.c urldecode.c
OBJ = ${SRC:.c=.o}

CFLAGS ?= -Os
PREFIX ?= /usr/local

all: cwikistats

.c.o:
	@echo CC $<
	@${CC} -c -Wall -std=c99 -pedantic ${CFLAGS} $<

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
