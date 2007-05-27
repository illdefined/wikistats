SRC = aux.c cwikistats.c hash.c log.c urldecode.c
OBJ = ${SRC:.c=.o}

CFLAGS += -Os -Wall

all: cwikistats

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}:

cwikistats: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}
	@strip $@

clean:
	@echo cleaning
	@rm -f cwikistats ${OBJ}

.PHONY: all clean
