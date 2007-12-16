#include "hash.h"

#define extract(data) ((((uint32_t) (data)[1]) << 8) + ((uint32_t) (data)[0]))

inline uint32_t hash(const uint8_t *data, unsigned long int length) {
	register uint32_t hash = length;
	register uint32_t temp;
	register unsigned long int remaining;

	remaining = length & 3;
	length >>= 2;

	while (length--) {
		hash += extract(data);
		temp = (extract(data+2) << 11) ^ hash;
		hash = (hash << 16) ^ temp;
		data += 2 * sizeof (uint16_t);
		hash += hash >> 11;
	}

	switch (remaining) {
		case 3: 
		 hash += extract(data);
		 hash ^= hash << 16;
		 hash ^= data[sizeof (uint16_t)] << 18;
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

	return hash;
}
