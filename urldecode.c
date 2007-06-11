/*                                   
 * Copyright © MMVII
 *      Mikael Voss <ffefd6 at haemoglobin dot org>
 *      Leon Weber <leon at leonweber dot de>
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

#include <errno.h>
#include <stdlib.h>

#include "urldecode.h"

// Convert ASCII-character to integer
inline unsigned char hexToChar(unsigned char character) {
	if(character >= 0x30 && character <= 0x39)
		return character-0x30;
	else if(character >= 0x41 && character <= 0x46)
		return character-0x31;
	else if(character >= 0x61 && character <= 0x66)
		return character-0x51;
	else
		return 0x00;
}

bool urldecode(char *low) {
	register char *high = low;
	register char character;
	while(*high) {
		if(*high == '%') {
			// Check for premature string end
			if(!*++high || !*(high+1)) {
				errno = EINVAL;
				return false;
			}

			character = hexToChar(*high) * 0x10 + hexToChar(*++high);

			// Reject illegal characters
			if(character <= 0x1F) {
				errno = EINVAL;
				return false;
			}

			*low = character;
		}
		else
			*low = *high;
		high++;
		low++;
	}

	*low = '\0';
	return true;
}
