#include <system.h>
#include <string.h>

uint32_t strlen(uint8_t *str)
{
	uint32_t len = 0;
	while(*str++) ++len;
	return len;
}

uint8_t *strdup(uint8_t *str)
{
	uint8_t *tmp = kmalloc(strlen(str) + 1);
	uint8_t *ret = tmp;
	while( *str && (*tmp++ = *str++));
	return *tmp = '\0', ret;
}

uint32_t strcmp(uint8_t *str1, uint8_t *str2)
{
	while( *str1 && *str2 && *str1++ == *str2++ );
	return *str1 - *str2;
}

void *memcpy(void *dst, void *src, uint32_t size)
{
	while(size--) *(uint8_t*)dst = *(uint8_t*)src;
}
