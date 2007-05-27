/*                                   
 * Copyright © MMVII
 *      Mikael Voss <photon at haemoglobin dot org>
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

#ifndef LOG_H
#define LOG_H

#include <errno.h>
#include <syslog.h>

#define log(priority, offset, call, errnum) log_((priority), __FILE__, __LINE__, (offset), __func__, (call), (errnum))
#define catch(expression, priority) if(expression) log((priority), 0, #expression, errno) 

enum Priority {
	EMERGENCY = LOG_EMERG,
	ALERT = LOG_ALERT,
	CRITICAL = LOG_CRIT,
	ERROR = LOG_ERR,
	WARNING = LOG_WARNING,
	NOTICE = LOG_NOTICE,
	INFO = LOG_INFO,
	DEBUG = LOG_DEBUG
};

void log_(enum Priority priority, const char *file, unsigned int line, signed int offset, const char *function, const char *expression, int errnum);

#endif
