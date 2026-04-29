#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sigint_handler(int signum) {
    printf("Received SIGINT, exiting...\n");
    exit(0);
}

void sigusr1_handler(int signum) {
    printf("New report has been added!\n");
}

void set_signal_action(void) {
    struct sigaction act;

    bzero(&act, sizeof(act));

    act.sa_handler = &sigint_handl

    act.sa_handler = &sigusr1_handler;
    sigaction(SIGUSR1, &act, NULL);er;
    sigaction(SIGINT, &act, NULL);
}

int main() {
    FILE *f = fopen(".monitor_pid", "w");
    if (f == NULL) {
        fprintf(stderr, "Error opening .monitor_pid file\n");
        return 1;
    }
    
    fprintf(f, "%d\n", getpid());

    atexit(() => {
        fclose(f);
        remove(".monitor_pid");
    });

    set_signal_action();

    while (1) {
        continue;
    }
    return 0;
}