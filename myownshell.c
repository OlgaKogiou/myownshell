#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<stdio.h>
#include<dirent.h>
#define DIR_PATH "/bin"
#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line, int* words);
char *lsh_read_line();
void lsh_execute(char **args, int* words);
void lsh_loop();
int main(int argc, char **argv)
{
  // Load config files, if any.
 for(;;){
  // Run command loop.
  lsh_loop();
  // Perform any shutdown/cleanup.
 }
  return EXIT_SUCCESS;
}
//Read: Read the command from standard input.
//Parse: Separate the command string into a program and arguments.
//Execute: Run the parsed command.
void lsh_loop()
{
  char *line;
  char **args;
  int words;


    printf("olga@myownshell:$>> ");
    line = lsh_read_line();
    args = lsh_split_line(line, &words);
    lsh_execute(args, &words);
    free(line);
    free(args);
}

char *lsh_read_line(){
        int bufsize = LSH_RL_BUFSIZE;
        int position = 0;
          char *buffer = malloc(sizeof(char) * bufsize);
          int c;
          if (!buffer) {
            fprintf(stderr, "lsh: allocation error\n");
            exit(EXIT_FAILURE);
   }

  while (1) {
    // Read a character

    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return.
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }
    else {
      buffer[position] = c;
    }
    position++;
    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);

      }
    }
  }

}
char **lsh_split_line(char *line, int* words)
{
  int bufsize = LSH_TOK_BUFSIZE, position=0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  *words=position;
  return tokens;
}

void lsh_execute(char **args, int* words){
        int fd[2];
        pipe(fd);
        pid_t pid;
        int status,out,save_out, j=0;
        char **comm1, **comm2, **comm3;
        switch(pid=fork()){
        case -1:{
         exit(EXIT_FAILURE); //an error in the parent process
         break;
        }
        case 0:{ //process child
        int i=0, w=0;
        pid_t pd;
        do{
                if(strcmp(args[j],">>") == 0){
                        comm1 = (char**)malloc(sizeof(char*) * (j));
                        for(i=0; i<j; i++){
                            comm1[i]=(char*)malloc(sizeof(char)*(strlen(args[i])));
                            strcpy(comm1[i], args[i]);
                        }
                        out= open(args[j+1], O_RDWR|O_CREAT|O_APPEND, 0600);
                        if (-1 == out)  perror("opening...");
                        save_out = dup(fileno(stdout));
                        if (-1 == dup2(out, fileno(stdout)))  perror("cannot redirect stdout");
                        execvp(comm1[0], comm1);
                        fflush(stdout); close(out);
                        dup2(save_out, fileno(stdout));
                        close(save_out);
                }
                
                else if(strcmp(args[j],"|") == 0){
                        comm2 = (char**)malloc(sizeof(char*) * (j));
                        for(i=0; i<j; i++){
                            comm2[i]=(char*)malloc(sizeof(char)*(strlen(args[i])));
                            strcpy(comm2[i], args[i]);
                        }
                        i=0;
                        comm3 = (char**)malloc(sizeof(char*) * (*words-j-1));
                        for(i=j+1; i<*words; i++){
                            comm3[w]=(char*)malloc(sizeof(char)*(strlen(args[i])));
                            strcpy(comm3[w], args[i]);
                            w++;
                        }
                        switch(pd=fork()){
                            case 0: {
                                dup2(fd[1],STDOUT_FILENO); //redirect stdout
                                execvp(comm2[0], comm2);
                                close(fd[1]);
                                //fflush(stdout);
                                break;
                            }
                            default:{
                                waitpid(-1,&status,0);
                                close(fd[1]);
                                dup2(fd[0], STDIN_FILENO);
                                execvp(comm3[0], comm3);
                                close(fd[0]); close(fd[1]);
                                return;
                            }
                        }
                        
                }
                
                j++;
        }while(j<*words);
        if(j==*words){
            execvp(args[0], args);
        }
         break;
        }
         default:
         waitpid(-1,&status,0);
          return;
                if(pid==0)
                        exit(EXIT_FAILURE);

        }

}

