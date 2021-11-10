#include "ash_path.h"

int pid;
int ash(int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist, int cmdsize);
void where(char *command, struct pathelement *pathlist, int cmdsize);
void list(char *dir);
void printenv(char **envp);
void freepath(struct pathelement *pathlist);
void freeargs(char **args, int argsct);

#define PROMPTMAX 32
#define MAXARGS 64
#define PATHLEN 128
#define BUFFSIZE 64
