#ifndef IO_H
#define IO_H

#include <stdint.h>

typedef uint32_t ioAddress;
typedef uint16_t ioData;

void IO_Write(ioAddress addr, ioData data);
ioData IO_Read(ioAddress addr);

#endif // IO_H
