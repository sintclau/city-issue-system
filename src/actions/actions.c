#include "actions.h"
#include "monitor-communication.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAX_CONDS 16

static int parse_condition(const char *input, char *field, char *op, char *value) {
    const char *p1 = strchr(input, ':');
    if (!p1) return -1;
    const char *p2 = strchr(p1 + 1, ':');
    if (!p2) return -1;

    strncpy(field, input, (size_t)(p1 - input));
    field[p1 - input] = '\0';

    strncpy(op, p1 + 1, (size_t)(p2 - p1 - 1));
    op[p2 - p1 - 1] = '\0';

    strncpy(value, p2 + 1, 255);
    value[255] = '\0';
    return 0;
}

static int match_condition(issue_report_t *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int v = atoi(value);
        int s = r->severity;
        if (strcmp(op, "==") == 0) return s == v;
        if (strcmp(op, "!=") == 0) return s != v;
        if (strcmp(op, "<")  == 0) return s <  v;
        if (strcmp(op, "<=") == 0) return s <= v;
        if (strcmp(op, ">")  == 0) return s >  v;
        if (strcmp(op, ">=") == 0) return s >= v;
    } else if (strcmp(field, "category") == 0) {
        int cmp = strcmp(r->category, value);
        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
        if (strcmp(op, "<")  == 0) return cmp <  0;
        if (strcmp(op, "<=") == 0) return cmp <= 0;
        if (strcmp(op, ">")  == 0) return cmp >  0;
        if (strcmp(op, ">=") == 0) return cmp >= 0;
    } else if (strcmp(field, "inspector") == 0) {
        int cmp = strcmp(r->inspector.username, value);
        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
        if (strcmp(op, "<")  == 0) return cmp <  0;
        if (strcmp(op, "<=") == 0) return cmp <= 0;
        if (strcmp(op, ">")  == 0) return cmp >  0;
        if (strcmp(op, ">=") == 0) return cmp >= 0;
    } else if (strcmp(field, "timestamp") == 0) {
        long v = atoi(value);
        long t = (long)r->timestamp;
        if (strcmp(op, "==") == 0) return t == v;
        if (strcmp(op, "!=") == 0) return t != v;
        if (strcmp(op, "<")  == 0) return t <  v;
        if (strcmp(op, "<=") == 0) return t <= v;
        if (strcmp(op, ">")  == 0) return t >  v;
        if (strcmp(op, ">=") == 0) return t >= v;
    }
    return 0;
}

int add_report(const char *district, issue_report_t *report) {
    if (district_init(district) == -1) return -1;

    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    if (check_permission(path, report->inspector.role, 1) == -1) return -1;

    printf("X: ");
    if (scanf("%lf", &report->location.latitude) != 1) {
        fprintf(stderr, "Error: invalid latitude.\n");
        return -1;
    }
    printf("Y: ");
    if (scanf("%lf", &report->location.longitude) != 1) {
        fprintf(stderr, "Error: invalid longitude.\n");
        return -1;
    }

    printf("Category (road/lighting/flooding/other): ");
    if (scanf("%31s", report->category) != 1) {
        fprintf(stderr, "Error: invalid category.\n");
        return -1;
    }

    printf("Severity level (1/2/3): ");
    if (scanf("%d", &report->severity) != 1 || report->severity < 1 || report->severity > 3) {
        fprintf(stderr, "Error: severity must be 1, 2, or 3.\n");
        return -1;
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    printf("Description: ");
    if (fgets(report->description, sizeof(report->description), stdin) == NULL) {
        fprintf(stderr, "Error: invalid description.\n");
        return -1;
    }
    size_t len = strlen(report->description);
    if (len > 0 && report->description[len - 1] == '\n')
        report->description[len - 1] = '\0';

    report->timestamp = time(NULL);

    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, PERM_REPORTS_DAT);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror(path);
        close(fd);
        return -1;
    }
    report->id = (int)(st.st_size / sizeof(issue_report_t)) + 1;

    if (write(fd, report, sizeof(issue_report_t)) == -1) {
        perror(path);
        close(fd);
        return -1;
    }
    close(fd);

    chmod(path, PERM_REPORTS_DAT);
    log_action(district, report->inspector.role, report->inspector.username, "add");
    announce_new_report();
    return 0;
}

