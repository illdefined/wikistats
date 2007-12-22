#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include <buffer.h>
#include <mmap.h>
#include <scan.h>

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
static struct Table table;
static struct Table flush_cache;
static pthread_mutex_t flush_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t flush_cond = PTHREAD_COND_INITIALIZER;

void *flush(void *data) {
loop:
	pthread_mutex_lock(&flush_mutex);
	pthread_cond_wait(&flush_cond, &flush_mutex);

	char temp[] = "wikistats.XXXXXX";
	if (injresize(&flush_cache, &table, path, temp))
		exit(1);

	zero(&flush_cache);

	pthread_mutex_unlock(&flush_mutex);

	goto loop;
}

static void xchg(struct Table *t1, struct Table *t2) {
	struct Table tmp;

	tmp.data = t1->data;
	tmp.size = t1->size;
	t1->data = t2->data;
	t1->size = t2->size;
	t2->data = tmp.data;
	t2->size = tmp.size;
}

int devour(int argc, char *argv[]) {
	struct Table cache;

	if (argc != 3)
		return -1;

	scan_ulong(argv[2], &cache.size);
	if (cache.size < 1024ul) {
		buffer_putsflush(buffer_2, "Are you kidding?\n");
		return -1;
	}

	cache.data = mmap((void *) 0, cache.size * sizeof (struct Entry), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (cache.data == (struct Entry *) -1) {
		buffer_puterror(buffer_2);
		buffer_putnlflush(buffer_2);
		return -1;
	}

	flush_cache.size = cache.size;
	flush_cache.data = mmap((void *) 0, flush_cache.size * sizeof (struct Entry), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (flush_cache.data == (struct Entry *) -1) {
		buffer_puterror(buffer_2);
		buffer_putnlflush(buffer_2);
		return -1;
	}

	path = argv[1];
	table.data = (struct Entry *) mmap_shared(path, &table.size);
	table.size /= sizeof (struct Entry);

	if (table.size < 1024ul) {
		buffer_putsflush(buffer_2, "This is ridiculous.\n");
		return -1;
	}

	zero(&cache);
	zero(&flush_cache);

	pthread_t flush_thread;
	if (pthread_create(&flush_thread, (pthread_attr_t *) 0, flush, (void *) 0))
		return -1;

	/*
	 * Parser
	 */

	char buf[2048];
	char *url;
	int len;

	while ((len = buffer_getline(buffer_0, buf, sizeof buf)) != -1) {
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
		skip(size, ' '); /* Request size */

		if (buf[idx++] != 'G') /* only GET requests */
			continue;

		skip(meth, ' '); /* Request method */
		save(url, ' ');

		urldecode((unsigned char *) url);

		/* Commit to cache and flush it if necessary */
		if (increment(&cache, url)) {
			pthread_mutex_lock(&flush_mutex);
			xchg(&cache, &flush_cache);
			pthread_mutex_unlock(&flush_mutex);
			pthread_cond_signal(&flush_cond);

			increment(&cache, url);
		}
	}

	pthread_mutex_lock(&flush_mutex);
	xchg(&cache, &flush_cache);
	pthread_mutex_unlock(&flush_mutex);
	pthread_cond_signal(&flush_cond);

	/* Wait for flush thread to finish */
	pthread_mutex_lock(&flush_mutex);
	pthread_mutex_unlock(&flush_mutex);

	return 0;
}
