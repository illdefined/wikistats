include config.mk

SRC = wikistats.c devour.c vomit.c hash.c table.c urldecode.c

wikistats: ${SRC}
	${CC} ${CFLAGS} -o $@ ${SRC} ${LDFLAGS}
	strip $@

clean:
	rm -f wikistats

.PHONY: all clean
