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

#include <stdio.h>

#include "log.h"
#include "hash.h"
#include "urldecode.h"

#define tokenize(str, delim) \
	str = ptr; \
	loop_##str: \
	if(*ptr != (delim)) { \
		if(*ptr == '\0') \
			continue; \
		ptr++; \
		goto loop_##str; \
	} \
	*ptr = '\0'; \
	ptr++;

void parse(struct Table table, struct Table cache, char *buffer, size_t bufsize) {
	char *hostname, *sequence, *time, *reqtime, *ip, *squidStatus,
	     *httpStatus, *size, *method, *url, *peer, *mime, *referrer,
	     *forwarded, *useragent;

	while (fgets(buffer, bufsize, stdin)) {
		register char *ptr = buffer;

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
		debug(
			urldecode((unsigned char *) url)
		);

		// Commit to cache and flush it if necessary
		if (increase(cache, url)) {
			catch(
				inject(cache, table)
			);
		}
	}
}
