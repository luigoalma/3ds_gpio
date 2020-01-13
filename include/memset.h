#pragma once
#include <stdint.h>
#include <stddef.h>

inline static void _memset32_aligned(void* dest, uint32_t c, size_t size) {
	uint32_t *_dest = (uint32_t*)dest;
	for (; size >= 4; size -= 4) {
		*_dest = c;
		_dest++;
	}
	uint8_t *_dest8 = (uint8_t*)_dest;
	for (; size > 0; size--) {
		*_dest8 = c;
		_dest8++;
	}
}
