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
#include "sh.h"
#include <errno.h>
#include <glob.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
char* built_in_commands[] = {"exit", "which", "where", "cd", "pwd", "list", "pid", "kill", "prompt", "printenv", "setenv", "watchuser", "noclobber"};
#define BUFFER_SIZE 1024
extern char **environ;



void zombie(int sig) {
  int child;
  waitpid((pid_t) (-1), &child, WNOHANG);
}
void sig_intCatcher(int button) {  
    signal(SIGINT, sig_intCatcher);
    fflush(stdout);
}
void sig_tstpCatcher(int button) {
  signal(SIGTSTP, sig_tstpCatcher);
  fflush(stdout);
}
void sig_termCatcher(int button) {
  signal(SIGTERM, sig_termCatcher);
  fflush(stdout);
}
void *mythread(void *param) {
  char **arg = (char **) param;
  execve(arg[0], arg, environ);
  perror("execve");
}
int sh( int argc, char **argv, char **envp )
{
  
  char buf[BUFFER_SIZE];
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  int noclobber = 0;
  struct pathelement *pathlist;
  
  struct stat b;
  int red_index;
  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;/* Home directory to start
				      out with*/
  /* if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
    {
      perror("getcwd");
      exit(2);
    }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd)); */
  prompt[0] = ' '; prompt[1] = '\0';
  char* output;
  /* Put PATH into a linked list */
  pathlist = get_path();
  signal(SIGINT, sig_intCatcher);
  signal(SIGTSTP, sig_tstpCatcher);
  signal(SIGTERM, sig_termCatcher); 
  signal(SIGCHLD, zombie);
  int redirect;
  char *red_op;
  while ( go )
    {
      redirect = 0;
      owd = getcwd(NULL, PATH_MAX + 1);
      signal(SIGINT, sig_intCatcher);
      signal(SIGTSTP, sig_tstpCatcher);
      signal(SIGTERM, sig_termCatcher);
      /* print your prompt */
      int i = 0;
      for (int j = 0; j < MAXARGS; j++) {
            args[j] = NULL;
        }
      printf("%s[%s]> ", prompt, owd);
      /* get command line and process */
      
      
      if (fgets(buf, BUFFER_SIZE, stdin) == NULL) {
        printf("\n Use exit\n");
        continue;
      }
      
      char* token = strtok(buf, " \n\t");
      
      if (token == NULL) {
        continue;
      }
      i = 0;

      while (token != NULL) {
       if ((strstr(token, "*") != NULL) || (strstr(token, "?") != NULL)) {
         
          glob_t paths;
          int csource;    // check if token has wildcard
          char **p;
          csource = glob(token, 0, NULL, &paths);
          
          if (csource == 0) {
            for (p = paths.gl_pathv; *p != NULL; ++p) {
              //printf("%s\n", *p);
              args[i] = strdup(*p);
              
              i++;
            }
            args[i] = NULL;
            globfree(&paths);
          }
          token = strtok(NULL, " \n\t");
          
        }
        else {
          
          args[i] = token;
          token = strtok(NULL, " \n\t");
          i++;
        }
      }
      
      int args_count = i;
      
      for (int i = 0; i < args_count; i++) {
        if ((strcmp(args[i], ">")==0) || (strcmp(args[i], ">&")==0) || (strcmp(args[i], ">>")==0) 
            || (strcmp(args[i], ">>&")==0) || (strcmp(args[i], "<")==0)) {
              redirect = 1;
              red_op = args[i];
              red_index = i;
            }

      }

      /* check for each built in command and implement */
      //printf("%s", args[1]);
      if (strcmp(args[0], "which") == 0) {
        printf("Executing built-in %s\n", args[0]); //calls which function
        if (args_count < 2) {
        fprintf(stderr, "%s: Too few arguments.\n", args[0]);
        continue;
        }
        
        for (int j = 1; j < args_count; j++) {
          
          output = which(args[j], pathlist);
          if (output == NULL) {
            fprintf(stderr, "%s: Command not found.\n", args[j]);
            continue;
          }
          else {
            printf("%s\n", output);
            free(output);
          }
        }
        continue;  
      }

      else if (strcmp(args[0], "noclobber") == 0) {
        if (noclobber) {
          noclobber = 0;
        }
        else {
          noclobber = 1;
        }
      }

      else if (strcmp(args[0], "watchuser") == 0) {
        printf("Executing built-in %s\n", args[0]);
        if (args_count == 2) {

        }
        else if (args_count == 3) {

        }
        else {
          fprintf(stderr, "%s: Incorrect amount of arguments.\n", args[0]);
        }
      }

      else if (strcmp(args[0], "list") == 0) {
        printf("Executing built-in %s\n", args[0]);
        if (args_count == 1) {
          list(getcwd(NULL, PATH_MAX + 1));
        }
        else {
          for (int n = 1; n < args_count; n++) {
            if (args[n] != NULL) {
              printf("%s:\n", args[n]);
              list(args[n]);
            }
          }
        }
      }

      else if (strcmp(args[0], "where") == 0) {
        printf("Executing built-in %s\n", args[0]);
        if (args_count < 2) {
          fprintf(stderr, "%s: Too few arguments.\n", args[0]);
          continue;
        }

        for (int j = 1; j < args_count; j++) {
          where(args[j], pathlist);
        }
        continue;
      }

      else if (strcmp(args[0], "exit") == 0) {
        
        struct pathelement *next;
        printf("Executing built-in %s\n", args[0]);
        free(owd);
        free(commandline);
        free(prompt);
        free(args);
        while (pathlist != NULL) {
          next = pathlist->next;
          free(pathlist);
          pathlist = next;
        }
        
        return 0;
      }

      else if (strcmp(args[0], "pid") == 0) {
        printf("PID: %d\n", getpid());
        continue;
      }

      else if (strcmp(args[0], "pwd") == 0) {
        pwd = getcwd(NULL, PATH_MAX + 1);
        printf("%s\n", pwd);
      }

      else if (strcmp(args[0], "cd") == 0) {
        char *home = getenv("HOME");
        if (args_count == 1) {
          setenv("OLDPWD", getenv("PWD"), 1);
          setenv("PWD", home, 1);
          chdir(home);
        }
        else if (args_count == 2) {
          if (strcmp(args[1], "-") == 0) {
            char* temp = getenv("PWD");
            chdir(getenv("OLDPWD"));
            setenv("PWD", getenv("OLDPWD"), 1);
            setenv("OLDPWD", temp, 1);
                      
          }
          else {
            char *t = getenv("PWD");  //set environment variables appropiately
            if (chdir(args[1]) < 0) {
              perror("Not a directory");
              continue;
            }
            setenv("OLDPWD", t, 1);
            pwd = getcwd(NULL, PATH_MAX + 1);
            setenv("PWD", pwd, 1);
            
          }
        }
        else {
          fprintf(stderr, "%s: Too many arguments.\n", args[0]);
          continue;
        }
      }

      else if (strcmp(args[0], "kill") == 0) {
        kill_proc(args_count, args);
      }

      else if (strcmp(args[0], "prompt") == 0) {
        char *s = calloc(PROMPTMAX, sizeof(char));
        int c = 0;
        while (prompt[c] != '\0') { //process until null terminating
          prompt[c] = '\0';
          c++;
        }   
        c = 0; 
        s = print_prompt(args, args_count);
        while ((s[c] != '\n') && (s[c] != '\0')) { //account for \n input
          prompt[c] = s[c];
          c++;
        }
        prompt[c] = ' ';
        free(s);
      }
      
      else if (strcmp(args[0], "setenv") == 0) {
        if ((strcmp(args[1],"PWD") == 0) || (strcmp(args[1], "OLDPWD") == 0)) {
          continue;
        }
        if (args_count == 1) {
          print_env(args_count, environ, args);
          continue;
        } 
        else if (args_count == 2) {
          setenv(args[1], "", 1);    //switch between diff arguments for setenv
        }
        else if (args_count == 3) {
          setenv(args[1], args[2], 1);
        }
        if (strcmp(args[0], "PATH") == 0) {
          struct pathelement *next;
          while (pathlist != NULL) {
          next = pathlist->next;
          free(pathlist);
          pathlist = next;
        }
          pathlist = get_path();
        }
      }

      else if (strcmp(args[0], "printenv") == 0) {
         print_env(args_count, environ, args);
         continue;
      }
      
      else {
        printf("Executing %s\n", args[0]);
        pid_t pid;
        char *cmd;
        int ex = 0;
        output = which(args[0], pathlist);
        if (output != NULL) {
          cmd = output;
          ex = 1;
        }
        else if (access(args[0], F_OK) == 0) {
          cmd = args[0];
          ex = 1;
        }
        int background = 0;
        if (strcmp(args[args_count-1], "&") == 0) {
            args[args_count-1] = NULL;
            background = 1;
        }
        if ((pid = fork()) < 0) {
          printf("fork error\n");
        }
        else if (pid == 0) {
          if (redirect) { //redirect detected
            int file_exists = 0;
            if (access(args[red_index+1], F_OK) == 0) {
              file_exists = 1;
            }
            if (strcmp(red_op, ">") == 0) {
              if ((noclobber) && (file_exists)) {
                fprintf(stderr, "%s: File exists.\n", args[args_count - 1]);
                continue;
              }
              int fid = open(args[red_index+1], O_WRONLY|O_CREAT|O_TRUNC, 0777);
              close(1);
              dup(fid);
              close(fid);
            }
            else if (strcmp(red_op, ">&") == 0) {
              if ((noclobber) && (file_exists)) {
                fprintf(stderr, "%s: File exists.\n", args[args_count - 1]);
                continue;
              }
              int fid = open(args[red_index+1], O_WRONLY|O_CREAT|O_TRUNC, 0777);
              close(1);
              dup(fid);
              close(2);
              dup(fid);
              close(fid);
            }
            else if (strcmp(red_op, ">>") == 0) {
              if ((noclobber) && (!file_exists)) {
                fprintf(stderr, "%s: File does not exist.\n", args[args_count - 1]);
                continue;
              }
              int fid = open(args[red_index+1], O_WRONLY|O_CREAT|O_APPEND, 0777);
              close(1);
              dup(fid);
              close(fid);
            }
            else if (strcmp(red_op, ">>&") == 0) {
              if ((noclobber) && (!file_exists)) {
                fprintf(stderr, "%s: File does not exist.\n", args[args_count - 1]);
                continue;
              }
              int fid = open(args[red_index+1], O_WRONLY|O_CREAT|O_APPEND, 0777);
              close(1);
              dup(fid);
              close(2);
              dup(fid);
              close(fid);
            }
            else if (strcmp(red_op, "<") == 0) {
              int fid = open(args[red_index+1], O_RDONLY);
              close(0);
              dup(fid);
              close(fid);
            }
            for (int i = red_index; i < args_count; i++) {
              args[i] = NULL;
            }
          }

          if (ex) {
            execve(cmd, args, environ);
          }
          else {
            fprintf(stderr, "%s: Command not found.\n", args[0]);
            exit(0);
          }

        }
        else {
          int child_stat;
          if (background) {
            waitpid(pid, &child_stat, WNOHANG);
          }
          else {
            waitpid(pid, &child_stat, 0);
          }
        }
        if (output != NULL) {
          free(output);
        }
      }
	/* find it */
	/* do fork(), execve() and waitpid() */
      
     // else
     //   fprintf(stderr, "%s: Command not found.\n", args[0]);
      
      
    }
    
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
  for (int i = 0; i < 11; i++) {
    if (strcmp(command, built_in_commands[i]) == 0) {
      printf("%s: built in command\n", command);
      return NULL;
    }
  }
  while (pathlist != NULL) {
    char *s = malloc(strlen(pathlist->element) + strlen(command) + 1);
    strcpy(s, pathlist->element);
    strcat(s, "/");
    strcat(s, command);
    if (access(s, F_OK) == 0) {
      return s;
    }
    free(s);
    pathlist = pathlist->next;
  }
  
  return NULL;
  /* loop through pathlist until finding command and return it.  Return
     NULL when not found. */
  //while (pathlist) {

  //}
} /* which() */

