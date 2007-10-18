CFLAGS ?= -Os -fomit-frame-pointer
CFLAGS += -Wall -Wextra -Werror

DEVOUR = devour.c hash.c parse.c table.c urldecode.c
INJECT = inject.c hash.c table.c
SPAWN  = spawn.c
VOMIT  = vomit.c hash.c table.c

all: devour inject spawn vomit

devour: ${DEVOUR}
	${CC} ${CFLAGS} -o $@ ${DEVOUR}
	strip $@

inject: ${INJECT}
	${CC} ${CFLAGS} -o $@ ${INJECT}
	strip $@

spawn: ${SPAWN}
	${CC} ${CFLAGS} -o $@ ${SPAWN}
	strip $@

vomit: ${VOMIT}
	${CC} ${CFLAGS} -o $@ ${VOMIT}
	strip $@

clean:
	rm -f devour inject vomit

.PHONY: all clean
