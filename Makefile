SRC = cwikistats.c hash.c parse.c urldecode.c
OBJ = ${SRC:.c=.o}

CFLAGS ?= -Os

all: cwikistats

.c.o:
	@echo " CC $<"
	@${CC} -c -Wall,extra,error ${CFLAGS} $<

${OBJ}:

cwikistats: ${OBJ}
	@echo " CC -o $@"
	@${CC} -o $@ ${OBJ} ${LDFLAGS}
	@strip $@

clean:
	@echo cleaning
	@rm -f cwikistats ${OBJ}

.PHONY: all clean
