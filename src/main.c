#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "types/types.h"
#include "actions/actions.h"

char *program_name = NULL;

static void print_usage(const char *prog) {
    printf("Usage: %s --role <inspector|manager> <action> [args...]\n", prog);
    printf("Options:\n");
    printf("  -h, --help                   Show this help message\n");
    printf("  --role <inspector|manager>   Set the user role\n");
    printf("  --user <username>            Specify the username\n");
    printf("Actions:\n");
    printf("  --add <district_id>                              Report an issue\n");
    printf("  --list <district_id>                             List all reports\n");
    printf("  --view <district_id> <report_id>                 View a report\n");
    printf("  --remove_report <district_id> <report_id>        Remove a report\n");
    printf("  --update_threshold <district_id> <value>         Update severity threshold\n");
    printf("  --filter <district_id> <field:op:value> ...      Filter reports\n");
    printf("  --remove_district <district_id>                  Remove entire district (manager only)\n");
}

static role_t parse_role(const char *str) {
    if (strcmp(str, "inspector") == 0) return ROLE_INSPECTOR;
    if (strcmp(str, "manager") == 0)   return ROLE_MANAGER;
    return ROLE_NONE;
}

enum {
    OPT_ROLE,
    OPT_USER,
    OPT_ADD,
    OPT_REMOVE_REPORT,
    OPT_VIEW,
    OPT_UPDATE_THRESHOLD,
    OPT_LIST,
    OPT_FILTER,
    OPT_REMOVE_DISTRICT
};

int main(int argc, char *argv[]) {
    program_name = argv[0];

    static struct option long_options[] = {
        {"help",             no_argument,       NULL, 'h'},
        {"role",             required_argument, NULL, OPT_ROLE},
        {"user",             required_argument, NULL, OPT_USER},
        {"add",              required_argument, NULL, OPT_ADD},
        {"remove_report",    required_argument, NULL, OPT_REMOVE_REPORT},
        {"view",             required_argument, NULL, OPT_VIEW},
        {"update_threshold", required_argument, NULL, OPT_UPDATE_THRESHOLD},
        {"list",             required_argument, NULL, OPT_LIST},
        {"filter",           required_argument, NULL, OPT_FILTER},
        {"remove_district",  required_argument, NULL, OPT_REMOVE_DISTRICT},
        {NULL, 0, NULL, 0}
    };

    user_t user;
    memset(&user, 0, sizeof(user));
    user.role = ROLE_NONE;

    const char *action = NULL;
    const char *district = NULL;
    int value = 0;
    int nconds = 0;
    const char **conditions = NULL;

    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(program_name);
                return 0;
            case OPT_ROLE:
                user.role = parse_role(optarg);
                if (user.role == ROLE_NONE) {
                    fprintf(stderr, "Invalid role '%s'. Must be 'inspector' or 'manager'.\n", optarg);
                    return 1;
                }
                break;
            case OPT_USER:
                strncpy(user.username, optarg, sizeof(user.username) - 1);
                user.username[sizeof(user.username) - 1] = '\0';
                break;
            case OPT_ADD:
                action = "add";
                district = optarg;
                break;
            case OPT_REMOVE_REPORT:
                action = "remove_report";
                district = optarg;
                if (optind < argc) {
                    value = atoi(argv[optind++]);
                } else {
                    fprintf(stderr, "Error: --remove_report requires a report_id argument.\n");
                    print_usage(program_name);
                    return 1;
                }
                break;
            case OPT_VIEW:
                action = "view";
                district = optarg;
                if (optind < argc) {
                    value = atoi(argv[optind++]);
                } else {
                    fprintf(stderr, "Error: --view requires a report_id argument.\n");
                    print_usage(program_name);
                    return 1;
                }
                break;
            case OPT_UPDATE_THRESHOLD:
                action = "update_threshold";
                district = optarg;
                if (optind < argc) {
                    value = atoi(argv[optind++]);
                } else {
                    fprintf(stderr, "Error: --update_threshold requires a value argument.\n");
                    print_usage(program_name);
                    return 1;
                }
                break;
            case OPT_LIST:
                action = "list";
                district = optarg;
                break;
            case OPT_FILTER:
                action = "filter";
                district = optarg;
                conditions = (const char **)(argv + optind);
                nconds = argc - optind;
                optind = argc;
                break;
            case OPT_REMOVE_DISTRICT:
                action = "remove_district";
                district = optarg;
                break;
            default:
                print_usage(program_name);
                return 1;
        }
    }

    if (user.role == ROLE_NONE) {
        fprintf(stderr, "Error: --role is required.\n");
        print_usage(program_name);
        return 1;
    }

    if (strlen(user.username) == 0) {
        fprintf(stderr, "Error: --user is required.\n");
        print_usage(program_name);
        return 1;
    }

    if (!action) {
        fprintf(stderr, "Error: no action specified.\n");
        print_usage(program_name);
        return 1;
    }

    if (strcmp(action, "add") == 0) {
        issue_report_t report;
        memset(&report, 0, sizeof(report));
        report.inspector = user;
        if (add_report(district, &report) != 0) {
            fprintf(stderr, "Error: failed to add report.\n");
            return 1;
        }
    } else if (strcmp(action, "remove_report") == 0) {
        if (remove_report(district, value, user) != 0) {
            fprintf(stderr, "Error: failed to remove report %d.\n", value);
            return 1;
        }
    } else if (strcmp(action, "view") == 0) {
        if (view_report(district, value) != 0) {
            fprintf(stderr, "Error: report %d not found.\n", value);
            return 1;
        }
    } else if (strcmp(action, "list") == 0) {
        if (list_reports(district) != 0) {
            fprintf(stderr, "Error: failed to list reports for '%s'.\n", district);
            return 1;
        }
    } else if (strcmp(action, "update_threshold") == 0) {
        if (value <= 0) {
            fprintf(stderr, "Error: threshold value must be a positive integer.\n");
            return 1;
        }
        if (update_threshold(district, value, user) != 0) {
            fprintf(stderr, "Error: failed to update threshold for '%s'.\n", district);
            return 1;
        }
    } else if (strcmp(action, "filter") == 0) {
        if (filter_reports(district, nconds, conditions) != 0) {
            fprintf(stderr, "Error: filter failed for district '%s'.\n", district);
            return 1;
        }
    } else if (strcmp(action, "remove_district") == 0) {
        if (remove_district(district, user) != 0) {
            fprintf(stderr, "Error: failed to remove district '%s'.\n", district);
            return 1;
        }
    } else {
        fprintf(stderr, "Error: unknown action '%s'.\n", action);
        print_usage(program_name);
        return 1;
    }

    return 0;
}
