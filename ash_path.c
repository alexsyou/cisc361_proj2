/*
 * ash_path.c
 * Alex You
 *
 * Borrowed ideas from get_path.c by Ben Miller
*/
#include "get_path.h"

/*
 * get_path returns a pathelement of the current path
 *
 * Parameters: none
 * Returns: pathelement of the current PATH
 * Side effects: allocates memory that needs to be freed
 */

struct pathelement *get_path() {
        char *path, *p;
        struct pathelement *tmp, *pathlist = NULL;

        p = getenv("PATH");
        path = malloc((strlen(p)+1)*sizeof(char));
        strncpy(path, p, strlen(p));
        path[strlen(p)] = '\0';

        p = strtok(path, ":");
        do {
                if (!pathlist) {
                        tmp = calloc(1, sizeof(struct pathelement));
                        pathlist = tmp;
                } else {
                        tmp->next = calloc(1, sizeof(struct pathelement));
                        tmp = tmp->next;
                }
                tmp->element = p;
                tmp->next = NULL;
        } while ((p = strtok(NULL, ":")));
        return pathlist;
}
