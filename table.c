#include <errno.h>
#include <string.h>

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

	errno = ENOMEM;

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

long int inject(struct Table *src, struct Table *dest) {
	register struct Entry *iter = src->data;

	while (iter < src->data + src->size) {
		if (iter->value)
			if (commit(dest, (char *) iter->key, iter->value))
				return -(iter - src->data);
		iter++;
	}

	return 0;
}
