#ifndef TYPES_H
#define TYPES_H

#include <time.h>

typedef enum { ROLE_NONE, ROLE_INSPECTOR, ROLE_MANAGER } role_t;

typedef struct {
    char username[50];
    role_t role;
}user_t;

typedef struct {
    double latitude;
    double longitude;
} gps_coordinates_t;

typedef struct {
    int              id;
    user_t           inspector;
    gps_coordinates_t location;
    char             category[32];  // "road", "lighting", "flooding", etc.
    int              severity;      // 1=minor, 2=moderate, 3=critical
    time_t           timestamp;
    char             description[256];
} issue_report_t;

#endif // TYPES_H
