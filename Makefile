CFLAGS ?= -Os -fomit-frame-pointer
CFLAGS += -Wall -Wextra -Werror

DEVOUR = devour.c hash.c parse.c table.c urldecode.c
INJECT = inject.c hash.c table.c
VOMIT  = vomit.c hash.c table.c

all: devour inject vomit

devour: ${DEVOUR}
	${CC} ${CFLAGS} -o $@ ${DEVOUR}
	strip $@

inject: ${INJECT}
	${CC} ${CFLAGS} -o $@ ${INJECT}
	strip $@

vomit: ${VOMIT}
	${CC} ${CFLAGS} -o $@ ${VOMIT}
	strip $@

clean:
	rm -f devour inject vomit

.PHONY: all clean
