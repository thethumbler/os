#include <stdio.h>
#include <stdlib.h>

static __inline__ unsigned long long rdtsc(void)
{
  unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
}

int main() {
    unsigned long long cycles = rdtsc(); //1
    cycles = rdtsc() - cycles;           //2
    printf("Time is %d\n", (unsigned)cycles);
    return 0;
}
