#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void sigint_handler(int signum) {
    printf("Received SIGINT, exiting...\n");
    remove(".monitor_pid");
    exit(0);
}

void sigusr1_handler(int signum) {
    printf("New report has been added!\n");
}

void set_signal_action(void) {
    struct sigaction act;

    memset(&act, 0, sizeof(act));

    act.sa_handler = &sigint_handler;
    sigaction(SIGINT, &act, NULL);

    act.sa_handler = &sigusr1_handler;
    sigaction(SIGUSR1, &act, NULL);
}

int main() {
    FILE *f = fopen(".monitor_pid", "w");
    if (f == NULL) {
        fprintf(stderr, "Error opening .monitor_pid file\n");
        return 1;
    }
    fprintf(f, "%d\n", getpid());
    fclose(f);

    set_signal_action();

    while (1) {
        sigsuspend(NULL);
    }
    return 0;
}