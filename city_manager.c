// City infrastructure issue reporting and monitoring system
// Author: Spiescu Claudiu

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

char *program_name = NULL;
typedef enum { ROLE_NONE, ROLE_INSPECTOR, ROLE_MANAGER } role_t;

static void print_usage(const char *prog) {
    printf("Usage: %s --role <inspector|manager> <action> [args...]\n", prog);
    printf("Options:\n");
    printf("  -h, --help                  Show this help message\n");
    printf("  --role <inspector|manager>  Set the user role\n");
    printf("  --user <username>              Specify the username\n");
    printf("Actions:\n");
    printf("  --list <district_id>              List all reports for a district\n");
    printf("  --view <district_id> <report_id> View details of a specific report\n");
    printf("  --add <district_id>                Report an issue in a district\n");
    printf("  --remove_report <district_id> <report_id> Remove a report\n");
    printf("  --update_threshold <district_id> <value> Update the issue threshold for a district\n");
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
    OPT_LIST
};

int main(int argc, char *argv[]) {
    program_name = argv[0];

    static struct option long_options[] = {
        {"help",          no_argument,       NULL, 'h'},
        {"role",          required_argument, NULL, OPT_ROLE},
        {"user",          required_argument, NULL, OPT_USER},
        {"add",           required_argument, NULL, OPT_ADD},
        {"remove_report", required_argument, NULL, OPT_REMOVE_REPORT},
        {"view",          required_argument, NULL, OPT_VIEW},
        {"update_threshold", required_argument, NULL, OPT_UPDATE_THRESHOLD},
        {"list",          required_argument, NULL, OPT_LIST},
        {NULL, 0, NULL, 0}
    };

    role_t role = ROLE_NONE;
    const char *action = NULL;
    const char *username = NULL;
    int district = 0;
    int value = 0;

    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(program_name);
                return 0;
            case OPT_ROLE:
                role = parse_role(optarg);
                if (role == ROLE_NONE) {
                    fprintf(stderr, "Invalid role '%s'. Must be 'inspector' or 'manager'.\n", optarg);
                    return 1;
                }
                break;
            case OPT_USER:
                username = optarg;
                if (strlen(username) == 0) {
                    fprintf(stderr, "Username cannot be empty.\n");
                    return 1;
                }
                break;
            case OPT_ADD:
                action = "add";
                district = atoi(optarg);
                break;
            case OPT_REMOVE_REPORT:
                action = "remove_report";
                district = atoi(optarg);
                break;
            case OPT_VIEW:
                action = "view";
                district = atoi(optarg);
                break;
            case OPT_UPDATE_THRESHOLD:
                action = "update_threshold";
                district = atoi(optarg);
                if (optind < argc) {
                    value = atoi(argv[optind]);
                    optind++;
                } else {
                    fprintf(stderr, "Error: --update_threshold requires a value argument.\n");
                    print_usage(program_name);
                    return 1;
                }
                break;
            case OPT_LIST:
                action = "list";
                district = atoi(optarg);
                break;
            default:
                print_usage(program_name);
                return 1;
        }
    }

    if (role == ROLE_NONE) {
        fprintf(stderr, "Error: --role is required.\n");
        print_usage(program_name);
        return 1;
    }

    if (username == NULL) {
        fprintf(stderr, "Error: --user is required.\n");
        print_usage(program_name);
        return 1;
    }

    if (username && strlen(username) == 0) {
        fprintf(stderr, "Error: username cannot be empty.\n");
        print_usage(program_name);
        return 1;
    }

    if (!action) {
        fprintf(stderr, "Error: no action specified.\n");
        print_usage(program_name);
        return 1;
    }

    if (strcmp(action, "update_threshold") == 0) {
        if (value <= 0) {
            fprintf(stderr, "Error: threshold value must be a positive integer.\n");
            print_usage(program_name);
            return 1;
        }

        printf("Updating threshold for district '%d' to %d (user: %s, role: %s)\n",
               district, value, username, role == ROLE_MANAGER ? "manager" : "inspector");
        
        // check district folder exists
        // update threshold file in district folder
        // log the action
        // close files and exit
    }

    return 0;
}