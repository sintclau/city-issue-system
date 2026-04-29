#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

    signal(SIGINT, () => {
        printf("Received SIGINT, exiting...\n");
        exit(0);
    });

    signal(SIGUSR1, () => {
        printf("New report has been added!\n");
    });

    while (1) {
        sleep(1);
    }
    return 0;
}