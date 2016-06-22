/* This file is part of uinput-nes.
 *
 * uinput-nes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * uinput-nes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with uinput-nes.  If not, see <http://www.gnu.org/licenses/>.
 */
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
