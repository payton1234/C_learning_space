extern int create_ipc( int *, int * );
extern int setup_ipc_child( int, int, int * );
extern int setup_ipc_parent( int, int, int * );
extern int send_file( char *, int );
extern int rcv_file( char *, int );

static inline int readline( FILE *fp, char *buf, int max, int *eof )
{
  int len = 0;
  char c;

  /* get next entry */
  while ( len < max ) {
    c = fgetc( fp );
    if ( c == EOF ) {
      *eof = 1;
      break;
    }
    buf[len++] = c;
    if ( c == '\n' )
      break;
  }      

  return len;
}
