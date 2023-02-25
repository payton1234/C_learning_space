
/**********************************************************************

   File          : cse473-p1-mypipe.c

   Description   : This is the library file for Linux Shared Memory IPC
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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sched.h>

    /* 
       send contents of a file to another process via IPC 
       or should it be multiple passes...
    */

#include "cse473-p1-mypipe.h"

#define SHM_KEY 1236
#define SHM_SZ  32

struct mypipe {
  int shmid;
  int read;
  int write;
  int size;
  char shm[SHM_SZ];
};

struct mypipe *only_pipe = NULL;

/**********************************************************************

    Function    : mypipe_init
    Description : create a new shared memory pipe -- return an index for communication
    Inputs      : none
    Outputs     : >0 if successful, -1 if failure

***********************************************************************/

/* Functions */
int mypipe_init( void )
{
  int shmid;

  /* YOUR CODE HERE */
  shmid = shmget(SHM_KEY, sizeof(struct mypipe), IPC_CREAT | 0666);
  if(shmid == -1) {
    perror("shmget");
    return -1;
  }

  // printf("mypipe_init: shmid:%d\n", shmid);
  // only_pipe->shmid = shmid;
  // printf("shmid: %d, only_pipe->shmid: %d\n", shmid, *only_pipe->shmid);
  return shmid;
}


/**********************************************************************

    Function    : mypipe_attach
    Description : create a new shared memory region and setup
    Inputs      : none
    Outputs     : >0 if successful, -1 if failure

***********************************************************************/


int mypipe_attach( int index, int *new )
{

  /* YOUR CODE HERE */
  // printf("mypipe_attach: shmid:%d\n", index);
  only_pipe = (struct mypipe*)shmat(index, NULL, 644 | IPC_CREAT);
  new = (int*)only_pipe;
  only_pipe->shmid = index;
  only_pipe-> read = 0;
  only_pipe-> write = 0;
  only_pipe->size = SHM_SZ;
  only_pipe->shm[SHM_SZ];
  //memset(only_pipe->shm, 0, SHM_SZ);
  if(*new == -1) {
    perror("shmat");
    return -1;
  }
  /*new = (int*)shmat(index, NULL, 0);
  struct mypipe* p = (struct mypipe*)new;
  p->shmid = index;
  p->read = 0;
  p->write = 0;
  p->size = 0;*/
  return 0;
}



/**********************************************************************

    Function    : mypipe_read
    Description : read message from pipe into buffer of max bytes
    Inputs      : index to mypipe
                  buf to write characters from pipe
                  max is number of bytes maximum
    Outputs     : >0 if successful (number of bytes read), <0 otherwise

***********************************************************************/

int mypipe_read( int index, char *buf, int max )
{
  int read, write;
  int bytes;
  
  /* need to store vars -- in case of concurrent update */
  read = only_pipe->read;
  write = only_pipe->write; 

  /* if write == read, give the writer a chance */
  while ( write == read ) { 
    sched_yield(), read = only_pipe->read, write = only_pipe->write; 
  }

  /* YOUR CODE HERE */
  //printf("starting mypipe_read\nmessage in shared memory: %s\n", only_pipe->shm);
  // happy path
  if(write > read) {
    //printf("path 1\n");
    memcpy(buf, only_pipe->shm + read, write -read);
    bytes = write - read;
  }
  else {
    // need to memcpy twice
    //printf("path 2\n");
    memcpy(buf, only_pipe->shm + read, only_pipe->size - read);
    memcpy(buf + (only_pipe->size - read), only_pipe->shm, write);
    bytes = write + only_pipe->size - read;
  }
  // -----------------
  char debugbuf[128];
  //memcpy(debugbuf, only_pipe->shm + read, write -read);
  //debugbuf[write -read] = '\0';
  printf("copied to buf: \"%s\"\n", buf);
  only_pipe-> read = ( read + bytes ) % only_pipe->size;


  return bytes;
}


/**********************************************************************

    Function    : mypipe_write
    Description : write message from buffer into shm of len bytes
    Inputs      : index to mypipe
                  buf to read characters for pipe
                  len is number of bytes to read
    Outputs     : >0 if successful (number of bytes read), <0 otherwise

***********************************************************************/

int mypipe_write( int index, char *buf, int len ) 
{
  int read, write;
#if DEBUG
  printf("only_pipe (write): %d; %d; %d; %d; %s\n", 
	only_pipe->shmid,
	only_pipe->read,
	only_pipe->write,
	only_pipe->size,
	only_pipe->shm);
#endif

 /* need to store vars -- in case of concurrent update */
  read = only_pipe->read;
  write = only_pipe->write; 
  //printf("write: %d, read: %d\n", write, read);
  
  /* cannot let the writes overlap the reads */
  while ((( write >= read ) && (( write + len ) >= ( only_pipe->size + read ))) ||
	 (( write < read ) && (( write + len ) >= read ))) { 
    sched_yield(), read = only_pipe->read, write = only_pipe->write; 
  }

  /* YOUR CODE HERE */
  //printf("starting mypipe_write\nbuf sent: %s, sizeof(buf) = %d\n", buf, len);
  if(write + len < SHM_SZ) {
    memcpy(only_pipe->shm + write, buf, len);
    printf("cpoied to shared memory: \"%s\"\n", only_pipe->shm);
  }
  else {
    memcpy(only_pipe->shm + write, buf, only_pipe->size - write);
    memcpy(only_pipe->shm, buf + (only_pipe->size - write), ( write + len ) % only_pipe->size);
  }
  only_pipe->write = ( write + len ) % only_pipe->size;

  return len;
}

