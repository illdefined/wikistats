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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "log.h"
#include "hash.h"
#include "urldecode.h"

#define tokenize(str, delim) str = ptr; while(*ptr != (delim)) { if(*ptr == '\0') goto leave; ptr++; } *ptr = '\0'; ptr++;


const struct {
	const unsigned char major;
	const unsigned char minor;
	const unsigned char micro;
} version = { 0, 2, 2 };

long int pageSize;

const char const *versionString = "cwikistats version %hhu.%hhu.%hhu\n"
 "\n"
 "Copyright © MMVII\n"
 "  Mikael Voss <ffefd6 at haemoglobin dot org>\n"
 "  Leon Weber <leon at leonweber dot de>\n"
 "\n"
 "Provided that these terms and disclaimer and all copyright notices\n"
 "are retained or reproduced in an accompanying document, permission\n"
 "is granted to deal in this work without restriction, including un-\n"
 "limited rights to use, publicly perform, distribute, sell, modify,\n"
 "merge, give away, or sublicence.\n"
 "\n"
 "This work is provided “AS IS” and WITHOUT WARRANTY of any kind, to\n"
 "the utmost extent permitted by applicable law, neither express nor\n"
 "implied; without malicious intent or gross negligence. In no event\n"
 "may a licensor, author or contributor be held liable for indirect,\n"
 "direct, other damage, loss, or other issues arising in any way out\n"
 "of dealing in the work, even if advised of the possibility of such\n"
 "damage or existence of a defect, except proven that it results out\n"
 "of said person's immediate fault when using the work as intended.\n";


inline unsigned long int align(unsigned long int size, unsigned long int alignment) {
	return (size + alignment - 1ul) & ~(alignment - 1ul);
}

void synchronize() {
	debug(msync(table, align(entries * sizeof (struct Entry), (unsigned long int) pageSize), MS_SYNC));
}

int main(int argc, char *argv[]) {
	char *dbFile = "/var/db/wikistats/database";
	register int dbHandle;

	bool dump = false;
	unsigned long int minimum = 1;

	for (register unsigned int iterator = 1; iterator < argc; iterator++) {
		if (argv[iterator][0] == '-'
		 && argv[iterator][1] != '\0'
		 && argv[iterator][2] == '\0') {
			switch (argv[iterator][1]) {
				case 'd':
				 if (++iterator >= argc) {
				 	fputs("You must specify a path!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 dbFile = argv[iterator];
				 break;

				case 'h':
				 printf("usage: %s [options]\n"
				  "  -d path   Use path as database\n"
				  "  -h        Issue this help\n"
				  "  -l        Dump database\n"
				  "  -m num    Minimum\n"
				  "  -n num    Create num buckets\n"
				  "  -v        Show version\n",
				  argv[0]);
				 return EXIT_SUCCESS;

				case 'l':
				 dump = true;
				 break;

				case 'm':
				 if (++iterator >= argc) {
				 	fputs("You must specify a number!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 minimum = strtoul(argv[iterator], (char **) 0, 0);
				 break;

				case 'n':
				 if (++iterator >= argc) {
				 	fputs("You must specify a number!\n", stderr);
				 	return EXIT_FAILURE;
				 }
				 // No checking needed - if entries is invalid, it will fail anyway!
				 entries = strtoul(argv[iterator], (char **) 0, 0);
				 break;
				
				case 'v':
				 printf(versionString, version.major, version.minor, version.micro);
				 return EXIT_SUCCESS;

				default:
				 fprintf(stderr, "Invalid command line option -%c!\n", argv[iterator][1]);
				 return EXIT_FAILURE;
			}
		}
		else {
			fprintf(stderr, "Unknown command line argument %s!\n", argv[iterator]);
			return EXIT_FAILURE;
		}
	}

	// Alignsize of hash table mapping to page size
	catch((pageSize = sysconf(_SC_PAGESIZE)) == -1l);

	// Open system log
	openlog("cwikistats", LOG_PERROR, LOG_DAEMON);

	// Attempt to lock all memory to avoid paging of perfomance critical stuff
	warn(mlockall(MCL_CURRENT | MCL_FUTURE));

	// Open database file
	catch((dbHandle = open(dbFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1);

	// Truncate file to appropriate size (well, this fixes these mysterious bus errors...)
	catch(ftruncate(dbHandle, align(entries * sizeof (struct Entry), (unsigned long int) pageSize)));

	// Establish mapping
	catch((table = mmap(0, align(entries * sizeof (struct Entry), (unsigned long int) pageSize), PROT_READ | PROT_WRITE, MAP_SHARED, dbHandle, 0)) == MAP_FAILED);

	// Flush changes on exit
	atexit(synchronize);

	if (dump) {
		register struct Entry *current = table;
		while (current < table + entries) {
			if (current->value > minimum)
				printf("%020llu %s\n", current->value, current->key);
			current++;
		}
		return EXIT_SUCCESS;
	}

	static char readBuffer[4096];
	char *hostname, *sequence, *time, *reqtime, *ip, *squidStatus, *httpStatus, *size, *method, *url, *peer, *mime, *referrer, *forwarded, *useragent;

	while (fgets(readBuffer, sizeof readBuffer, stdin)) {
		register char *ptr = readBuffer;

		tokenize(hostname, ' ');
		tokenize(sequence, ' ');
		tokenize(time, ' ');
		tokenize(reqtime, ' ');
		tokenize(ip, ' ');
		tokenize(squidStatus, '/');
		tokenize(httpStatus, ' ');

		if(httpStatus[0] != '2'
		 || httpStatus[1] != '0'
		 || httpStatus[2] != '0'
		 || httpStatus[3] != '\0')
			continue;

		tokenize(size, ' ');
		tokenize(method, ' ');

		if(method[0] != 'G'
		 || method[1] != 'E'
		 || method[2] != 'T'
		 || method[3] != '\0')
			continue;

		tokenize(url, ' ');
		tokenize(peer, ' ');
		tokenize(mime, ' ');
		tokenize(referrer, ' ');
		tokenize(forwarded, ' ');
		useragent = ptr;

		// URL-decode URL
		debug(!urldecode((unsigned char *) url));

		// Commit to database
		catch(!increase(url));
leave:;
	}

	return EXIT_SUCCESS;
}
