#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static void write_msg(const char *type, const char *content) {
    printf("%s:%s\n", type, content);
    fflush(stdout);
}

void sigint_handler(int signum) {
    (void)signum;
    write_msg("MSG", "Monitor shutting down");
    remove(".monitor_pid");
    exit(0);
}

void sigusr1_handler(int signum) {
    (void)signum;
    write_msg("MSG", "New report has been added!");
}

void set_signal_action(void) {
    struct sigaction act;

    memset(&act, 0, sizeof(act));

    act.sa_handler = &sigint_handler;
    sigaction(SIGINT, &act, NULL);

    act.sa_handler = &sigusr1_handler;
    sigaction(SIGUSR1, &act, NULL);
}

int main(void) {
    FILE *pid_file = fopen(".monitor_pid", "r");
    if (pid_file != NULL) {
        pid_t existing_pid = 0;
        if (fscanf(pid_file, "%d", &existing_pid) == 1 && kill(existing_pid, 0) == 0) {
            fclose(pid_file);
            printf("ERROR:%d\n", existing_pid);
            fflush(stdout);
            return 1;
        }
        fclose(pid_file);
    }

    pid_file = fopen(".monitor_pid", "w");
    if (pid_file == NULL) {
        fprintf(stderr, "Error opening .monitor_pid file\n");
        return 1;
    }
    fprintf(pid_file, "%d\n", getpid());
    fclose(pid_file);

    set_signal_action();

    write_msg("MSG", "Monitor started successfully");

    while (1) {
        sigsuspend(NULL);
    }
    return 0;
}
