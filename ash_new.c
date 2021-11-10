/*
 * The ashell (ash) by Alex You
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <glob.h>
#include "ash_new.h"


/*
 * ash is the function that runs the shell
 *
 * Parameters: argc - number of arguments, argv - argument array, envp - envpointers.
 * Returns: int exit value.
 * Side Effects: may take in values from stdio, may change directories and environment variables, may allocate unfree'd memory if 
 * calling execve.
 */

int ash( int argc, char **argv, char **envp) {
	char *prompt = calloc(PROMPTMAX, sizeof(char));
	char *commandline = calloc(MAX_CANON, sizeof(char));
	char *command, *arg, *commandpath, *p, *pwd, *owd, *pwd_copy, *tmp;
    char command_copy[MAX_CANON];
	char **args = calloc(MAXARGS, sizeof(char*));
    char **new_args = calloc(MAXARGS+1, sizeof(char*));
	int uid, i, k_pid, status, argsct, newargsct, go = 1;
	struct passwd *password_entry;
	char *homedir;
	struct pathelement *pathlist;
	pid_t pid, cpid, w;
    glob_t globbuf = {0};

	uid = getuid();
	password_entry = getpwuid(uid);
	if ((homedir = getenv("HOME")) == NULL)
	   homedir = password_entry->pw_dir;

	if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL ) {
		perror("getcwd");
		exit(2);
	}
	owd = calloc(strlen(pwd) + 1, sizeof(char));
	memcpy(owd, pwd, strlen(pwd));
	prompt[0] = ' ';
	prompt[1] = '\0';

	pathlist = get_path();
	
	pid = getpid();
	while ( go ) {
		printf("%s[%s]>", prompt, pwd);
		if (fgets(commandline, MAX_CANON, stdin) != NULL) { 
			if (commandline[strlen(commandline)-1] == '\n')
			    commandline[strlen(commandline)-1] = '\0';
			if ((command = strtok(commandline, " ")) != NULL) {
				argsct = 0;

                //Non-built in, so args[0] is the command name
                if (command[0] == '/') {
                    args[0] = calloc(strlen(command)+1,sizeof(char));
                    strcpy(args[0], command);
                    argsct++;
                } else if (strncmp(command, "./", 2) == 0) {   
                    memmove(command, command+1, strlen(command));
                    snprintf(command_copy, MAX_CANON, "%s%s", pwd, command);
                    command = command_copy;
                    args[0] = calloc(strlen(command)+1,sizeof(char));
                    strcpy(args[0], command);
                    argsct++;
                } else if (strncmp(command, "../", 3) == 0) {
                    for (int i=strlen(pwd)-1;i>=0;i--) {
                        if (pwd[i] == '/') {
                            pwd_copy = calloc(i+1,sizeof(char));
                            strncpy(pwd_copy, pwd, i);
                            pwd_copy[i] = '\0';
                            break;
                        } else if (i == 0) {
                            printf("error: pwd has no /\n");
                        }    
                    }
                    memmove(command, command+2, strlen(command));
                    snprintf(command_copy, MAX_CANON, "%s%s", pwd_copy, command);
                    command = command_copy;
                    args[0] = calloc(strlen(command)+1,sizeof(char));
                    strcpy(args[0], command);
                    argsct++;
                    free(pwd_copy);
                }

                //glob necessary args
				while ((arg = strtok(NULL, " ")) != NULL) {
                    if (command[0] == '/' && (strchr(arg, '*')!=NULL || strchr(arg, '?')!=NULL)) {
                        glob(arg, GLOB_DOOFFS, NULL, &globbuf);
                        for (size_t i = 0; i!=globbuf.gl_pathc; ++i) {
                            args[argsct] = calloc(strlen(globbuf.gl_pathv[i])+1, sizeof(char));
                            strcpy(args[argsct], globbuf.gl_pathv[i]);
                            argsct++;
                        }
                        globfree(&globbuf);
                    } else {
					    args[argsct] = calloc(strlen(arg)+1, sizeof(char));
					    strcpy(args[argsct], arg);
					    argsct++;
                    }
				}

                //builtin exit
				if (strcmp(command, "exit") == 0) {
					printf("exiting shell\n");
					go = 0;
					break;
				} 
                //builtin pid
                else if (strcmp(command, "pid") == 0) {
					printf("executing builtin pid:\npid of current process is [%d]\n",pid);
				
                } 
                //builtin which
                else if (strcmp(command, "which") == 0) {
					printf("executing builtin which:\n");
					if(argsct>0) {
                        for (int i=0;i<argsct;i++) {
						    p = which(args[i], pathlist, strlen(args[i]));
                            free(p);
                        }
					} else {
						printf("error: which missing second argument\n");
					}
				} 
                //builtin where
                else if (strcmp(command, "where") == 0) {
					printf("executing builtin where:\n");
					if (argsct>0) {
                        for (int i=0;i<argsct;i++) {
						    where(args[i], pathlist, strlen(args[i]));
                        }
					} else {
						printf("error: where missing second argument\n");
					}
				}
                //builtin prompt
                else if (strcmp(command, "prompt") == 0) {
					printf("executing builtin prompt:\n");
					if (argsct>0) {
						strcpy(prompt, args[0]);
					} else {
						printf("input prompt prefix: ");
						if (fgets(prompt, PROMPTMAX, stdin) != NULL) {
							if (prompt[strlen(prompt)-1] == '\n')
							prompt[strlen(prompt)-1] = '\0';
						}
					}
				} 
                //builtin cd
                else if (strcmp(command, "cd") == 0) {
					printf("executing builtin cd:\n");
					if (argsct == 0) {
						if (chdir(homedir) == 0) {
							strcpy(owd, pwd);
							strcpy(pwd, homedir);
						} else {
							perror("chdir");
						}
					} else if (strcmp(args[0], "-") == 0) {
						if (chdir(owd) == 0) {
                            tmp = (char *) malloc((strlen(pwd) + 1) * sizeof(char));
							strcpy(tmp, pwd);
							strcpy(pwd, owd);
							strcpy(owd, tmp);
                            free(tmp);
						} else {
							perror("chdir");
						}
					} else if (argsct == 1) {
						if (chdir(args[0]) == 0) {
							strcpy(owd, pwd);
							strcpy(pwd, args[0]);
						} else {
							perror("chdir");
						}
					} else {
                        printf("error: cd has too many arguments\n");
                    }
				} 
                //builtin pwd
                else if (strcmp(command, "pwd") == 0) {
					printf("executing builtin pwd:\n");
					printf("%s\n", pwd);
				} 
                //builtin list
                else if (strcmp(command, "list") == 0) {
                    printf("executing builtin list:\n");
                    if (argsct == 0) {
                        list(pwd);
                    } else {
                        for (int i=0;i<argsct;i++) {
                            printf("%s:\n", args[i]);
                            list(args[i]);
                        }
                    }
                } 
                //builtin kill
                else if (strcmp(command, "kill") == 0) {
                    printf("executing builtin kill:\n");
                    if (argsct > 0) {
                        if (argsct > 1) {
                            if (args[0][0] != '-') {
                                printf("invalid signal number, invoke with -<signal number>\n");
                            } else {
                                i = -1*atoi(args[0]);
                                k_pid = atoi(args[1]);
                                kill(k_pid, i);
                            }
                        } else {
                            k_pid = atoi(args[0]);
                            kill(k_pid, SIGTERM);
                        }
                    }
                } 
                //builtin printenv
                else if (strcmp(command, "printenv") == 0) {
                    printf("executing builtin printenv\n");
                    if (argsct == 0) {
                        for (char **env = envp; *env != 0; env++) {
                            printf("%s = %s\n", *env, getenv(*env));
                        }
                    } else if (argsct > 1) {
                        fprintf(stderr, "more than one environment variable was specified\n");
                    } else {
                        char *env;
                        if ((env = getenv(args[0])) == NULL) {
                            printf("invalid env\n");
                        } else {
                            printf("%s = %s\n", args[0], env);
                        }
                    }
                } 
                //bulitin setenv
                else if (strcmp(command, "setenv") == 0) {
                    printf("executing builtin setenv\n");
                    if (argsct == 0) {
                        for (char **env = envp; *env != 0; env++) {
                            printf("%s = %s\n", *env, getenv(*env));
                        }
                    } else if (argsct == 1) {
                        setenv(args[0], "", 1);
                    } else if (argsct == 2) {
                        setenv(args[0], args[1], 1);
                        if (strcmp(args[0], "PATH") == 0) {
                            freepath(pathlist);
                            pathlist = get_path();
                        }
                        if (strcmp(args[0], "HOME") == 0) {
                            homedir = getenv("HOME");
                        }
                    } else {
                        fprintf(stderr, "more than two arguments were given to setenv\n");
                    }
                } 
                //non-builtin
                else if (command[0] == '/') {
                    if (access(command, X_OK) == 0) {
                        printf("executing %s\n", command);
                        if ((cpid = fork()) < 0) {
                            perror("fork");
                        } else if (cpid == 0) {
                            execve(command, args, envp);
                            printf("couldn't execute: %s\n", command);
                            exit(127);
                        } else {
                            if ((w = waitpid(cpid, &status, 0)) < 0) {
                                perror("waitpid");
                            } else if (WIFEXITED(status)) {
                                printf("Exit %d\n", WEXITSTATUS(status));
                            }
                        }
                    } else {
                        perror("access");
                    }
                } 
                //check if it is command elsewhere
                else {
                    if ((commandpath = which(command, pathlist, strlen(command)))) {
                        if (access(commandpath, X_OK) == 0) {
                            printf("executing %s\n", commandpath);
                            newargsct = 0;

                            new_args[0] = calloc(strlen(commandpath)+1, sizeof(char));
                            strcpy(new_args[0], commandpath);
                            newargsct++;
                            for (int i=0;i<argsct;i++) {
                                if (strchr(args[i], '*')!=NULL || strchr(args[i], '?')!=NULL) {
                                    glob(args[i], GLOB_DOOFFS, NULL, &globbuf);
                                    for (size_t i = 0; i!=globbuf.gl_pathc; ++i) {
                                        new_args[newargsct] = calloc(strlen(globbuf.gl_pathv[i])+1, sizeof(char));
                                        strcpy(new_args[newargsct], globbuf.gl_pathv[i]);
                                        newargsct++;
                                        printf("this is %s\n", new_args[newargsct-1]);
                                    }
                                    globfree(&globbuf);
                                } else {
                                    new_args[i+1] = calloc(strlen(args[i])+1, sizeof(char));
                                    strcpy(new_args[i+1], args[i]);
                                    newargsct++;
                                }
                            }
                            if ((cpid = fork()) < 0) {
                                perror("fork");
                            } else if (cpid == 0) {
                                execve(commandpath, new_args, envp);
                                printf("couldn't execute: %s\n", commandpath);
                                exit(127);
                            } else {
                                if ((w = waitpid(cpid, &status, 0)) < 0) {
                                    perror("waitpid");
                                } else if (WIFEXITED(status)) {
                                    printf("Exit %d\n", WEXITSTATUS(status));
                                }
                            }
                            freeargs(new_args, newargsct);
                            free(new_args);
                            new_args = calloc(MAXARGS+1, sizeof(char*));
                        } else {
                            perror("access");
                        }
                    }
                    free(commandpath);
                }
                if (args != NULL) {
                    freeargs(args, argsct);
                    free(args);
                    args = calloc(MAXARGS, sizeof(char*));
                }
			}
		} else {
            printf("\n");
            clearerr(stdin);
        }
	}
    free(args);
    free(new_args);
	free(prompt);
	free(commandline);
	free(owd);
	free(pwd);
	freepath(pathlist);
	return 0; 
}

