#include <stdio.h>
#include "IO.h"

void IO_Write(ioAddress addr, ioData data)
{
    ioData *p = NULL;
    *(p + addr) = data;
}

ioData IO_Read(ioAddress addr)
{
    ioData *p = NULL;
    return *(p + addr);
}
