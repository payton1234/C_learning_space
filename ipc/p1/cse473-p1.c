
/**********************************************************************

   File          : cse473-p1.c

   Description   : This is the main file for pipe-based IPC
                   (see .h for applications)

   Last Modified : Jan 1 09:54:33 EST 2009
   By            : Trent Jaeger

***********************************************************************/
/**********************************************************************
Copyright (c) 2008 The Pennsylvania State University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of The Pennsylvania State University nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

/* Include Files */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

/* Project Include Files */
#include "cse473-p1.h"

/* Definitions */
#define CLIENT_USAGE "cse473-p1 <input> <output>"
#define TRUE 1
#define STDIN 0
#define STDOUT 1

    /* 
       send contents of a file to another process via IPC 
       or should it be multiple passes...
    */

/**********************************************************************

    Function    : main
    Description : this is the main function for project #1 pipe IPC
    Inputs      : argc - number of command line parameters
                  argv - the text of the arguments
    Outputs     : 0 if successful, -1 if failure

***********************************************************************/

/* Functions */
int main( int argc, char **argv ) 
{
    int pid;
    int read_index = 0, write_index = 0, new_index = 0;
    int err;

    /* Check for arguments */
    if ( argc < 3 ) 
    {
        /* Complain, explain, and exit */
        fprintf( stderr, "missing or bad command line arguments\n" );
        fprintf( stderr, CLIENT_USAGE );
        exit( -1 );
    }

    /* get the index for the IPC object */
    err = create_ipc( &read_index, &write_index );
    if ( err ) {
      fprintf( stderr, "create ipc error\n" );
      exit(-1);
    }

    /* create receiver process */
    if (( pid = fork() != 0 )) {
      /* child */

      /* setup the IPC channel */
      setup_ipc_child( read_index, write_index,  &new_index );
      if ( err ) {
        fprintf( stderr, "child: error in ipc setup\n" );
	exit(-1);
      }

      /* dump the file contents */
      rcv_file( argv[2], new_index );
      fprintf( stderr, "child: finished\n" );
    }
    else {
      /* parent */

      /* setup the IPC channel */
      err = setup_ipc_parent( read_index, write_index, &new_index );
      if ( err ) {
        fprintf( stderr, "parent: error in ipc setup\n" );
	exit(-1);
      }
	
      /* send the file contents */
      send_file( argv[1], new_index );
      fprintf( stderr, "parent: finished\n" );
    }

    exit( 0 );
}


/**********************************************************************

    Function    : create_ipc
    Description : obtain the descriptor to the new IPC channel 
    Inputs      : read and write index pointers (ignore the value)
    Outputs     : >0 if successful, <0 otherwise

***********************************************************************/

int create_ipc( int *read, int *write )
{
  int result;
  int fd[2];

  /* YOUR CODE HERE */
  if(pipe(fd) == -1){
    perror("pipe");
    result = -1;
  }
  else{
    *read = fd[0];
    *write = fd[1];

    result = 0;
  }
  return result;
}


/**********************************************************************

    Function    : setup_ipc_child
    Description : Using the IPC descriptor 'index', create a 'new' descriptor for reading data
    Inputs      : read -- read descriptor for pipe
                  write -- write descriptor for pipe 
                  new -- pointer to holder of new descriptor on output
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int setup_ipc_child( int read, int write, int *new ) 
{
  int err = 0; 

  /* YOUR CODE HERE */
  close(write);
  dup2(read, STDIN);
  *new = STDIN;

  return err;
}


/**********************************************************************

    Function    : setup_ipc_parent
    Description : Using the IPC descriptor 'index', create a 'new' descriptor for writing data
    Inputs      : read -- read descriptor for pipe
                  write -- write descriptor for pipe 
                  new -- pointer to holder of new descriptor on output
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int setup_ipc_parent( int read, int write, int *new ) 
{
  int err = 0;

  /* YOUR CODE HERE */
  close(read);
  if(dup2(write, STDOUT) == -1){
    printf("error = %s\n", strerror(errno));
    err = -1;
  }
  else{
    *new = STDOUT;
  }
  return err;
}


/**********************************************************************

    Function    : send_file
    Description : Send file 'name' data over IPC channel 'index'
    Inputs      : name -- file path
                  index -- IPC descriptor
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/

int send_file( char *name, int index )
{

  /* YOUR CODE HERE */
  int size, byte_read;
  struct stat sfile;
  //open the file
  int fd = open(name, O_RDONLY);
  int ret = 0;
  if (fd == -1) 
    { 
        // print which type of error have in a code 
        printf("Error Number % d\n", errno); 
          
        // print program detail "Success or failure" 
        perror("Program");                 
    } 
  //find the size of the file 
  stat(name, &sfile);
  size = sfile.st_size;
  printf("the size of the file is %d", size);
  //read from the file
  char *c = (char*) calloc(size, sizeof(char));
  byte_read = read(fd, c, size);
  c[byte_read] = '\0'; 
  close(fd);
  //write to the pipe
  if(write(index, c, strlen(c)) == -1){
    printf("fail to write");
    ret = -1;
  }
  close(index);
  return ret;
}


/**********************************************************************

    Function    : rcv_file
    Description : Receive file 'name' data over IPC channel 'index'
    Inputs      : name -- file path
                  index -- IPC descriptor
    Outputs     : 0 if successful, -1 otherwise

***********************************************************************/


int rcv_file( char *name, int index )
{

  /* YOUR CODE HERE */
  char *c = (char*) calloc(100000, sizeof(char));
  int fd, size, ret = 0, eof = 0;
  printf("starting rce_file\n");
  //open the output file
  *fd = open(name, O_WRONLY);

  if (fd == -1) 
    { 
      // print which type of error have in a code 
      printf("Error Number % d\n", errno); 
        
      // print program detail "Success or failure" 
      perror("Program");                 
    }
  size = read(index, c, 100000);
  printf("the size of the file read is %d\n", size);
  if(write(fd, c ,size) == -1){
    printf("fail to write");
    ret = -1;
  }
  close(fd);
  close(index);
  
  return ret;
}

