#ifndef MICROTIME_H
#define MICROTIME_H

#include <stdint.h>

void MicroTime_Init(uint32_t start, uint32_t incr);
uint32_t MicroTime_Get(void);


#endif /*_MICROTIME_H_*/
