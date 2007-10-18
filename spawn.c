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

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "table.h"
#include "version.h"

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main(int argc, char *argv[]) {
	char *path = "";
	int handle;

	long int pageSize;
	unsigned long int buckets = 2097152;

	register int iter;

	for (iter = 1; iter < argc; iter++) {
		if (argv[iter][0] == '-'
		 && argv[iter][1] != '\0'
		 && argv[iter][2] == '\0') {
			switch (argv[iter][1]) {
				case 'd':
				 if (++iter >= argc) {
				 	fputs(REQ_PATH, stderr);
				 	return EXIT_FAILURE;
				 }
				 path = argv[iter];
				 break;

				case 'h':
				 printf(USAGE OPT_D OPT_H OPT_N OPT_V, argv[0]);
				 return EXIT_SUCCESS;

				case 'n':
				 if (++iter >= argc) {
				 	fputs(REQ_NUM, stderr);
				 	return EXIT_FAILURE;
				 }
				 buckets = strtoul(argv[iter], (char **) 0, 0);
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

	handle = open(path, O_WRONLY | O_CREAT, PERMS);
	if (handle < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	// Truncate file to appropriate size
	if (ftruncate(handle, 0)
	    || ftruncate(handle, align(buckets * sizeof (struct Entry), (unsigned long int) pageSize))) {
		perror("ftruncate");
		return EXIT_FAILURE;
	}

	if (close(handle))
		perror("close");

	return EXIT_SUCCESS;
}
