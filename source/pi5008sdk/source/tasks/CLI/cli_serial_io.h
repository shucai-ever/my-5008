#ifdef SUPPORT_DEBUG_CLI
#ifndef __SERIAL_IO_H__
#define __SERIAL_IO_H__

unsigned char serial_getc(unsigned char *a_data);
void serial_putc(unsigned char *data, unsigned char a_len);


#endif /* __SERIAL_IO_H__ */
#endif /* SUPPORT_DEBUG_CLI */