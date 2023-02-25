
/**********************************************************************

   File          : cse473-p1-shm.c

   Description   : This is the main file for Linux Shared Memory IPC
                   (see .h for applications)
                   See http://www.cs.cf.ac.uk/Dave/C/node27.html for info

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
#include "cse473-p1-mypipe.h"

/* Definitions */
#define CLIENT_USAGE "cse473-p1 <input> <output>"
#define TRUE 1
#define MSG_SIZE 30
#define SEND 1

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
#if SEND
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
#endif
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
#if SEND
    }
#endif

    exit( 0 );
}


/**********************************************************************

    Function    : create_ipc
    Description : obtain the descriptor to the new IPC channel 
    Inputs      : read and write index pointers (ignore the value)
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int create_ipc( int *read, int *write )
{

  /* YOUR CODE HERE */
  int shmid;
  shmid = mypipe_init();
  printf("create_ipc: shmid:%d\n", shmid);
  if(shmid == -1){
    perror("mypipe_init");
    return -1;
  }
  *read = shmid;
  *write = shmid;
  // printf("create_ipc finished\n");
  return 0;
}


/**********************************************************************

    Function    : setup_ipc_child
    Description : Using the IPC descriptor 'index', create a 'new' descriptor for reading data
    Inputs      : read -- read descriptor for mypipe
                  write -- write descriptor for mypipe
                  new -- pointer to holder of new descriptor on output
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int setup_ipc_child( int read, int write, int *new ) 
{
  int err = 0;
  /* do child-specific setup of IPC */
  err = mypipe_attach( read, new );
  if(err == -1) {
    perror("mypipe_attach");
    return -1;
  }
  // printf("setup_ipc_child finished\n");
  return err;
}


/**********************************************************************

    Function    : setup_ipc_parent
    Description : Using the IPC descriptor 'index', create a 'new' descriptor for writing data
    Inputs      : read -- read descriptor for mypipe
                  write -- write descriptor for mypipe
                  new -- pointer to holder of new descriptor on output
    Outputs     : 0 if successful, <0 otherwise

***********************************************************************/

int setup_ipc_parent( int read, int write, int *new ) 
{
  int err = 0;
  /* do parent-specific setup of IPC*/ 
  err = mypipe_attach( write, new );
  if(err == -1) {
    perror("mypipe_attach");
    return -1;
  }
  // printf("setup_ipc_parent finished\n");
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
  printf("starting: send file\n");
  FILE *fptr = fopen(name, "r");
  char* buf;
  int eof = 0, count = 0;
  while(eof == 0) {
    memset(buf, 0, MSG_SIZE);
    readline(fptr, buf, MSG_SIZE, &eof);
    //printf("stored \"%s\" in the buffer, sizeof(buf) = %ld\n", buf, strlen(buf));
    //printf("entering mypipe_write\n");
    mypipe_write(index, buf, strlen(buf));
    //printf("exited mypipe_write\n");
    count++;
  }
  sleep(0.5);
  memset(buf, 0, MSG_SIZE);
  mypipe_write(index, "EOF", strlen("EOF"));
  //printf("ending: send file\n");
  

  return 0;
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
  printf("starting: rcv file\n");
  FILE *fptr = fopen(name, "w");
  char buf[MSG_SIZE];
  int byte, count = 0;
  for(;;) {
    printf("round %d\n", count);
    memset(buf, 0, MSG_SIZE);
    byte = mypipe_read(index, buf, MSG_SIZE);
    printf("byte = %d\n", byte);
    printf("stored \"%s\" in the buffer\n", buf);
    if(strcmp(buf, "EOF") == 0) {
      printf("Reached the end of the file\n");
      break;
    }
    fwrite(buf, sizeof(char), byte, fptr);
    count ++;
  }
  printf("ending: rcv file\n");

  return 0;
}
