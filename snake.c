#include "snake.h"

void *memsetf(void *dest, register int val, register uint32 len)
{
  register unsigned char *ptr = (unsigned char*)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}