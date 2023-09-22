#ifndef _P_UNIX_H
#define _P_UNIX_H

#include <stdint.h>

#define U8_FS  "%hhu"
#define U16_FS "%hu"
#define U32_FS "%u"

void rdelay(uint16_t d);
void outportb(uint16_t port, uint8_t byte);

#endif /* _P_UNIX_H */
