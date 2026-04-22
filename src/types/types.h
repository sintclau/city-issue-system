#ifndef TYPES_H
#define TYPES_H

typedef enum { ROLE_NONE, ROLE_INSPECTOR, ROLE_MANAGER } role_t;

typedef struct {
    double latitude;
    double longitude;
}gps_coordinates_t;

typedef enum { ISSUE_NONE, ISSUE_ROADS, ISSUE_LIGHTING, ISSUE_FLOODING } issue_category_t;

typedef struct {
    int id;
    char inspector[50];
    gps_coordinates_t location;
    issue_category_t category;
    int severity;
    char description[256];
}issue_report_t;

#endif // TYPES_H
