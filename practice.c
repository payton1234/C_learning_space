#include <stdio.h>
int main ( void )
{
  char filename[] = "file.txt";
  FILE *file = fopen ( filename, "r" );

  if (file != NULL) {
    char line [1000];
    while(fgets(line,sizeof line,file)!= NULL) /* read a line from a file */ {
      fprintf(stdout,"%s",line); //print the file contents on stdout.
    }

    fclose(file);
  }
  else {
    perror(filename); //print the error message on stderr.
  }

  return 0;
}