#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ash_path.h"

#define MAXLINE 128

int main() {
        char buff[MAXLINE];
        char *tok;
        pid_t pid;
        int status;
        char *curr_dir;
        char *ptr;
        char **args;
        int background;

        background = 1;
        curr_dir = getcwd(NULL, 0);
        printf("[%s]> ", curr_dir);
        free(curr_dir);
        pid = getpid();
        while (fgets(buff, MAXLINE, stdin)!=NULL) {
                if (buff[strlen(buff)-1] == '\n')
                        buff[strlen(buff)-1] = '\0';
                tok = strtok(buff, " ");
                if (strcmp(tok, "pwd") == 0) {
                        ptr = getcwd(NULL, 0);
                        printf("cwd = [%s]\n", ptr);
                        free(ptr);
                } else if (strcmp(tok, "exit") == 0) {
                        exit(0);
                } else {
                        if ((pid = fork()) < 0) {
                                printf("fork ERR\n");
                                exit(1);
                        } else if (pid == 0) {
                                execlp(buff, buff, (char*)0);
                                printf("couldn't execute: %s\n", buff);
                                exit(127);
                        }
                        if (!background) {
                                if ((pid = waitpid(pid, &status, 0)) < 0)
                                        printf("waitpid ERR\n");
                        } else {

                        }
                }
                pid = waitpid(pid, &status, WNOHANG);
                curr_dir = getcwd(NULL, 0);
                printf("[%s]> ", curr_dir);
                free(curr_dir);
        }
        exit(0);
}
