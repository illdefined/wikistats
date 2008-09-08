CFLAGS ?= -Os -fomit-frame-pointer
CFLAGS += -Wall -Wextra -Werror

all: devour inject spawn vomit

devour: devour.c hash.c fileio.c parse.c table.c urldecode.c
	${CC} ${CFLAGS} -o $@ $^
	strip $@

inject: inject.c hash.c table.c
	${CC} ${CFLAGS} -o $@ $^
	strip $@

spawn: spawn.c
	${CC} ${CFLAGS} -o $@ $^
	strip $@

vomit: vomit.c hash.c table.c
	${CC} ${CFLAGS} -o $@ $^
	strip $@

clean:
	rm -f devour inject spawn vomit


.PHONY: all clean
