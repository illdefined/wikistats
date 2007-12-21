#include <stdbool.h>
#include <unistd.h>

#include <buffer.h>
#include <mmap.h>

#include "table.h"

static unsigned long int len(const char *str, unsigned long int max) {
	unsigned long int iter = 0;

	while (iter < max && str[iter] != '\0')
		iter++;

	return iter;
}

int vomit(int argc, char *argv[]) {
	struct Entry *table;
	unsigned long int size, iter = 0;

	if (argc != 2)
		return -1;

	table = (struct Entry *) mmap_read(argv[1], &size);
	if (table == (struct Entry *) 0)
		return -1;

	size /= sizeof (struct Entry);

	while (iter <= size) {
		if (table[iter].value != 0) {
			buffer_putulonglong(buffer_1, table[iter].value);
			buffer_putspace(buffer_1);
			buffer_put(buffer_1, table[iter].key, len(table[iter].key, sizeof (table->key)));
			buffer_putnlflush(buffer_1);
		}
		iter++;
	}

	return 0;
}
