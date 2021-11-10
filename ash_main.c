#include "ash_new.h"
#include <signal.h>
#include <stdio.h>

void sig_handler(int signal);

/*
 * main runs the shell and handles signals
 *
 * Parameters: argc - count of arguments passed in, argv - array of arguments, envp - array of environment variables
 * Returns: exit value
 * Side effects: none
 */

int main( int argc, char **argv, char **envp) {
        struct sigaction siga;
        sigemptyset(&siga.sa_mask);
        siga.sa_handler = SIG_IGN;
        siga.sa_flags = SA_RESTART;
        if (sigaction(SIGINT, &siga, NULL) < 0) {
                perror("sigaction-SIGINT");
        }
        if (sigaction(SIGTSTP, &siga, NULL) < 0) {
                perror("sigaction-SIGTSTP");
        }
        if (sigaction(SIGTERM, &siga, NULL) < 0) {
                perror("sigaction-SIGTERM");
        }

        return ash(argc, argv, envp);
}

/*
 * sig_handler stops signals
 *
 * Parameters: signal - the signal to catch
 * Returns: none
 * Side effects: none
 */

void sig_handler(int signal) {
}
