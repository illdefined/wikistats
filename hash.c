/*                                   
 * Copyright © MMVII
 *      Mikael Voss <ffefd6 at haemoglobin dot org>
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


#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "hash.h"
#include "log.h"

struct Entry *table;
unsigned long int entries = 32768;

inline uint32_t extract(const uint8_t *data) {
        return (((uint32_t) data[1]) << 8) + ((uint32_t) data[0]);
}

inline uint32_t hash(const uint8_t *data, unsigned long int length) {
        register uint32_t hash = length;
        register uint32_t temp;
        register unsigned long int remaining;

        remaining = length & 3;
        length >>= 2;

        while(length--) {
                hash += extract(data);
                temp = (extract(data+2) << 11) ^ hash;
                hash = (hash << 16) ^ temp;
                data += 2 * sizeof(uint16_t);
                hash += hash >> 11;
        }
        
        switch(remaining) {
                case 3: 
                        hash += extract(data);
                        hash ^= hash << 16;
                        hash ^= data[sizeof(uint16_t)] << 18;
                        hash += hash >> 11;
                        break;
                
                case 2: 
                        hash += extract(data);
                        hash ^= hash << 11;
                        hash += hash >> 17;
                        break;

                case 1: 
                        hash += *data;
                        hash ^= hash << 10;
                        hash += hash >> 1;
                        break;
        }
        
        hash ^= hash << 3;
        hash += hash >> 5;
        hash ^= hash << 4;
        hash += hash >> 17;
        hash ^= hash << 25;
        hash += hash >> 6;

        return hash % entries;
}

inline signed char flip(unsigned long int integer) {
	// Equivalent to (-1) to the power of integer
	return (integer % 2) ? -1 : 1;
}

inline struct Entry *lookup(const char *key) {
	register unsigned long hashValue;
	register struct Entry *entry;

	hashValue = hash((uint8_t *) key, strlen(key));
	entry = table + hashValue;

	// Test if bucket is empty
	if(*(entry->key) == '\0') {
		strncpy((char *) entry->key, key, sizeof(table->key));
		return entry;
	}

	// Check for collision
	if(!strncmp(key, (char *) entry->key, sizeof(table->key)))
		return entry;

	// Quadratic probing
	for(register unsigned int iterator = 0; iterator < 64; iterator++) {
		entry = table + ((hashValue + flip(iterator) * (iterator/2) * (iterator/2)) % entries);

		// Test if bucket is empty
		if(*(entry->key) == '\0') {
			strncpy((char *) entry->key, key, sizeof(table->key));
			return entry;
		}

		// Check for collision
		if(!strncmp(key, (char *) entry->key, sizeof(table->key)))
			return entry;
	}

	return 0;
}

bool commit(const char *key, unsigned long long int value) {
	register struct Entry *entry = lookup(key);

	if(entry == 0)
		return false;

	entry->value = value;
	return true;
}

bool increase(const char *key) {
	register struct Entry *entry = lookup(key);

	if(entry == 0)
		return false;

	entry->value++;
	return true;
}

