#ifndef TABLE_H
#define TABLE_H

#define storsize(buckets) ((buckets) * sizeof (struct Entry))

struct Entry {
	char key[512 - sizeof (unsigned long long int)];
	unsigned long long int value;
};

struct Table {
	struct Entry *data;
	unsigned long int size;
};

inline struct Entry *lookup(struct Table *, const char *);
int commit(struct Table *, const char *, unsigned long long int);
int increment(struct Table *, const char *);
int inject(struct Table *, struct Table *);

#endif
