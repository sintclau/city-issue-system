#include "filesystem.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

void district_path(const char *district_id, const char *filename, char *out, size_t size) {
    snprintf(out, size, "%s/%s", district_id, filename);
}

static int create_file_if_missing(const char *path, mode_t perm) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, perm);
    if (fd == -1) {
        if (errno == EEXIST) return 0;
        perror(path);
        return -1;
    }
    close(fd);
    if (chmod(path, perm) == -1) {
        perror(path);
        return -1;
    }
    return 0;
}

int district_init(const char *district_id) {
    if (mkdir(district_id, PERM_DISTRICT_DIR) == -1 && errno != EEXIST) {
        perror(district_id);
        return -1;
    }
    if (chmod(district_id, PERM_DISTRICT_DIR) == -1) {
        perror(district_id);
        return -1;
    }

    char path[512];

    district_path(district_id, REPORTS_FILE, path, sizeof(path));
    if (create_file_if_missing(path, PERM_REPORTS_DAT) == -1) return -1;

    district_path(district_id, CONFIG_FILE, path, sizeof(path));
    if (create_file_if_missing(path, PERM_DISTRICT_CFG) == -1) return -1;

    struct stat st;
    if (stat(path, &st) == 0 && st.st_size == 0) {
        int fd = open(path, O_WRONLY);
        if (fd != -1) {
            char buf[32];
            int n = snprintf(buf, sizeof(buf), "threshold=%d\n", DEFAULT_THRESHOLD);
            write(fd, buf, (size_t)n);
            close(fd);
        }
    }

    district_path(district_id, LOG_FILE, path, sizeof(path));
    if (create_file_if_missing(path, PERM_LOGGED) == -1) return -1;

    symlink_create(district_id);
    return 0;
}

int check_permission(const char *path, role_t role, int need_write) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror(path);
        return -1;
    }

    mode_t m = st.st_mode;
    int allowed;

    if (role == ROLE_MANAGER) {
        allowed = need_write ? (m & S_IWUSR) : (m & S_IRUSR);
    } else {
        allowed = need_write ? (m & S_IWGRP) : (m & S_IRGRP);
    }

    if (!allowed) {
        char sym[10];
        mode_to_string(m, sym);
        fprintf(stderr, "Permission denied: %s (permissions: %s, role: %s, access: %s)\n",
                path, sym,
                role == ROLE_MANAGER ? "manager" : "inspector",
                need_write ? "write" : "read");
        return -1;
    }
    return 0;
}

void log_action(const char *district_id, role_t role, const char *username, const char *action) {
    char path[512];
    district_path(district_id, LOG_FILE, path, sizeof(path));

    if (role != ROLE_MANAGER) {
        fprintf(stderr, "Permission denied: inspector cannot write to %s\n", path);
        return;
    }

    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, PERM_LOGGED);
    if (fd == -1) {
        perror(path);
        return;
    }

    char buf[512];
    time_t now = time(NULL);
    int n = snprintf(buf, sizeof(buf), "%ld\t%s\t%s\t%s\n",
                     (long)now, username,
                     role == ROLE_MANAGER ? "manager" : "inspector",
                     action);
    write(fd, buf, (size_t)n);
    close(fd);
}

void mode_to_string(mode_t mode, char out[10]) {
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? 'x' : '-';
    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? 'x' : '-';
    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? 'x' : '-';
    out[9] = '\0';
}

void symlink_create(const char *district_id) {
    char link_name[256];
    snprintf(link_name, sizeof(link_name), "active_reports-%s", district_id);

    char target[512];
    snprintf(target, sizeof(target), "%s/%s", district_id, REPORTS_FILE);

    struct stat lst;
    if (lstat(link_name, &lst) == 0) {
        if (S_ISLNK(lst.st_mode)) {
            struct stat st;
            if (stat(link_name, &st) == -1)
                fprintf(stderr, "Warning: dangling symlink %s removed\n", link_name);
            unlink(link_name);
        }
    }

    if (symlink(target, link_name) == -1)
        perror(link_name);
}
