/*				   
 * Copyright © MMVII
 *      Mikael Voss <ffefd6 at haemoglobin dot org>
 *	Leon Weber <leon at vserver152.mastersystems dot com>
 * 
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un-
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *	      
 * This work is provided "AS IS" and WITHOUT WARRANTY of any kind, to
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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "aux.h"
#include "log.h"
#include "hash.h"

const char *dbFile = "/var/db/wikistats/database";

int main(int argc, char *argv[]) {
	static char readBuffer[4096];
	register int dbStream;
	register int errorNumber;
	register char *hostname, *sequence, *time, *reqtime, *ip, *squidStatus, *httpStatus, *size, *method, *url, *peer, *mime, *referer, *forwarded, *useragent;

	// Align size of hash table mapping to page size
	catch((pageSize = sysconf(_SC_PAGESIZE)) == (unsigned long int) -1, CRITICAL);

	// Open system log
	openlog("cwikistats", LOG_PID | LOG_PERROR, LOG_DAEMON);

	// Attempt to lock all memory to avoid paging of perfomance critical stuff
	catch(mlockall(MCL_CURRENT | MCL_FUTURE), ERROR);

	// Open database file
	catch((dbStream = open(dbFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH)) == -1, CRITICAL);

	// MMap database
	catch((table = mmap(0, align(entries * sizeof(struct Entry), pageSize), PROT_READ | PROT_WRITE, MAP_SHARED, dbStream, 0)) == MAP_FAILED, CRITICAL);

	while(fgets(readBuffer, sizeof readBuffer, stdin)) {
		// Tokenize input line
		if(!(hostname = strtok(readBuffer, " ")))
			continue;

		if(!(sequence = strtok(NULL, " ")))
			continue;

		if(!(time = strtok(NULL, " ")))
			continue;

		if(!(reqtime = strtok(NULL, " ")))
			continue;

		if(!(ip = strtok(NULL, " ")))
			continue;

		if(!(squidStatus = strtok(NULL, "/")))
			continue;

		if(!(httpStatus = strtok(NULL, " "))
			|| strncmp(httpStatus, "200", 4))
			continue;

		if(!(size = strtok(NULL, " ")))
			continue;

		// Drop non-GET requests
		if(!(method = strtok(NULL, " "))
			|| strncmp(method, "GET", 4))
			continue;

		if(!(url = strtok(NULL, " ")))
			continue;

		if(!(peer = strtok(NULL, " ")))
			continue;

		if(!(mime = strtok(NULL, " ")))
			continue;

		if(!(referer = strtok(NULL, " ")))
			continue;

		if(!(forwarded = strtok(NULL, " ")))
			continue;

		if(!(useragent = strtok(NULL, " ")))
			continue;

		// URL-decode URL
		catch(!urldecode(url), DEBUG);

		// Commit to database
		catch(!increase(url), ERROR);
	}
}
