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

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "table.h"
#include "version.h"

int main(int argc, char *argv[]) {
	struct Entry entry;
	struct Table table;

	const char *path = DEF_DB;
	int handle;
	struct stat statbuf;
	unsigned long long int minimum = 1ull;

	long int pageSize;

	register int iter;

	for (iter = 1; iter < argc; iter++) {
		if (argv[iter][0] == '-'
		 && argv[iter][1] != '\0'
		 && argv[iter][2] == '\0') {
			switch (argv[iter][1]) {
				case 'd':
				 if (++iter >= argc) {
				 	fputs("You must specify a path!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 path = argv[iter];
				 break;

				case 'h':
				 printf(USAGE OPT_D OPT_H OPT_M OPT_V, argv[0]);
				 return EXIT_SUCCESS;

				case 'm':
				 if (++iter >= argc) {
				 	fputs("You must specify a number!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 minimum = strtoull(argv[iter], (char **) 0, 10);
				 break;

				case 'v':
				 printf(VERSION_STRING, VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
				 return EXIT_SUCCESS;

				default:
				 fprintf(stderr, "Invalid command line option -%c!\n", argv[iter][1]);
				 return EXIT_FAILURE;
			}
		}
		else {
			fprintf(stderr, "Unknown command line argument %s!\n", argv[iter]);
			return EXIT_FAILURE;
		}
	}

	pageSize = sysconf(_SC_PAGESIZE);
	if (pageSize < 0) {
		perror("sysconf");
		return EXIT_FAILURE;
	}

	handle = open(path, O_RDWR);
	if (handle < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	if(fstat(handle, &statbuf)) {
		perror("fstat");
		return EXIT_FAILURE;
	}

	table.size = statbuf.st_size / sizeof (struct Entry);
	table.data = mmap(0, align(storsize(table), (unsigned long) pageSize),
		PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (table.data == MAP_FAILED) {
		perror("mmap");
		return EXIT_FAILURE;
	}

	if (close(handle))
		perror("close");

	while (read(0, &entry, sizeof (entry)) == sizeof (entry)) {
		if(entry.value >= minimum) {
			if(commit(table, entry.key, entry.value)) {
				perror("commit");
				return EXIT_FAILURE;
			}
		}
	}

	return EXIT_SUCCESS;
}
