#include "main.h"
#include "signal.h"

void signal_handle(int sig) {

	interrupt = sig;
        busy = 0;

}

void signal_install(void) {

        struct sigaction sig_act_old, sig_act_new;

        sig_act_new.sa_handler = signal_handle;
        sigemptyset(&sig_act_new.sa_mask);

        sig_act_new.sa_flags = 0;

        sigaction(SIGINT, NULL, &sig_act_old);

        if(sig_act_old.sa_handler != SIG_IGN)
                sigaction(SIGINT, &sig_act_new, NULL);

        sigaction(SIGHUP, NULL, &sig_act_old);

        if(sig_act_old.sa_handler != SIG_IGN)
                sigaction(SIGHUP, &sig_act_new, NULL);

        sigaction(SIGTERM, NULL, &sig_act_old);

        if(sig_act_old.sa_handler != SIG_IGN)
                sigaction (SIGTERM, &sig_act_new, NULL);

}

