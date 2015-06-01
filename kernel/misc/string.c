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
	for(; *str1 && *str2 && *str1 == *str2; ++str1, ++str2);
	return *str1 - *str2;
}

void *memcpy(void *dst, void *src, uint64_t size)
{
	while(size--) *(uint8_t*)dst++ = *(uint8_t*)src++;
}

uint8_t *strcat(uint8_t *str1, uint8_t *str2)
{
	uint8_t *ret = kmalloc(strlen(str1) + strlen(str2) + 1);
	uint8_t *_ret = ret;
	uint32_t i = 0;
	uint32_t str1_len = strlen(str1);
	uint32_t str2_len = strlen(str2);
	while(i++ < str1_len)
		*ret++ = *str1++;
	i = 0;
	while(i++ < str2_len)
		*ret++ = *str2++;
	*ret = '\0';
	return _ret;
}

void *memset(void *addr, uint8_t val, uint32_t size)
{
	uint8_t *_addr = (uint8_t*)addr;
	while(size--) *_addr++ = val;
	return (void*)addr;
}

uint8_t *itoa(uint32_t val)
{
	uint8_t *buf = kmalloc(12);
	buf[11] = '\0';
	uint32_t i = 11;
	if(!val) return (buf[10] = '0'), &buf[10];
	while(val)
	{
		buf[--i] = '0' + val%10;
		val /= 10;
	}
	return (uint64_t)&buf[i];
}

uint8_t *strndup(uint8_t *src, uint32_t len)
{
	uint8_t *ret = kmalloc(len + 1);
	uint32_t i;
	for(i = 0; i < len; ++i)
		ret[i] = src[i];
	ret[i] = '\0';
	return ret;
}
