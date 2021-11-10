/*
 * ash_path.h
 * Alex You
 *
 * Borrowed ideas from get_path.h by Ben Miller
*/
#include <stdio.h>

struct pathelement *get_path();

struct pathelement {
        char *element;
        struct pathelement *next;
};
