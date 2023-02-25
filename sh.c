#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
// Simplifed xv6 shell.

#define MAXARGS 10

char tmpout[512] = {0};
char* inputfile;
char* outputfile;

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or > 
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // |
  struct cmd *left;  // left side of pipe
  struct cmd *right; // right side of pipe
};

int fork1(void);  // Fork but exits on failure.
struct cmd *parsecmd(char*);


void do_ls() 
{
  fprintf(stderr, "list files\n");  
    
  DIR *dir;
  struct dirent *entry;

  if ((dir = opendir(".")) == NULL)
    perror("opendir() error");
  else {
    while ((entry = readdir(dir)) != NULL) {
      char tmpstr[512];
      sprintf(tmpstr, "%s ", entry->d_name);
      // fprintf(stderr, "### tmpstr:%s\n", tmpstr);
      strcat(tmpout, tmpstr);
      strcat(tmpout, "\n");  
      // fprintf(stderr, "### tmpout:%s\n", tmpout);

    }
    closedir(dir);    
  }
  strcat(tmpout, "\n");  

}

void do_cat(int argc,char*argv[])
{
printf("###argc: %d\n", argc);
if(argc!=2){ //checks if two arguments are present
printf("\nThe syntax should as be follows");
printf("\nCommandname filetoread\n");
exit(1);
}
int fdold,count;
char buffer[2048]; //characer buffer to store the bytes
fdold=open(argv[1], O_RDONLY);
if(fdold==-1)
{
  printf("cannot open file\n");
  exit(1);
}
while((count=read(fdold,buffer,sizeof(buffer)))>0) //displaying the content
{
  printf("%s",buffer);
}
exit(0);
}

void sortfile(char ** arr, int linecount)
{
  for (;;) 
  {
    int swapped = 0;
    for (int j = 1; j < linecount; j++) {
        if (strcmp(arr[j - 1], arr[j]) > 0) {
            char *t = arr[j - 1];
            arr[j - 1] = arr[j];
            arr[j] = t;
            swapped = 1;
        }
    }
    if (swapped == 0)
        break;
  }
}

void do_uniq(char ** arr, int linecount) 
{
  fprintf(stderr, "starting to do uniq\n");
  int i, j;
  char **narr = malloc(sizeof(*arr) * linecount);
  narr[0] = arr[0];
  int index = 1;
  for (j = 1; j < linecount; j++)
  {
    if(strcmp(arr[j], arr[j-1]) != 0)
    {
      narr[index] = arr[j];
      index += 1;
    }
  }
  for (i = 0; narr[i] != NULL; i++)
  {
    arr[i] = narr[i];
  }
  for (j = i; j < linecount; j++)
  {
    arr[j] = NULL;
    fprintf(stderr, "arr[%d]: %s\n", j, arr[j]);
  }
}

void do_wc(char * filename)
{
  const char *myfile = filename;
   int bytes = 0;
   int words = 0;
   int newLine = 0;
   char buffer[1];
   int file = open(myfile,O_RDONLY);
   enum states { WHITESPACE, WORD };
   int state = WHITESPACE;
   if(file == -1){
      printf("can not find :%s\n",myfile);
   }
   else{
      char last = ' '; 
      while (read(file,buffer,1) ==1 )
      {
         bytes++;
         if ( buffer[0]== ' ' || buffer[0] == '\t'  )
         {
            state = WHITESPACE;
         }
         else if (buffer[0]=='\n')
         {
            newLine++;
            state = WHITESPACE;
         }
         else 
         {
            if ( state == WHITESPACE )
            {
               words++;
            }
            state = WORD;
         }
         last = buffer[0];
      }        
      printf("%d %d %d %s\n",newLine,words,bytes,myfile);        
   } 
}


