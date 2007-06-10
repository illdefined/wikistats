#include "aux.h"

unsigned long int pageSize;

unsigned long int align(unsigned long int size, unsigned long int alignment) {
	return (size + alignment - 1ul) & ~(alignment - 1ul);
}