void where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
  for (int i = 0; i < 11; i++) {
      if (strcmp(command, built_in_commands[i]) == 0) {
        printf("%s: built in command\n", command);
        return;
      }
  }

  while (pathlist != NULL) {
    char *s = malloc(strlen(pathlist->element) + strlen(command) + 1);
    strcpy(s, pathlist->element);
    strcat(s, "/");
    strcat(s, command);
    if (access(s, F_OK) == 0) { //check if file is there
      printf("%s\n", s);
    }

    pathlist = pathlist->next;
    free(s);
  }

  
} /* where() */

void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
     the directory passed */
  DIR *d = opendir(dir);
  struct dirent *r;
  if (d == NULL) {
    printf("%s\n", "Not a directory");
  }
  else {
    while ((r = readdir(d)) != NULL) {
      printf("%s\n", r->d_name);
    }
    closedir(d);
  }
} /* list() */

void print_env(int argcount, char **envp, char **args) {
  if (argcount == 1) {
          int count = 0;
          while (envp[count] != NULL) {
            printf("%s\n", envp[count]);
            count++;
          }
  }
  else if (argcount == 2) {
    if(getenv(args[1]) != NULL) {
      printf("%s\n", getenv(args[1]));
    }
  }
  else {
    fprintf(stderr, "%s: Too many arguments.\n", args[0]);
  }
}

