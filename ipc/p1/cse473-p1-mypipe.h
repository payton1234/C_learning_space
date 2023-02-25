extern int mypipe_init( void );
extern int mypipe_attach( int index, int *new );
extern int mypipe_read( int index, char *buf, int max );
extern int mypipe_write( int index, char *buf, int len );