int remove_report(const char *district, int report_id, user_t user) {
    if (user.role != ROLE_MANAGER) {
        fprintf(stderr, "Permission denied: only managers can remove reports.\n");
        return -1;
    }

    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    if (check_permission(path, user.role, 1) == -1) return -1;

    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror(path);
        close(fd);
        return -1;
    }

    int total = (int)(st.st_size / (off_t)sizeof(issue_report_t));
    int del_idx = -1;
    issue_report_t report;

    for (int i = 0; i < total; i++) {
        lseek(fd, (off_t)(i * (int)sizeof(issue_report_t)), SEEK_SET);
        if (read(fd, &report, sizeof(issue_report_t)) != (ssize_t)sizeof(issue_report_t))
            break;
        if (report.id == report_id) {
            del_idx = i;
            break;
        }
    }

    if (del_idx == -1) {
        fprintf(stderr, "Report with ID %d not found in district '%s'.\n", report_id, district);
        close(fd);
        return -1;
    }

    for (int i = del_idx + 1; i < total; i++) {
        lseek(fd, (off_t)(i * (int)sizeof(issue_report_t)), SEEK_SET);
        if (read(fd, &report, sizeof(issue_report_t)) != (ssize_t)sizeof(issue_report_t))
            break;
        lseek(fd, (off_t)((i - 1) * (int)sizeof(issue_report_t)), SEEK_SET);
        write(fd, &report, sizeof(issue_report_t));
    }

    ftruncate(fd, (off_t)((total - 1) * (int)sizeof(issue_report_t)));
    close(fd);

    log_action(district, user.role, user.username, "remove_report");
    printf("Report %d removed from district '%s'.\n", report_id, district);
    return 0;
}

int view_report(const char *district, int report_id) {
    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    issue_report_t report;
    int found = 0;
    while (read(fd, &report, sizeof(issue_report_t)) == (ssize_t)sizeof(issue_report_t)) {
        if (report.id == report_id) {
            found = 1;
            break;
        }
    }
    close(fd);

    if (!found) {
        fprintf(stderr, "Report with ID %d not found in district '%s'.\n", report_id, district);
        return -1;
    }

    printf("Report ID:   %d\n", report.id);
    printf("Inspector:   %s (%s)\n", report.inspector.username,
           report.inspector.role == ROLE_MANAGER ? "manager" : "inspector");
    printf("Location:    (%.6f, %.6f)\n", report.location.latitude, report.location.longitude);
    printf("Category:    %s\n", report.category);
    printf("Severity:    %d\n", report.severity);
    printf("Description: %s\n", report.description);
    printf("Timestamp:   %s", ctime(&report.timestamp));
    return 0;
}

int list_reports(const char *district) {
    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    struct stat st;
    if (stat(path, &st) == -1) {
        perror(path);
        return -1;
    }

    char sym[10];
    mode_to_string(st.st_mode, sym);

    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));

    printf("File:        %s\n", path);
    printf("Permissions: %s  |  Size: %lld bytes  |  Last modified: %s\n",
           sym, (long long)st.st_size, timebuf);
    printf("Records:     %lld\n", (long long)(st.st_size / (off_t)sizeof(issue_report_t)));
    printf("---\n");

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    issue_report_t report;
    int count = 0;
    while (read(fd, &report, sizeof(issue_report_t)) == (ssize_t)sizeof(issue_report_t)) {
        char ts[32];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M", localtime(&report.timestamp));
        printf("[%d] %-20s | %-12s | sev:%d | %s | %s\n",
               report.id, report.inspector.username,
               report.category, report.severity,
               ts, report.description);
        count++;
    }
    close(fd);

    if (count == 0)
        printf("No reports found.\n");
    return 0;
}

