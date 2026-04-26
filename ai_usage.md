# AI Usage Documentation – Phase 1

**Student:** Spiescu Claudiu  
**Tool used:** Claude (Anthropic)  
**Date:** April 2026

---

## What I used AI for

Two helper functions for the `filter` command: `parse_condition` and `match_condition`. Everything else was written by me.

---

## Prompt 1 — `parse_condition`

**Prompt:**

> I have a C program that implements a city report filter. Conditions are passed as strings in the format `field:op:value`, for example `severity:>=:2` or `category:==:road`. The operator can be `==`, `!=`, `<`, `<=`, `>`, `>=`. Write a C function:
> ```c
> int parse_condition(const char *input, char *field, char *op, char *value);
> ```
> that splits the input string into the three parts. Return 0 on success, -1 if the format is invalid.

**What was generated:**

```c
int parse_condition(const char *input, char *field, char *op, char *value) {
    const char *p1 = strchr(input, ':');
    if (!p1) return -1;
    const char *p2 = strchr(p1 + 1, ':');
    if (!p2) return -1;

    strncpy(field, input, (size_t)(p1 - input));
    field[p1 - input] = '\0';

    strncpy(op, p1 + 1, (size_t)(p2 - p1 - 1));
    op[p2 - p1 - 1] = '\0';

    strcpy(value, p2 + 1);
    return 0;
}
```

**What I changed:** The last line used `strcpy` with no length limit, which could overflow the buffer. I replaced it with `strncpy(value, p2 + 1, 255); value[255] = '\0';`. I also added `static` since the function is only used inside `actions.c`.

**What I learned:** I learned about `strchr`, which finds the first occurrence of a character in a string and returns a pointer to it. Calling it twice on the same string finds the two `:` separators. The pointer subtraction `p1 - input` gives the length of the field part, used as the copy length for `strncpy`.

---

## Prompt 2 — `match_condition`

**Prompt:**

> My report struct is:
> ```c
> typedef struct {
>     int               id;
>     user_t            inspector;
>     gps_coordinates_t location;
>     char              category[32];
>     int               severity;
>     time_t            timestamp;
>     char              description[256];
> } issue_report_t;
> ```
> Write a C function:
> ```c
> int match_condition(issue_report_t *r, const char *field, const char *op, const char *value);
> ```
> Supported fields: `severity`, `category`, `inspector`, `timestamp`. Supported operators: `==`, `!=`, `<`, `<=`, `>`, `>=`. Return 1 if satisfied, 0 otherwise.

**What was generated:**

```c
int match_condition(issue_report_t *r, const char *field, const char *op, const char *value) {
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
        int v = atoi(value);
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
```

**What I changed:** Nothing, the code worked correctly. I only added `static`.

**What I learned:** The function uses `strcmp` to check which field is being filtered, then compares the value accordingly. For numeric fields it converts the string with `atoi`, for string fields it uses `strcmp` directly. I reviewed it line by line and understood how each case works.

---
