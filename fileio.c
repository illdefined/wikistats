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
#include <limits.h>
#include <time.h>

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int opendb(const char *format, int flags) {
	char path[NAME_MAX];

	time_t cur;
	struct tm *loc;

	time(&cur);
	loc = localtime(&cur);
	strftime(path, sizeof path, format, loc);

	return open(path, flags, PERMS);
}