void kill_proc(int argscount, char **args) {
  
  if (argscount == 1) {
    fprintf(stderr, "%s: Too few arguments.\n", args[0]);
  }
  else if (argscount == 2) {
    long pid = strtol(args[1], NULL, 10); //convert given pid
    
    if (errno == EINVAL) {
      perror("Not a process id");
      return;
    }
    printf("%d", kill(getpid(), SIGTERM));
  }
  else if (argscount == 3) {
    char *sig = malloc(strlen(args[1]) * sizeof(char));
    strcpy(sig, args[1]);
    sig[0] = ' ';
    long pid = strtol(args[2], NULL, 10);
    if (errno == EINVAL) {
      perror("Not a process id");
      return;
    }
    long signum = strtol(sig, NULL, 10);
    if (errno == EINVAL) {
      perror("Not a process id");
      return;
    }
    pid_t num = (pid_t) pid;
    int s = (int) signum;
    free(sig);
    kill(num, s);
  }
} 
char *print_prompt(char **args, int argcount) {
  char *inp;
  if (argcount == 1) {
    printf("%s", "input prompt prefix: ");
    while (fgets(inp, BUFFER_SIZE, stdin) == NULL) {
      printf("%s", "input prompt prefix: ");
    }
    return inp;
  }
  else {
    inp = args[1];
    return inp;
  }
}

