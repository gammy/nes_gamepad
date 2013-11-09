#ifndef FTDI_H
#define FTDI_H

#include "main.h"
#include <ftdi.h>

struct ftdi_context *
bub_init(unsigned int baud_rate,
	 unsigned char latency,
	 unsigned int tx_buf_size,
	 unsigned int rx_buf_size);

long
bub_fetch(struct ftdi_context *ftdic, uint8_t *buf, unsigned long s);

#endif
