/*
 * Copyright © MMVII
 * 	Mikael Voss <ffefd6 at haemoglobin dot org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un-
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided “AS IS” and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person's immediate fault when using the work as intended.
 */

#ifndef HASH_H
#define HASH_H

#define storsize(table) (table.size * sizeof (struct Entry))

struct Entry {
	unsigned char key[512 - sizeof (unsigned long long int)];
	unsigned long long int value;
};

struct Table {
	struct Entry *data;
	unsigned long int size;
};

inline struct Entry *lookup(struct Table, const char *);
int commit(struct Table, const char *, unsigned long long int);
int increase(struct Table, const char *);
int inject(struct Table, struct Table);

#endif
