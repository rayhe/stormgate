/*   
 *  Sigh.... all of the whacked out Identd code... whoooooo        
 *  Much thanks to Simkin at harmless@h.imap.itd.umich.edu
 *  Basically, start_auth is called once a person connects to
 *  the port which the mud is running on, start_auth opens a connection
 *  to port 113 at the host of the user which just connected
 *  (port 113 is used as a identd daemon if the system has one)
 *  then, next time through the unix_game_loop, it calls send_auth
 *  to send the information which is being requested, then next
 *  time through, read_auth gets whatever information was returned
 *  and searches through it for the accountname
 */

/*$Id: id.c,v 1.11 2005/02/22 23:55:17 ahsile Exp $*/

#ifdef RUN_AS_WIN32SERVICE
#include <winsock2.h>
#endif

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "merc.h"

#ifndef RUN_AS_WIN32SERVICE
#include <netdb.h>
#include <arpa/telnet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define DUNNO_STRICMP
#endif

#define FLAG_WRAUTH   1 /* Auth unsent yet, send if able */
#define FLAG_AUTH     2 /* Authorization in progress     */

extern int   maxdesc;
#ifdef RUN_AS_WIN32SERVICE
extern int   _write   args( ( int fd, char *buf, int nbyte ) );
extern int   _read    args( ( int fd, char *buf, int nbyte ) );
#else
extern int   write   args( ( int fd, char *buf, int nbyte ) );
extern int   read    args( ( int fd, char *buf, int nbyte ) );
#endif
extern int   close   args( ( int fd ) );
void  nonblock     args( ( int s ) );
void  start_auth   args( ( struct descriptor_data *d ) );
void  read_auth    args( ( struct descriptor_data *d ) );
void  send_auth    args( ( struct descriptor_data *d ) );
void  userl_update args( ( struct descriptor_data *d, bool login ) );
void  userl_load   args( ( void ) );
void  userl_save   args( ( void ) );
void  ul_sort      args( ( USERL_DATA *ul ) );

#if defined(DUNNO_STRICMP)
int   stricmp      args( ( char *a, char *b ) );
#endif
USERL_DATA *      user_list;

/* Start_auth - yay,  TRI */

void  start_auth( struct descriptor_data *d )
{
  struct  sockaddr_in sock;
    int   err;     /* error & result stuffs */
    int   tlen;
   
   if ( !str_prefix( "130.63.236", d->host ) 
      || !str_prefix( "130.63", d->host ) )
   {
     free_string(d->user);
     d->user = str_dup( "(ncsa format)" );
     return;
   }
   
   d->auth_fd = socket( AF_INET, SOCK_STREAM, 0 );
   err = errno;
   
   if ( d->auth_fd < 0 && err == EAGAIN )
     bug( "Can't allocate fd for authorization check", 0 );
   nonblock( d->auth_fd );

   /* Clone incoming host address */
   tlen = sizeof( sock );
     getpeername( d->descriptor, ( struct sockaddr * )&sock, &tlen );
     sock.sin_port = htons( 113 );
     sock.sin_family = AF_INET;
   
   #ifdef RUN_AS_WIN32SERVICE  
   if ( ( connect( d->auth_fd, ( struct sockaddr *)&sock, sizeof(sock)) == -1 ))
   #else
   if ( ( connect( d->auth_fd, ( struct sockaddr *)&sock, sizeof(sock)) == -1 )
     && ( errno != EINPROGRESS ))
   #endif
   {
     /* Identd Denied */
     bug( "Unable to verify userid", 0 );
     close( d->auth_fd );
     free_string(d->user);
     d->user = str_dup( "(no verify)" );
     d->auth_fd = -1;
     d->auth_state = 0;
     d->atimes = 70;
     return;
   }
   
#if !defined(RUN_AS_WIN32SERVICE)
   if ( errno == ECONNREFUSED )
   {
     close( d->auth_fd );
     d->auth_fd = -1;
     free_string(d->user);
     d->user = str_dup( "(no identd)" );
     d->auth_state = 0;
     d->atimes = 70;
     return;
   }
#endif
   
   d->auth_state |= ( FLAG_WRAUTH|FLAG_AUTH ); /* Successful, but not sent */
   if ( d->auth_fd > maxdesc ) maxdesc = d->auth_fd; 
   return;
}