void do_wirte_file(char* filename)
{ 
  fprintf(stderr, "calling do_wirte_file:%s\n", filename);
  FILE * pFile;
  pFile = fopen (filename, "wb");
  fprintf(stderr, "### tmpout:%s\n", tmpout);
  fwrite (tmpout , sizeof(char), strlen(tmpout)+1, pFile);
  fclose (pFile);
  fprintf(stderr, "calling do_wirte_file\n");         
}

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2], r;
  struct execcmd *ecmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
  {
    fprintf(stderr, "exit with 0:%d\n", __LINE__);
    exit(0);
  }
  
  switch(cmd->type){
  default:
    fprintf(stderr, "unknown runcmd\n");
    exit(-1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0) {
      fprintf(stderr, "exit with 0:%d\n", __LINE__);
      exit(0);
    }

    //do_ls
    fprintf(stderr, "### ecmd->argv[0]: %s\n", ecmd->argv[0]);
    fprintf(stderr, "### ecmd->argv[1]: %s\n", ecmd->argv[1]);
    if(strcmp(ecmd->argv[0], "ls") == 0) {
      do_ls();
    }

    //do_cat
    if(strcmp(ecmd->argv[0], "cat") == 0) {
      if (ecmd->argv[1] == NULL)
      {
        ecmd->argv[1] = inputfile;
      }
      do_cat(2, ecmd->argv);
    }

    //sort/uniq
    if(strcmp(ecmd->argv[0], "sort") == 0 || strcmp(ecmd->argv[0], "uniq") == 0){
      FILE *fileIN, *fileOUT;
      char singleline[500];
      int i, linecount;

      fileIN = fopen(ecmd->argv[1], "r");
      if (fileIN == NULL) {
          fprintf(stderr, "cannot open %s\n", ecmd->argv[1]);
          exit(1);
      }

      linecount = 0;
      while (fgets(singleline, 500, fileIN)) {
          linecount++;
      }

      printf("line count=%d\n", linecount);

      char **arr = malloc(sizeof(*arr) * linecount);
      if (arr == NULL) {
          fprintf(stderr, "memory allocation failure\n");
          exit(1);
      }

      rewind(fileIN);
      for (i = 0; i < linecount && fgets(singleline, 500, fileIN) != NULL; i++) {
          arr[i] = strdup(singleline);
          if (arr[i] == NULL) {
              fprintf(stderr, "memory allocation failure\n");
              exit(1);
          }
      }
      fclose(fileIN);

      if (i != linecount) {
          fprintf(stderr, "line count mismatch: i=%d, lilnecount=%d\n",i, linecount);
          linecount = i;
      }
      fprintf(stderr, "argv[0]: %s\n", ecmd->argv[0]);
      fprintf(stderr, "strcmp(ecmd->argv[0], sort): %d\n", strcmp(ecmd->argv[0], "sort"));
      if(strcmp(ecmd->argv[0], "sort") == 0) {
        fprintf(stderr, "starting to sort\n");
        sortfile(arr, linecount);
        
      }
      else {  
        do_uniq(arr, linecount);
      }
      for (i = 0; i < linecount; i++) {
          printf("%s", arr[i]);
      }
      fileOUT = fopen("out.txt", "w");
      if (!fileOUT) {
          fprintf(stderr, "cannot open %s\n", "out.txt");
          exit(1);
      }
      for (i = 0; i < linecount; i++) {
          fprintf(fileOUT, "%s", arr[i]);
      }
      fclose(fileOUT);

      for (i = 0; i < linecount; i++) {
          free(arr[i]);
      }
      free(arr);
    }

    //do_wc
    if(strcmp(ecmd->argv[0], "wc") == 0) {
      printf("starting to do wc\n");
      do_wc(ecmd->argv[1]);
    }

    if(strcmp(ecmd->argv[0], "rm") == 0) {
      printf("starting to do rm\n");
      if (remove(ecmd->argv[1]) == 0) 
      {
        printf("The file is deleted successfully.\n");
      } 
      else 
      {
        printf("The file is not deleted.\n");
      }
    }

    else {
      fprintf(stderr, "exec not implemented\n");      
    }    
    // Your code here ...
    break;

  case '>':
    rcmd = (struct redircmd*)cmd;
    // char* argv0 = ecmd->argv[0];    
    // fprintf(stderr, "argv0: %s\n", argv0);
    // char* filename = ecmd->argv[1];
    fprintf(stderr, "ecmd->argv[1]: %s pid:%d\n", rcmd->file, getpid());
    
    // fprintf(stderr, "rcmd->cmd: %s\n", rcmd->cmd);
    // char* tmpstr = "hello world";
    fprintf(stderr, "### before calling runcmd\n");    
    runcmd(rcmd->cmd);
    fprintf(stderr, "### after calling runcmd\n");
    do_wirte_file(rcmd->file);
    
    
    break;
  case '<':
    rcmd = (struct redircmd*)cmd;
    //fprintf(stderr, "redir not implemented\n");
    // Your code here ...
    fprintf(stderr, "### before calling runcmd\n");   
    inputfile = rcmd->file; 
    printf("###inputfile: %s\n", inputfile);
    runcmd(rcmd->cmd);
    fprintf(stderr, "### after calling runcmd\n");
    break;

  case '|':
    pcmd = (struct pipecmd*)cmd;
    //fprintf(stderr, "pipe not implemented\n");
    // Your code here ...
    printf("pcmd->right: %c, pcmd->left: %c\n", pcmd-> right->type, pcmd->left->type);
    int fd[2];
    if (pipe(fd) == -1)
    {
      printf("pipe failed");
      exit(1);
    }
    int pid = fork();
    if (pid < 0)
    {
      printf("fork failed");
      exit(1);
    }
    if (pid == 0)
    {
      //child process
      dup2(fd[1], STDOUT_FILENO);
      close(fd[0]);
      close(fd[1]);
      runcmd(pcmd->right);
    }

    int pid2 = fork();
    if (pid2 < 0)
    {
      printf("fork failed");
      exit(1);
    }
    if (pid2 == 0)
    {
      //child process
      dup2(fd[0], STDIN_FILENO);
      close(fd[0]);
      close(fd[1]);
      runcmd(pcmd->left);
    }

    waitpid(pid, NULL, 0);
    waitpid(pid2, NULL, 0);
    break;
  }    

  //fprintf(stderr, "exit with 0:%d pid:%d\n", __LINE__, getpid());
  // exit(0);
}

int
getcmd(char *buf, int nbuf)
{
  
  if (isatty(fileno(stdin)))
    fprintf(stdout, "$ ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void)
{
  static char buf[100];
  int fd, r;

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(stderr, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait(&r);
  }
  fprintf(stderr, "exit with 0:%d pid:%d\n", __LINE__, getpid());

  exit(0);
}

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '<':
    s++;
    break;
  case '>':
    s++;
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;
  
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char 
*mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    exit(-1);
  }
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  cmd = parsepipe(ps, es);
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a') {
      fprintf(stderr, "missing file for redirection\n");
      exit(-1);
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;
  
  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      fprintf(stderr, "syntax error\n");
      exit(-1);
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      exit(-1);
    }
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  return ret;
}