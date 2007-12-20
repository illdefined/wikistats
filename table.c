#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#include "hash.h"
#include "table.h"

#define flip(integer) (((integer) % 2) ? -1 : 1)

struct Entry *lookup(struct Table *table, const char *key) {
	register unsigned long hashValue;
	register struct Entry *entry;
	register size_t iter;

	hashValue = hash((uint8_t *) key, strlen(key)) % table->size;
	entry = table->data + hashValue;

	// Test if bucket is empty
	if (*(entry->key) == '\0') {
		strncpy(entry->key, key, sizeof (table->data->key));
		return entry;
	}

	// Check for collision
	if (!strncmp(key, entry->key, sizeof (table->data->key)))
		return entry;

	// Quadratic probing
	for (iter = 0; iter < 64; iter++) {
		entry = table->data + ((hashValue + flip(iter) * (iter/2) * (iter/2)) % table->size);

		// Test if bucket is empty
		if (*(entry->key) == '\0') {
			strncpy(entry->key, key, sizeof (table->data->key));
			return entry;
		}

		// Check for collision
		if (!strncmp(key, entry->key, sizeof (table->data->key)))
			return entry;
	}

	return (struct Entry *) 0;
}

int commit(struct Table *table, const char *key, unsigned long long int value) {
	register struct Entry *entry = lookup(table, key);

	if (entry == 0)
		return -1;

	entry->value += value;
	return 0;
}

int increment(struct Table *table, const char *key) {
	register struct Entry *entry = lookup(table, key);

	if (entry == 0)
		return -1;

	entry->value++;
	return 0;
}

int inject(struct Table *src, struct Table *dest) {
	struct Entry *iter = src->data;

	while (iter < src->data + src->size) {
		if (iter->value)
			if (commit(dest, (char *) iter->key, iter->value))
				return -1;
		iter++;
	}

	return 0;
}

int injresize(struct Table *src, struct Table *dest, const char *path, char *temp) {
	struct Entry *iter = src->data;

	while (iter < src->data + src->size) {
		if (iter->value)
			if (commit(dest, (char *) iter->key, iter->value)) {
				struct Table table;
				table.size = dest->size * 2;

				int fd = mkstemp(temp);
				if (fd < 0)
					return -1;

				if (ftruncate(fd, table.size * sizeof (struct Entry))) {
					close(fd);
					return -1;
				}

				table.data = mmap(0, table.size * sizeof (struct Entry), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
				if (table.data == (void *) -1) {
					close(fd);
					return -1;
				}

				close(fd);

				if (inject(dest, &table)) {
					munmap(table.data, table.size);
					return -1;
				}

				if (commit(&table, (char *) iter->key, iter->value)) {
					munmap(table.data, table.size);
					return -1;
				}

				if (munmap(dest->data, dest->size)) {
					munmap(table.data, table.size);
					return -1;
				}

				dest->data = table.data;
				dest->size = table.size;

				if (unlink(path))
					return -1;

				if (rename(temp, path))
					return -1;
			}
		iter++;
	}

	return 0;
}