/* send_auth */

void  send_auth( struct descriptor_data *d )
{
  struct  sockaddr_in  us, them;
    char  authbuf[32];
    int   ulen, tlen, z;
    
  tlen = ulen = sizeof( us );
  
  if ( getsockname( d->descriptor, ( struct sockaddr *)&us, &ulen )
      || getpeername( d->descriptor, ( struct sockaddr *)&them, &tlen ) )
  {
    bug( "auth getsockname error", 0 );
    goto authsenderr;
  }
  
  /* compose request */
  sprintf( authbuf, "%u , %u\r\n", 
    (unsigned int)ntohs(them.sin_port),
    (unsigned int)ntohs(us.sin_port));

  #ifdef RUN_AS_WIN32SERVICE
  z = send( d->auth_fd, authbuf, strlen( authbuf ), 0 );
  #else
  z = write( d->auth_fd, authbuf, strlen( authbuf ) );
  
  if ( errno == ECONNREFUSED )
  {
    close( d->auth_fd );
    d->auth_fd = -1;
    free_string(d->user);
    d->user = str_dup( "(no identd)" );
    d->auth_state = 0;
    return;
  }
  #endif

  if ( z != (int) strlen( authbuf ) )
  {
    if ( d->atimes >= 69 )
    {
      sprintf( log_buf, "auth request, broken pipe [%d/%d]", z, errno );
      bug( log_buf, 0 );
    }
    authsenderr:
    close( d->auth_fd );
    if ( d->auth_fd == maxdesc ) maxdesc--; 
    d->auth_fd = -1;
    d->auth_state &= ~FLAG_AUTH;    /* Failure/Continue */
    d->auth_inc = 0;
    if ( d->atimes < 70 ) d->atimes++;
  }
  d->auth_state &= ~FLAG_WRAUTH;  /* Successfully sent request */
  return;
}

/* read_auth */
void  read_auth( struct descriptor_data *d )
{
  char     *s, *t;
  int      len;                    /* length read */
  char     ruser[20], system[8];   /* remote userid */
  u_short  remp = 0, locp = 0;     /* remote port, local port */
  #ifdef RUN_AS_WIN32SERVICE
  struct sockaddr blah;
  int  blahlen = sizeof(blah);
  #endif
  *system = *ruser = '\0';
  
  
  /*
   * Can't allow any other reads from client fd while waiting on the
   * authfd to return a full valid string.  Use the client's input buffer
   * to buffer the authd reply.  May take more than one read.
   */
   
   #ifdef RUN_AS_WIN32SERVICE
   if ( ( len = recvfrom( d->auth_fd, d->abuf + d->auth_inc,
     			sizeof( d->abuf ) - 1 - d->auth_inc, 0, &blah, &blahlen ) ) >= 0 )
   #else
   if ( ( len = read( d->auth_fd, d->abuf + d->auth_inc,
     			sizeof( d->abuf ) - 1 - d->auth_inc ) ) >= 0 )
   #endif
   {
      d->auth_inc += len;
      d->abuf[d->auth_inc] = '\0';
    }
    
    if ( d->abuf[0] != '\0' )
    bug( d->abuf, 0 );

    if ( ( len > 0 ) && ( d->auth_inc != ( sizeof( d->abuf ) - 1 ) ) &&
       (sscanf( d->abuf, "%hd , %hd : USERID : %*[^:]: %10s",
          &remp, &locp, ruser ) == 3 ) )
    {
	  #ifdef RUN_AS_WIN32SERVICE
	  s = strrchr(d->abuf, ':');
	  #else
      s = rindex( d->abuf, ':');
	  #endif
      *s++ = '\0';
	  #ifdef RUN_AS_WIN32SERVICE
	  for ( t = ( strrchr( d->abuf, ':' ) + 1 ); *t; t++ )
	  #else
      for ( t = ( rindex( d->abuf, ':' ) + 1 ); *t; t++ )
	  #endif
            if ( !isspace(*t) )
               break;
      strncpy( system, t, sizeof( system ) );
      
      if ( !str_prefix( "OTHER", system ) )
      {
        close( d->auth_fd );
        if ( d->auth_fd == maxdesc ) maxdesc--;
        d->auth_state = 0;
        d->auth_fd = -1;
	free_string(d->user);
        d->user = str_dup( "(invalid identd)" );
        d->atimes = 70;
        return;
      }
      
      for ( t = ruser; *s && ( t < ruser + sizeof( ruser ) ); s++ )
            if ( !isspace( *s ) && *s != ':' )
               *t++ = *s;
      *t = '\0';
 
      sprintf( log_buf, "auth reply ok, incoming user: [%s]", ruser );
      bug( log_buf, 0 );
    }
   else if ( len != 0 )
    {
	  #ifdef RUN_AS_WIN32SERVICE
	  if (!strchr( d->abuf, '\n' ) && !strchr( d->abuf, '\r' ) ) return;
	  #else
	  if (!index( d->abuf, '\n' ) && !index( d->abuf, '\r' ) ) return;
	  #endif
      sprintf( log_buf, "bad auth reply: %s", d->abuf );
      bug( log_buf, 0 );
      *ruser = '\0';
    }
  close( d->auth_fd );
  if ( d->auth_fd == maxdesc ) --maxdesc; 
  d->auth_inc = 0;
  *d->abuf = '\0';
  d->auth_fd = -1;
  d->auth_state = 0;
  if (ruser[0] == '\0')
    strcpy(ruser, "(no auth)" );
  free_string(d->user);
  d->user = str_dup(ruser);
  userl_update( d, FALSE );     
  return;
}

