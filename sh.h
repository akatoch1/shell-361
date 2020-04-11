
#include "get_path.h"

int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
void where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void print_env(int argcount, char **envp, char **args);
void kill_proc(int argcount, char **args);
char *print_prompt(char **args, int argcount);
#define PROMPTMAX 32
#define MAXARGS 10
