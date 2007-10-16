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

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"
#include "version.h"

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

static struct Table table = { (struct Entry *) 0, 2097152 };
static struct Table cache = { (struct Entry *) 0, 131072 };

void signal_handler(int sig) {
	switch (sig) {
		case SIGTERM:
		 inject(cache, table);
		 exit(EXIT_FAILURE);

		case SIGUSR1:
		 inject(cache, table);
		 break;
	}
}

int main(int argc, char *argv[]) {
	char *path = DEF_DB;
	int handle;

	size_t bufsize = 1024;
	char *buffer;

	long int pageSize;

	register int iter;

	for (iter = 1; iter < argc; iter++) {
		if (argv[iter][0] == '-'
		 && argv[iter][1] != '\0'
		 && argv[iter][2] == '\0') {
			switch (argv[iter][1]) {
				case 'b':
				 if (++iter >= argc) {
				 	fputs(REQ_NUM, stderr);
				 	return EXIT_FAILURE;
				 }
				 bufsize = strtoul(argv[iter], (char **) 0, 0);
				 break;

				case 'c':
				 if (++iter >= argc) {
				 	fputs(REQ_NUM, stderr);
				 	return EXIT_FAILURE;
				 }
				 cache.size = strtoul(argv[iter], (char **) 0, 0);
				 break;

				case 'd':
				 if (++iter >= argc) {
				 	fputs(REQ_PATH, stderr);
				 	return EXIT_FAILURE;
				 }
				 path = argv[iter];
				 break;

				case 'h':
				 printf(USAGE OPT_B OPT_C OPT_D OPT_H OPT_N OPT_V, argv[0]);
				 return EXIT_SUCCESS;

				case 'n':
				 if (++iter >= argc) {
				 	fputs(REQ_NUM, stderr);
				 	return EXIT_FAILURE;
				 }
				 table.size = strtoul(argv[iter], (char **) 0, 0);
				 break;

				case 'v':
				 printf(VERSION_STRING, VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
				 return EXIT_SUCCESS;

				default:
				 fprintf(stderr, INV_OPT, argv[iter][1]);
				 return EXIT_FAILURE;
			}
		}
		else {
			fprintf(stderr, INV_ARG, argv[iter]);
			return EXIT_FAILURE;
		}
	}

	pageSize = sysconf(_SC_PAGESIZE);
	if (pageSize < 0) {
		perror("sysconf");
		return EXIT_FAILURE;
	}

	// Attempt to lock all memory to avoid paging of perfomance critical stuff
	if (mlockall(MCL_CURRENT | MCL_FUTURE))
		perror("mlockall");

	// Open database file
	handle = open(path, O_RDWR | O_CREAT, PERMS);
	if (handle < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	// Truncate file to appropriate size
	if (ftruncate(handle, align(storsize(table), (unsigned long int) pageSize))) {
		perror("ftruncate");
		return EXIT_FAILURE;
	}

	// Establish mapping
	table.data = mmap(0, align(storsize(table), (unsigned long) pageSize),
		PROT_READ | PROT_WRITE, MAP_SHARED, handle, 0);
	if (table.data == MAP_FAILED) {
		perror("mmap");
		return EXIT_FAILURE;
	}

	if (close(handle))
		perror("close");

	buffer = malloc(bufsize);
	if (!buffer) {
		perror("malloc");
		return EXIT_FAILURE;
	}

	cache.data = malloc(storsize(cache));
	if (!cache.data) {
		perror("malloc");
		return EXIT_FAILURE;
	}

	memset(cache.data, 0, storsize(cache));

	signal(SIGTERM, signal_handler);
	signal(SIGUSR1, signal_handler);

	if(parse(table, cache, buffer, bufsize)) {
		perror("parse");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
