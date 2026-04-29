#include "monitor-communication.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void announce_new_report() {
    FILE *f = fopen(".monitor_pid", "r");
    if (f == NULL) {
        fprintf(stderr, "Error opening .monitor_pid file\n");
        return;
    }

    int monitor_pid;
    if (fscanf(f, "%d", &monitor_pid) != 1) {
        fprintf(stderr, "Error reading monitor PID from file\n");
        return;
    }
    
    kill(monitor_pid, SIGUSR1);

    fclose(f);
}