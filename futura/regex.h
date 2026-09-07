#ifndef FUTURA_REGEX_H
#define FUTURA_REGEX_H
#include <stddef.h>
#include <sys/types.h>

typedef long regoff_t;
typedef struct {
    unsigned long magic;
    size_t re_nsub;
    int flags;
    char pattern[8192];
} regex_t;
typedef struct { regoff_t rm_so; regoff_t rm_eo; } regmatch_t;

#define REG_BASIC 0
#define REG_EXTENDED 1
#define REG_ICASE 2
#define REG_NOSUB 4
#define REG_NEWLINE 8
#define REG_NOMATCH 1
#define REG_BADPAT 2
#define REG_ESPACE 12
#define REG_BADRPT 13

int regcomp(regex_t *, const char *, int);
int regexec(const regex_t *, const char *, size_t, regmatch_t[], int);
void regfree(regex_t *);
size_t regerror(int, const regex_t *, char *, size_t);
#endif
