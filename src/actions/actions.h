#ifndef ACTIONS_H
#define ACTIONS_H

#include "../types/types.h"
#include "../filesystem/filesystem.h"

int add_report(const char* district, issue_report_t *report);

int remove_report(const char* district, int report_id, user_t user);

int view_report(const char* district, int report_id);

int list_reports(const char* district);

int update_threshold(const char* district, int new_threshold, user_t user);

int filter_reports(const char* district, const char *condition);

#endif // ACTIONS_H