void nonblock( int s )
{
#if !defined(RUN_AS_WIN32SERVICE)
  if ( fcntl( s, F_SETFL, FNDELAY ) == -1 )
  {
    perror( "Noblock" );
  bug( "Noblock", 0 );
  }
#endif
}

void userl_load( )
{
  FILE *fp;
  extern FILE *fpArea;
  extern char strArea[MAX_INPUT_LENGTH];
  char* buf;

  bug ( "Loading User_list...", 0 );
  
  if ( !( fp = fopen( USERLIST_FILE, "r" ) ) )
      return;

  for ( ; ; )
  {
    char letter;
    USERL_DATA *ul;

    do
    {
      letter = getc(fp);
      if ( feof(fp) || letter == '$' )
      {
	fclose(fp);
	bug("Done Loading User List.",0);
	return;
      }
    }
    while ( isspace(letter) );
    ungetc(letter, fp);

    ul = new_userl();

    if ( str_cmp( fread_word(fp), "name" ) )
      break;
    ul->name = fread_string(fp);
    if ( str_cmp( fread_word(fp), "lvl" ) )
      break;
    ul->level = fread_number(fp);
    if ( str_cmp( fread_word(fp), "user" ) )
      break;
    ul->user = fread_string(fp);
    if ( str_cmp( fread_word(fp), "host" ) )
      break;
    ul->host = fread_string(fp);
    if ( str_cmp( fread_word(fp), "last" ) )
      break;
    ul->lastlogin = fread_string(fp);
    if ( str_cmp( fread_word(fp), "desc" ) )
      break;
    ul->desc = fread_string(fp);
    buf = fread_word(fp);
    if ( !str_cmp( buf , "name" ) )
    {
      // push the word back onto the stream
      ungetc(' ', fp); ungetc('e', fp); ungetc('m', fp); ungetc('a', fp); ungetc('n', fp);
      ul->class = 20;
      ul->multi = 20;
    } else
    {
       if(!str_cmp(buf,"$"))
       {
         ungetc('$', fp);
       }
       else
       {
       	 ul->class = fread_number(fp);
       	 fread_word(fp);
       	 ul->multi = fread_number(fp);
       }
    }
    if ( ul->level == 0 || ul->level == 1 )
    {
      free_userl(ul);
      continue;
    }
    ul_sort(ul);
  }
  
  strcpy( strArea, USERLIST_FILE );
  fpArea = fp;
  bug("Load_userl: bad key word.", 0 );
  ___exit( 1 );
  return;
}

  
void userl_update( struct descriptor_data *d, bool login )
{
  int found = 0;
  USERL_DATA * ul;
  
  if ( !d->character )
    return;
  for ( ul = user_list; ul; ul = ul->next )
  {
    if ( !str_cmp( d->character->name, ul->name ) )
      found = 1;
    if ( ( found == 1 ) && ( !str_cmp( ul->user, "(unknown)" ) 
    || !str_cmp( ul->user, "(no auth)" ) )
       && str_cmp( d->user, "(unknown)" ) )
      found = 2;
    if ( found > 0 ) break;
  }
  
  if ( found == 1 )
  {
    if ( login )
    {
      sprintf( log_buf, "On since %s", ctime( &current_time ) );
      *(log_buf + strlen(log_buf) - 1) = '\0';
      free_string(ul->lastlogin);
      ul->lastlogin = str_dup( log_buf );
    }
    ul->class = d->character->class;
    ul->multi = d->character->multied;
    ul->level = d->character->level;
    free_string(ul->host);
    ul->host = str_dup( d->host );
    free_string(ul->user);
    ul->user = str_dup( d->user );
    return;
  }     
  
  if ( found == 0 )
  {
    ul = new_userl();
    ul->name = str_dup( d->character->name );
    ul->level = d->character->level;
    ul->user = str_dup( d->user );
    ul->host = str_dup( d->host );
    ul->class = d->character->class;
    ul->multi = d->character->multied;
    sprintf( log_buf, "On since %s", ctime( &current_time ) );
    *(log_buf + strlen(log_buf) - 1) = '\0';
    ul->lastlogin = str_dup( log_buf );
    ul->desc = str_dup( "(none)" );
    ul_sort(ul);
    return;
  }
  
  if ( found == 2 )
  {
    if ( login )
    {
      sprintf( log_buf, "On since %s", ctime( &current_time ) );
      *(log_buf + strlen(log_buf) - 1) = '\0';
      free_string(ul->lastlogin);
      ul->lastlogin = str_dup( log_buf );
    }
    ul->class = d->character->class;
    ul->multi = d->character->multied;    
    ul->level = d->character->level;
    free_string(ul->user);
    ul->user = str_dup( d->user );
    free_string(ul->host);
    ul->host = str_dup( d->host );
    return;
  }
  return;
}
  
  
void userl_save( )
{
  FILE *fp;
  USERL_DATA * ul;
  char filename[30];
  
  sprintf( filename, "users.txt" );
  fclose( fpReserve );
  if ( !( fp = fopen( filename, "w" ) ) )
    perror( filename );
  else
  {
    ul = NULL;
    for ( ul = user_list; ul; ul = ul->next )
    {
      fprintf( fp, "Name   %s~\n", ul->name );
      fprintf( fp, "Lvl    %d\n", ul->level );
      fprintf( fp, "User   %s~\n", ul->user );
      fprintf( fp, "Host   %s~\n", ul->host );
      fprintf( fp, "Last   %s~\n", (ul->lastlogin ?
				    ul->lastlogin : "(none)") );
      fprintf( fp, "Desc   %s~\n", (ul->desc ? ul->desc : "(none)") );
      fprintf( fp, "Class  %d\n", ul->class );
      fprintf( fp, "Multi  %d\n", ul->multi );

    }
    fprintf( fp, "$\n" );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

#if defined DUNNO_STRICMP
/*
 * Note: only works for alpha strings.  No punctuation or numbers, or things
 * might go messy.. :)..
 * returns: 1 if a is larger, -1 if b is larger, or 0 if equal
 * -- Altrag
 */
int stricmp( char *a, char *b )
{
  char *ap;
  char *bp;

  for ( ap = a, bp = b; ; ap++, bp++ )
  {
    if ( (*ap == *bp) && *ap == '\0' )
      return 0;
    if ( *ap == '\0' )
      return -1;
    if ( *bp == '\0' )
      return 1;
    if ( LOWER(*ap) < LOWER(*bp) )
      return -1;
    if ( LOWER(*bp) < LOWER(*ap) )
      return 1;
  }
  return 0;
}
#endif

void ul_sort( USERL_DATA *ul )
{
  USERL_DATA *user;

  if ( !user_list )
  {
    user_list = ul;
    return;
  }

  for ( user = user_list; user; user = user->next )
  {
    if (!stricmp(ul->name, user->name) ||
       ( stricmp(ul->name, user->name) > 0 &&
       (!user->next || stricmp(ul->name, user->next->name) < 0) ) )
    {
      ul->next = user->next;
      user->next = ul;
      return;
    }
  }
  ul->next = user_list;
  user_list = ul;
  return;
}
