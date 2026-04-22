#include "actions.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

int add_report(const char* district, issue_report_t *report) {
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

    // consume the leftover newline before fgets
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

    int file = open(path, O_WRONLY | O_CREAT | O_APPEND, PERM_REPORTS_DAT);
    if (file == -1) {
        perror(path);
        return -1;
    }

    struct stat st;
    if (fstat(file, &st) == -1) {
        perror(path);
        close(file);
        return -1;
    }
    report->id = (int)(st.st_size / sizeof(issue_report_t)) + 1;

    if (write(file, report, sizeof(issue_report_t)) == -1) {
        perror(path);
        close(file);
        return -1;
    }
    close(file);

    chmod(path, PERM_REPORTS_DAT);

    if (report->inspector.role == ROLE_MANAGER) log_action(district, report->inspector.role, report->inspector.username, "add");
    return 0;
}

int remove_report(const char* district, int report_id, user_t user) {
    // Implementation for removing a report
    if (user.role != ROLE_MANAGER) {
        fprintf(stderr, "Permission denied: only managers can remove reports.\n");
        return -1;
    }
    return 0;
}

int view_report(const char* district, int report_id) {
    // Implementation for viewing a report
    return 0;
}

int list_reports(const char* district) {
    // Implementation for listing reports
    return 0;
}

int update_threshold(const char* district, int new_threshold, user_t user) {
    // Implementation for updating threshold
    if (user.role != ROLE_MANAGER) {
        fprintf(stderr, "Permission denied: only managers can update threshold.\n");
        return -1;
    }
    return 0;
}

int filter_reports(const char* district, const char *condition) {
    // Implementation for filtering reports
    return 0;
}
