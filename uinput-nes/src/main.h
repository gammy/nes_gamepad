#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "signal.h"
#include "ftdi.h"
#include "uinput.h"

#define PADS_MAX	4

extern int verbosity;

#endif