/*
 * which searches for the command in the pathlist
 *
 * Parameters: command - the command, pathlist - the list of the PATH as a pathelement, cmdsize - the size of the command
 * Returns: The full command path
 * Side effects: callocs memory to be freed, prints to stdout where the command is located at
 */

char *which(char *command, struct pathelement *pathlist, int cmdsize) {
	char *cmd;
    cmd = calloc(PATHLEN, sizeof(char));

	while (pathlist != NULL) {
		sprintf(cmd, "%s/%s", pathlist->element, command);
		if (access(cmd, X_OK) == 0) {
			printf("%s located at [%s]\n", command, cmd);
			return cmd;
		}
		pathlist = pathlist->next;
	}
	printf("error: command not found in PATH\n");
    return cmd;
}

/*
 * where searches for the number of (command) in the pathlist
 *
 * Parameters: command - the command, pathlist - the list of the PATH as a pathelement, cmdsize - the size of the command
 * Returns: nothing
 * Side effects: prints to stdout where the command is located everytime it is found, and the number of times it is found
 */

void where(char *command, struct pathelement *pathlist, int cmdsize) {
	int cnt = 0;
	char cmd[PATHLEN];
	char buf[BUFFSIZE];

	while (pathlist != NULL) {
		sprintf(cmd, "%s/%s", pathlist->element, command);
		if (access(cmd, F_OK) == 0) {
			cnt++;
			printf("%s located at [%s]\n", command, cmd);
		}
		pathlist = pathlist->next;
	}
	snprintf(buf, 64, "where found %d copies of command", cnt);
	printf("%s\n", buf);
	return;
}

/*
 * list lists all files in current directory
 *
 * Parameters: dir - the current directory
 * Returns: nothing
 * Side effects: prints to stdout the files in the current directory
 */

void list (char *dir) {
    DIR *curr_dir;
    struct dirent *entry;

    if ((curr_dir = opendir(dir)) == NULL) 
        perror("opendir");
    else {
        while ((entry = readdir(curr_dir)) != NULL)
            printf("%s\n", entry->d_name);
        closedir(curr_dir);
    }
}

/*
 * freepath frees all the pathelements along the pathlist
 *
 * Parameters: pathlist - the root pathelement to free
 * Returns: nothing
 * Side effects: frees memory
 */

void freepath(struct pathelement *pathlist) {
	struct pathelement *tmp;

	if (pathlist)
	free(pathlist->element);

	while(pathlist!=NULL) {
		tmp = pathlist;
		pathlist = pathlist->next;
		free(tmp);
	}
}

/*
 * freeargs free all argumenst in the array
 *
 * Parameters: args - the array of args to free, argsct - the number of arguments in the args array
 * Returns: nothing
 * Side effects: frees memory
 */

void freeargs(char **args, int argsct) {
	for (int i=0;i<argsct;i++) {
		free(args[i]);
	}
}

