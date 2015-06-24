#include <system.h>
#include <device.h>
#include <isr.h>

#define MOUSE_CMD	0x64
#define MOUSE_DATA	0x60
#define SEND_CMD	0xD4	// Sent to port 0x64 to indicate that command will be sent to port 0x60
#define ENABLE_AUX_PS2	0xA8
#define GET_COMPAQ_STATUS	0x20
#define SET_COMPAQ_STATUS	0x60
#define USE_DEFAULTS	0xF6
#define ENABLE_MOUSE	0xF4


void mouse_wait(uint32_t i)
{
	if(!i)
		while(!(inb(MOUSE_CMD)&1));
	else
		while(inb(MOUSE_CMD)&2);
}

uint8_t mouse_read()
{
	mouse_wait(0);
	return inb(0x60);
}

void mouse_write(uint8_t chr)
{
	mouse_wait(1);
	outb(MOUSE_CMD, SEND_CMD);
	mouse_wait(1);
	outb(MOUSE_DATA, chr);
}

void mouse_init()
{
	mouse_wait(1);
	outb(MOUSE_CMD, ENABLE_AUX_PS2);
	
	mouse_wait(1);
	outb(MOUSE_CMD, GET_COMPAQ_STATUS);
	uint8_t status = mouse_read() | 2;	// Enable IRQ12
	
	mouse_wait(1);
	outb(MOUSE_CMD, SET_COMPAQ_STATUS);
	
	mouse_wait(0);
	outb(MOUSE_DATA, status);
	
	mouse_write(USE_DEFAULTS);
	mouse_read();	// Waits for ACK
	
	mouse_write(ENABLE_MOUSE);
	mouse_read();
}

typedef struct
{
	uint8_t left	: 1;
	uint8_t right	: 1;
	uint8_t mid		: 1;
	uint8_t _1		: 1;
	uint8_t x_sign	: 1;
	uint8_t y_sign	: 1;
	uint8_t x_over	: 1;
	uint8_t y_over	: 1;
} __attribute__((packed)) mouse_packet_t;

uint32_t posx = 0;
uint32_t posy = 0;

void mouse_handler(regs_t *regs)
{
	static uint8_t cycle = 0;
	static uint8_t mouse_data[3];
	mouse_data[cycle++] = inb(MOUSE_DATA);
	if(cycle == 3)
	{
		cycle = 0;	// We are done .. reset
		mouse_packet_t *packet = (mouse_packet_t*)&mouse_data[0];
		
		if(packet->x_over || packet->y_over) return;
		
		posx += mouse_data[1];
		posy += mouse_data[2];
		
		extern void draw_char(uint8_t, uint32_t, uint32_t, uint32_t);
		//draw_char('A', posx * 3, posy * 3, -1);
		debug("dx:%d dy:%d\n", 0xFF & mouse_data[1], 0xFF & mouse_data[2]);
	}
}
