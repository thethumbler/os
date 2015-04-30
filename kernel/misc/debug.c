#include <system.h>
#include <serial.h>
#include <debug.h>
#include <va_list.h>

static void debugs(uint8_t *str)
{
#ifdef _DEBUG_
	serial.write_str(str);
#endif
}

static void debugx(uint32_t val)
{
#ifdef _DEBUG_
	uint8_t *enc = "0123456789ABCDEF";
	uint8_t buf[9];
	buf[8] = '\0';
	uint8_t i = 8;
	while(i)
	{
		buf[--i] = enc[val&0xF];
		val >>= 4;
	}
	debug(buf);
#endif
}

static void debuglx(uint64_t val)
{
#ifdef _DEBUG_
	uint8_t *enc = "0123456789ABCDEF";
	uint8_t buf[17];
	buf[16] = '\0';
	uint8_t i = 16;
	while(i)
	{
		buf[--i] = enc[val&0xF];
		val >>= 4;
	}
	debug(buf);
#endif
}

static void debugud(uint32_t val)
{
#ifdef _DEBUG_
	uint8_t buf[11];
	buf[10] = '\0';
	if(!val) { buf[9] = '0'; debug(&buf[9]); }
	uint8_t i = 10;
	while(val)
	{
		buf[--i] = val%10 + '0';
		val = (val-val%10)/10;
	}
	debug(buf+i);
#endif
}

static void debugul(uint64_t val)
{
#ifdef _DEBUG_
	uint8_t buf[21];
	buf[20] = '\0';
	uint8_t i = 20;
	while(val)
	{
		buf[--i] = val%10 + '0';
		val = (val-val%10)/10;
	}
	debug(buf+i);
#endif
}

static void debugb(uint8_t val)
{
#ifdef _DEBUG_
	uint8_t buf[9];
	buf[8] = '\0';
	uint8_t i = 8;
	while(i)
	{
		buf[--i] = '0' + (val & 1);
		val >>= 1;
	}
	debug(buf);
#endif
}

void debug(uint8_t *fmt, ...)
{
#ifdef _DEBUG_
	va_list args;
	va_start(args, fmt);
	
	while(*fmt)
	switch(*fmt)
	{
		case '%':
			++fmt;
			switch(*fmt)
			{
				case 's':	/* char * */
					serial.write_str((uint8_t*)va_arg(args, uint8_t*));
					break;
				case 'd': /* decimal */
					debugud((uint32_t)va_arg(args, uint32_t));
					break;
				case 'l':	/* long */
					switch(*++fmt)
					{
						case 'x':	/* long hex */
							debuglx((uint64_t)va_arg(args, uint64_t));
							break;
						case 'd':
							debugul((uint64_t)va_arg(args, uint64_t));
							break;
						default:
							serial.write(*--fmt);
					}
					break;
				
				case 'b': /* binary */
					debugb((uint8_t)(uint32_t)va_arg(args, uint32_t));
					break;
				case 'x': /* Hexadecimal */
					debugx((uint32_t)va_arg(args, uint32_t));
					break;
				default:
					serial.write(*(--fmt));
			}
			++fmt;
			break;
		case '\n':
			serial.write_str("\n\r");
			++fmt;
			break;
		default:
			serial.write(*fmt);
			++fmt;
	}
	va_end(args);
#endif
}
