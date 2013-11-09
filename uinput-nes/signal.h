#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>

void signal_handle(int sig);
void signal_install(void);

extern volatile int busy, interrupt;

#endif
