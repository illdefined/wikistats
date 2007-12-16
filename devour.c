#include <pthread.h>
#include <string.h>

#include <buffer.h>

#include "table.h"
#include "urldecode.h"

#define skip(id, delim) \
	loop_##id: \
	if (idx >= len) \
		continue; \
	if (buf[idx++] != delim) \
		goto loop_##id;

#define save(str, delim) \
	str = buf + idx; \
	loop_##str: \
	if (idx >= len) \
		continue; \
	if (buf[idx++] != delim) \
		goto loop_##str; \
	else \
		buf[idx - 1] = '\0';

static struct Table *cache;

void *parser(void *data) {
	char buf[2048];
	char *url;
	int len;

	while ((len = buffer_getline(buffer_1, buf, sizeof buf))) {
		unsigned short idx = 0;

		skip(host, ' '); /* Host name */
		skip(seqc, ' '); /* Sequence */
		skip(time, ' '); /* Time */
		skip(rqtm, ' '); /* Request time */
		skip(addr, ' '); /* IP address */
		skip(sqst, '/'); /* Squid status */

		if (buf[idx++] != '2') /* only 2XX requests */
			continue;

		skip(htst, ' '); /* HTTP status */

		if (buf[idx++] != 'G') /* only GET requests */
			continue;

		skip(meth, ' '); /* Request method */
		save(url, ' ');

		urldecode((unsigned char *) url);

		/* Commit to cache and flush it if necessary */
		if (increment(cache, url)) {
			/* ... */
		}
	}

	return (void *) 0;
}

int devour(int argc, char *argv[]) {
	/* ... */

	return 0;
}
