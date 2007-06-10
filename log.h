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

#define log(priority, expression)  syslog(priority, "%s[%u]::%s{%s}::{%s}", __FILE__, __LINE__, __func__, expression, strerror(errno))
#define catch(expression) if(expression) { log(LOG_ERR, #expression); exit(EXIT_FAILURE); }
#define warn(expression)  if(expression) log(LOG_WARNING, #expression)
#define debug(expression) if(expression) log(LOG_DEBUG, #expression)

#endif
