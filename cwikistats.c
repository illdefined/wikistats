/*
 * Copyright © MMVII
 * 	Mikael Voss <ffefd6 at haemoglobin dot org>
 * 	Leon Weber <leon at leonweber dot de>
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

// Get rid of these annoying warning when using the bloated GNU libc
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "hash.h"
#include "log.h"
#include "parse.h"
#include "version.h"

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

static long int pageSize;

inline static unsigned long int align(unsigned long int size, unsigned long int alignment) {
	return (size + alignment - 1ul) & ~(alignment - 1ul);
}

int main(int argc, char *argv[]) {
	struct Table table = { (struct Entry *) 0, 2097152 };
	struct Table cache = { (struct Entry *) 0, 131072 };
	char *path = "/var/db/wikistats/database";
	int handle;

	size_t bufsize = 1024;
	char *buffer;

	int dump = 0;
	unsigned long int minimum = 1;

	register size_t iter;

	for (iter = 1; iter < argc; iter++) {
		if (argv[iter][0] == '-'
		 && argv[iter][1] != '\0'
		 && argv[iter][2] == '\0') {
			switch (argv[iter][1]) {
				case 'b':
				 if (++iter >= argc) {
				 	fputs("You must specify a number!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 bufsize = strtoul(argv[iter], (char **) 0, 0);
				 break;

				case 'c':
				 if (++iter >= argc) {
				 	fputs("You must specify a number!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 // No checking needed - if entries is invalid, it will fail anyway!
				 cache.size = strtoul(argv[iter], (char **) 0, 0);
				 break;

				case 'd':
				 if (++iter >= argc) {
				 	fputs("You must specify a path!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 path = argv[iter];
				 break;

				case 'h':
				 printf("usage: %s [options]\n"
				  "  -b num    Input buffer size\n"
				  "  -c num    Cache buckets\n"
				  "  -d path   Use path as database\n"
				  "  -h        Issue this help\n"
				  "  -l        Dump database\n"
				  "  -m num    Minimum\n"
				  "  -n num    Database buckets\n"
				  "  -v        Show version\n",
				  argv[0]);
				 return EXIT_SUCCESS;

				case 'l':
				 dump = 1;
				 break;

				case 'm':
				 if (++iter >= argc) {
				 	fputs("You must specify a number!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 minimum = strtoul(argv[iter], (char **) 0, 0);
				 break;

				case 'n':
				 if (++iter >= argc) {
				 	fputs("You must specify a number!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 // No checking needed - if entries is invalid, it will fail anyway!
				 table.size = strtoul(argv[iter], (char **) 0, 0);
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
	catch(pageSize < 0);

	// Open system log
	openlog("cwikistats", LOG_PERROR, LOG_DAEMON);

	// Attempt to lock all memory to avoid paging of perfomance critical stuff
	warn(
		mlockall(MCL_CURRENT | MCL_FUTURE)
	);

	// Open database file
	handle = open(path, O_RDWR | O_CREAT, PERMS);
	catch(handle < 0);

	// Truncate file to appropriate size
	catch(
		ftruncate(handle, align(storsize(table), (unsigned long int) pageSize))
	);

	// Establish mapping
	table.data = mmap(0, align(storsize(table), (unsigned long) pageSize),
		PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	catch(table.data == MAP_FAILED);

	warn(
		close(handle)
	);

	if (dump) {
		register struct Entry *current = table.data;
		while (current < table.data + table.size) {
			if (current->value > minimum)
				printf("%020llu %s\n", current->value, current->key);
			current++;
		}
		return EXIT_SUCCESS;
	}

	buffer = malloc(bufsize);
	catch(!buffer);

	cache.data = malloc(storsize(cache));
	catch(!cache.data);

	parse(table, cache, buffer, bufsize);

	return EXIT_SUCCESS;
}
