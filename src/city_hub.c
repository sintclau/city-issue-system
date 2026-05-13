#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define PIPE_READ  0
#define PIPE_WRITE 1
#define BUF_SIZE   512

static void hub_mon(void) {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("hub_mon: pipe");
        exit(1);
    }

    pid_t monitor_pid = fork();
    if (monitor_pid == -1) {
        perror("hub_mon: fork");
        exit(1);
    }

    if (monitor_pid == 0) {
        close(pipe_fd[PIPE_READ]);
        if (dup2(pipe_fd[PIPE_WRITE], STDOUT_FILENO) == -1) {
            perror("hub_mon: dup2");
            exit(1);
        }
        close(pipe_fd[PIPE_WRITE]);
        execl("./monitor_reports", "monitor_reports", NULL);
        perror("hub_mon: execl monitor_reports");
        exit(1);
    }

    close(pipe_fd[PIPE_WRITE]);

    FILE *pipe_in = fdopen(pipe_fd[PIPE_READ], "r");
    if (pipe_in == NULL) {
        perror("hub_mon: fdopen");
        exit(1);
    }

    char line[BUF_SIZE];
    while (fgets(line, sizeof(line), pipe_in) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        char *sep = strchr(line, ':');
        if (sep == NULL) {
            printf("[monitor] %s\n", line);
            fflush(stdout);
            continue;
        }

        *sep = '\0';
        const char *type    = line;
        const char *content = sep + 1;

        if (strcmp(type, "ERROR") == 0) {
            pid_t existing = (pid_t)atoi(content);
            printf("[hub_mon] Error: a monitor is already running with PID %d.\n", existing);
        } else if (strcmp(type, "MSG") == 0) {
            printf("[monitor] %s\n", content);
        } else {
            printf("[monitor] %s:%s\n", type, content);
        }
        fflush(stdout);
    }

    fclose(pipe_in);
    waitpid(monitor_pid, NULL, 0);
    printf("[hub_mon] Monitor process has ended.\n");
    fflush(stdout);
    exit(0);
}

static void start_monitor(void) {
    pid_t hub_pid = fork();
    if (hub_pid == -1) {
        perror("start_monitor: fork");
        return;
    }

    if (hub_pid == 0) {
        hub_mon();
    }

    printf("[city_hub] Monitor hub started (hub_mon PID: %d)\n", hub_pid);
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "start_monitor") == 0) {
        start_monitor();
    } else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
