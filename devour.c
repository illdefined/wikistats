#include <pthread.h>
#include <stdlib.h>
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

static char *path;
static struct Table *table;
static struct Table *flush_cache;
static pthread_mutex_t flush_mutex;
static pthread_cond_t flush_cond;

void *flush(void *data) {
loop:
	pthread_mutex_lock(&flush_mutex);
	pthread_cond_wait(&flush_cond, &flush_mutex);

	char temp[] = "wikistats.XXXXXX";
	if (injresize(flush_cache, table, path, temp))
		exit(1);

	memset(flush_cache->data, 0, flush_cache->size * sizeof (struct Entry));

	pthread_mutex_unlock(&flush_mutex);

	goto loop;
}

static void xchg(struct Table *t1, struct Table *t2) {
	struct Table *tmp;

	tmp = t1;
	t1 = t2;
	t2 = tmp;
}

int devour(int argc, char *argv[]) {
	struct Table *cache;

	/* TODO: Initialise mutex and condition, allocate caches and mmap table */

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
			pthread_mutex_lock(&flush_mutex);
			xchg(cache, flush_cache);
			pthread_mutex_unlock(&flush_mutex);
			pthread_cond_signal(&flush_cond);

			increment(cache, url);
		}
	}

	pthread_mutex_lock(&flush_mutex);
	xchg(cache, flush_cache);
	pthread_mutex_unlock(&flush_mutex);
	pthread_cond_signal(&flush_cond);

	/* Wait for flush thread to finish */
	pthread_mutex_lock(&flush_mutex);

	return 0;
}
