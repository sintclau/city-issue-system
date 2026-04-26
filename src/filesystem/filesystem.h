#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/stat.h>
#include "types/types.h"

#define PERM_DISTRICT_DIR  0750
#define PERM_REPORTS_DAT   0664
#define PERM_DISTRICT_CFG  0640
#define PERM_LOGGED        0644

#define REPORTS_FILE      "reports.dat"
#define CONFIG_FILE       "district.cfg"
#define LOG_FILE          "logged_district"

#define DEFAULT_THRESHOLD 2

void district_path(const char *district_id, const char *filename, char *out, size_t size);
int  district_init(const char *district_id);
int  check_permission(const char *path, role_t role, int need_write);
void log_action(const char *district_id, role_t role, const char *username, const char *action);
void mode_to_string(mode_t mode, char out[10]);
void symlink_create(const char *district_id);

#endif
