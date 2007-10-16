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

#include <fcntl.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "table.h"
#include "version.h"

int main(int argc, char *argv[]) {
	struct Entry entry;
	char *path = DEF_DB;
	int handle;
	unsigned long long int minimum = 1ull;
	const char *expression = (char *) 0;

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

				case 'e':
				 if (++iter >= argc) {
				 	fputs(REQ_EXP, stderr);
				 	return EXIT_FAILURE;
				 }
				 expression = argv[iter];
				 break;

				case 'h':
				 printf(USAGE OPT_D OPT_E OPT_H OPT_M OPT_V, argv[0]);
				 return EXIT_SUCCESS;

				case 'm':
				 if (++iter >= argc) {
				 	fputs(REQ_NUM, stderr);
				 	return EXIT_FAILURE;
				 }
				 minimum = strtoull(argv[iter], (char **) 0, 10);
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

	// Open database file
	handle = open(path, O_RDONLY);
	if (handle < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	if (expression) {
		regex_t preg;
		int ret;
		char errbuf[64];

		if ((ret = regcomp(&preg, expression, REG_NOSUB))) {
			regerror(ret, &preg, errbuf, sizeof errbuf);
			fputs(errbuf, stderr);
			return EXIT_FAILURE;
		}

		while (read(handle, &entry, sizeof (entry)) == sizeof (entry)) {
			if (entry.value >= minimum && !regexec(&preg, entry.key, 0, (regmatch_t *) 0, 0)) {
				printf("%020llu %s\n", entry.value, entry.key);
			}
		}
	}
	else {
		while (read(handle, &entry, sizeof (entry)) == sizeof (entry)) {
			if (entry.value >= minimum)
				printf("%020llu %s\n", entry.value, entry.key);
		}
	}

	return EXIT_SUCCESS;
}