int update_threshold(const char *district, int new_threshold, user_t user) {
    if (user.role != ROLE_MANAGER) {
        fprintf(stderr, "Permission denied: only managers can update the threshold.\n");
        return -1;
    }

    char path[512];
    district_path(district, CONFIG_FILE, path, sizeof(path));

    struct stat st;
    if (stat(path, &st) == -1) {
        perror(path);
        return -1;
    }
    if ((st.st_mode & 0777) != PERM_DISTRICT_CFG) {
        char sym[10];
        mode_to_string(st.st_mode, sym);
        fprintf(stderr, "Error: %s has unexpected permissions %s (expected rw-r-----). Refusing to write.\n",
                path, sym);
        return -1;
    }

    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    char buf[64];
    int n = snprintf(buf, sizeof(buf), "threshold=%d\n", new_threshold);
    write(fd, buf, (size_t)n);
    close(fd);

    log_action(district, user.role, user.username, "update_threshold");
    printf("Threshold for district '%s' updated to %d.\n", district, new_threshold);
    return 0;
}

int filter_reports(const char *district, int nconds, const char **conditions) {
    if (nconds == 0) {
        fprintf(stderr, "Error: --filter requires at least one condition (field:op:value).\n");
        return -1;
    }

    char path[512];
    district_path(district, REPORTS_FILE, path, sizeof(path));

    char fields[MAX_CONDS][32];
    char ops[MAX_CONDS][4];
    char values[MAX_CONDS][256];

    for (int i = 0; i < nconds && i < MAX_CONDS; i++) {
        if (parse_condition(conditions[i], fields[i], ops[i], values[i]) == -1) {
            fprintf(stderr, "Error: invalid condition '%s'. Expected format: field:op:value\n",
                    conditions[i]);
            return -1;
        }
    }

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror(path);
        return -1;
    }

    issue_report_t report;
    int found = 0;

    while (read(fd, &report, sizeof(issue_report_t)) == (ssize_t)sizeof(issue_report_t)) {
        int match = 1;
        for (int i = 0; i < nconds && i < MAX_CONDS; i++) {
            if (!match_condition(&report, fields[i], ops[i], values[i])) {
                match = 0;
                break;
            }
        }
        if (match) {
            char ts[32];
            strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M", localtime(&report.timestamp));
            printf("[%d] %-20s | %-12s | sev:%d | %s | %s\n",
                   report.id, report.inspector.username,
                   report.category, report.severity,
                   ts, report.description);
            found++;
        }
    }
    close(fd);

    if (found == 0)
        printf("No reports match the given condition(s).\n");
    return 0;
}

int remove_district(const char *district_id, user_t user) {
    if (user.role != ROLE_MANAGER) {
        fprintf(stderr, "Permission denied: only managers can remove a district.\n");
        return -1;
    }

    // Safety: reject district_id that contains '/' or starts with '.'
    // to prevent accidentally passing dangerous paths to rm -rf.
    if (strchr(district_id, '/') != NULL || district_id[0] == '.') {
        fprintf(stderr, "Error: invalid district name '%s'.\n", district_id);
        return -1;
    }

    struct stat st;
    if (stat(district_id, &st) == -1 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: district '%s' does not exist.\n", district_id);
        return -1;
    }

    // Fork a child process to run: rm -rf <district_id>
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        char *args[] = {"rm", "-rf", (char *)district_id, NULL};
        execvp("rm", args);
        perror("execvp");
        exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return -1;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Error: rm -rf failed for district '%s'.\n", district_id);
        return -1;
    }

    char link_name[256];
    snprintf(link_name, sizeof(link_name), "active_reports-%s", district_id);
    if (unlink(link_name) == -1)
        perror(link_name);

    printf("District '%s' removed.\n", district_id);
    return 0;
}
