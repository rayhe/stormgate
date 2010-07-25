/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  Envy Diku Mud improvements copyright (C) 1994 by Michael Quan, David   *
 *  Love, Guilherme 'Willie' Arnold, and Mitchell Tse.                     *
 *                                                                         *
 *  In order to use any part of this Envy Diku Mud, you must comply with   *
 *  the original Diku license in 'license.doc', the Merc license in        *
 *  'license.txt', as well as the Envy license in 'license.nvy'.           *
 *  In particular, you may not remove either of these copyright notices.   *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/*$Id: comm.c,v 1.65 2005/03/30 15:18:27 ahsile Exp $*/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

//#define RUN_AS_WIN32SERVICE  //  Only Uncomment for windows.
				//  Unfortunately, this has to be
				//  defined here because windows.h
				//  doesn't like to be included
				//  after merc.h. 
				

#ifdef RUN_AS_WIN32SERVICE

#include <winsock2.h>
#include <direct.h>

#endif

#if defined( macintosh )
#include <types.h>
#endif

#if defined ( linux ) || defined( unix)
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"

#define FLAG_WRAUTH   1
#define FLAG_AUTH     2

/*
 * Signal handling.
 * Apollo has a problem with __attribute( atomic ) in signal.h,
 *   I dance around it.
 */
#if defined( apollo )
#define __attribute( x )
#endif

#if defined( unix )
#include <signal.h>
#endif

#if defined( apollo )
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined( macintosh ) || defined( MSDOS ) || defined(RUN_AS_WIN32SERVICE)
/*
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
*/

/* Manually ripped these from arpa/telnet.h 
	- Ahsile 
*/
#define TELOPT_ECHO	1
#define GA			249
#define WILL		251
#define WONT		252
#define IAC			255

#pragma warning( disable : 4305 )

const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };

#endif


#if	defined( unix )
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/telnet.h>
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif



/*
 * OS-dependent declarations.
 */
#if	defined( _AIX )
#include <sys/select.h>
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			       int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if     defined( irix )
void	bzero		args( ( char *b, int length ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
int	close		args( ( int fd ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#endif

#if	defined( apollo )
#include <unistd.h>
void	bzero		args( ( char *b, int length ) );
#endif

#if	defined( __hpux )
int	accept		args( ( int s, void *addr, int *addrlen ) );
int	bind		args( ( int s, const void *addr, int addrlen ) );
void	bzero		args( ( char *b, int length ) );
int	getpeername	args( ( int s, void *addr, int *addrlen ) );
int	getsockname	args( ( int s, void *name, int *addrlen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	setsockopt	args( ( int s, int level, int optname,
			       const void *optval, int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if     defined( interactive )
#include <net/errno.h>
#include <sys/fcntl.h>
#endif

// Removed by Ahsile for CygWin
#if	defined( linux ) || defined( RUN_AS_WIN32SERVICE )
int	close		args( ( int fd ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
//int	listen		args( ( int s, int backlog ) );
int	_select		args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int	_socket		args( ( int domain, int type, int protocol ) );
extern void   read_auth   args( ( struct descriptor_data *d ) );/* TRI */
extern void   send_auth   args( ( struct descriptor_data *d ) );/* TRI */
extern void   start_auth  args( ( struct descriptor_data *d ) ); /* TRI */
bool    check_ban         args( ( struct descriptor_data *dnew, bool loggedin ) );
extern void  userl_update args( ( struct descriptor_data *d, bool login ) );
#endif
#ifdef RUN_AS_WIN32SERVICE
int	_read		args( ( int fd, char *buf, int nbyte ) );
int	_write		args( ( int fd, char *buf, int nbyte ) );
#endif
#ifdef linux
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined( macintosh )
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct	timeval
{
	time_t	tv_sec;
	time_t	tv_usec;
};
#if	!defined( isascii )
#define	isascii( c )		( ( c ) < 0200 )
#endif
static	long			theKeys	[4];

int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined( MIPS_OS )
extern	int		errno;
#endif

#if	defined( MSDOS )
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) ); /* JWD */
int	kbhit		args( ( void ) );
#endif

#if	defined( NeXT )
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined( htons )
u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined( ntohl )
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			       fd_set *exceptfds, struct timeval *timeout ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined( sequent )
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
int	close		args( ( int fd ) );
int	fcntl		args( ( int fd, int cmd, int arg ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined( htons )
u_short	htons		args( ( u_short hostshort ) );
#endif
int	listen		args( ( int s, int backlog ) );
#if	!defined( ntohl )
u_long	ntohl		args( ( u_long hostlong ) );
#endif
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			       fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
			       int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

/*
 * This includes Solaris SYSV as well
 */

#if defined( sun )
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			       fd_set *exceptfds, struct timeval *timeout ) );
#if defined( SYSV )
int     setsockopt      args( ( int s, int level, int optname,
			       const char *optval, int optlen ) );
#else
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			       int optlen ) );
#endif
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined( ultrix )
int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
void	bzero		args( ( char *b, int length ) );
int	close		args( ( int fd ) );
int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
int	listen		args( ( int s, int backlog ) );
int	read		args( ( int fd, char *buf, int nbyte ) );
int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
			       fd_set *exceptfds, struct timeval *timeout ) );
int	setsockopt	args( ( int s, int level, int optname, void *optval,
			       int optlen ) );
int	socket		args( ( int domain, int type, int protocol ) );
int	write		args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_free;	/* Free list for descriptors	*/
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    merc_down;		/* Shutdown                     */
bool		    wizlock;		/* Game is wizlocked		*/
int                 numlock = 0;        /* Game is numlocked at <level> */
int                 iRace;
char		    str_boot_time [ MAX_INPUT_LENGTH ];
time_t		    current_time;	/* Time of this pulse		*/



/*
 * OS-dependent local functions.
 */
#if defined( macintosh ) || defined( MSDOS ) 
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length, CHAR_DATA *ch ) ); 
#endif

#if defined( unix ) || defined (RUN_AS_WIN32SERVICE)
void	game_loop_unix		args( ( unsigned int control ) );
int	init_socket		args( ( int port ) );
void	new_descr		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length, DESCRIPTOR_DATA *d ) );
#endif

/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, char *name,
				       bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( DESCRIPTOR_DATA *d ) );
void    bust_a_color_prompt     args( ( DESCRIPTOR_DATA *d ) );

/* Semi-local functions that arent OS dependant.. :)..
   -- Altrag */
void note_cleanup               args( ( void ) );
int port;
int maxdesc;
int ch_invcount args( ( CHAR_DATA* ch ) );
char    *initial        args( ( const char *str ) );

#if defined( unix ) || defined(RUN_AS_WIN32SERVICE)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
           struct sockaddr_in sa;
                  int         x        = 1; 
                  int         fd;

	#if !defined(RUN_AS_WIN32SERVICE)
    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	#else
	WSADATA wsaData;
	WSAStartup(0x202,&wsaData);
	if ( ( fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
	#endif
    {
	perror( "Init_socket: socket" );
	___exit( 0 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof( x ) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close( fd );
	___exit( 0 );
    }

#if defined( SO_DONTLINGER ) && !defined( SYSV )
    {
	struct	linger	ld;

	ld.l_onoff  = 1;
	ld.l_linger = 1000;

	if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
	(char *) &ld, sizeof( ld ) ) < 0 )
	{
	    perror( "Init_socket: SO_DONTLINGER" );
	    close( fd );
	    ___exit( 0 );
	}
    }
#endif
	
    sa		    = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port	    = htons( (unsigned short) port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof( sa ) ) < 0 )
    {
	perror( "Init_socket: bind" );
	close( fd );
	___exit( 0 );
    }

    if ( listen( fd, 3 ) < 0 )
    {
	perror( "Init_socket: listen" );
	close( fd );
	___exit( 0 );
    }

    return fd;
}
#endif



#if defined( macintosh ) || defined( MSDOS )
void game_loop_mac_msdos( void )
{
    static        DESCRIPTOR_DATA dcon;
           struct timeval         last_time;
           struct timeval         now_time;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor	= 0;
    dcon.character      = NULL;
    dcon.connected	= CON_GET_NAME;
    dcon.host		= str_dup( "localhost" );
    dcon.outsize	= 2000;
    dcon.outbuf		= alloc_mem( dcon.outsize );
    dcon.showstr_head   = str_dup( "" );
    dcon.showstr_point  = 0;
    dcon.pEdit		= NULL;			/* OLC */
    dcon.pString	= NULL;			/* OLC */
    dcon.editor		= 0;			/* OLC */
    dcon.next		= descriptor_list;
    descriptor_list	= &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting_one;
	extern char * help_greeting_two;
	extern char * help_greeting_three;
	extern char * help_greeting_four;
	extern char * help_greeting_five;
/*	if ( help_greeting[0] == '.' )
	    write_to_buffer( &dcon, help_greeting+1, 0 );
	else
	    write_to_buffer( &dcon, help_greeting  , 0 ); */
        switch number_range(1, 5)
	{
	    case 1:
		write_to_buffer( &dcon, help_greeting_one, 0 );
	    case 2:
		write_to_buffer( &dcon, help_greeting_two, 0 );
	    case 3:
		write_to_buffer( &dcon, help_greeting_three, 0 );
	    case 4:
		write_to_buffer( &dcon, help_greeting_four, 0 );
	    case 5:
		write_to_buffer( &dcon, help_greeting_five, 0 );
	}
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

#if defined( MSDOS )
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character )
				save_char_obj( d->character, FALSE );

		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		/* OLC */
		if ( d->showstr_point )
		    show_string( d, d->incomm );
		else
		if ( d->pString )
		    string_add( d->character, d->incomm );
		else
		    if ( d->connected == CON_PLAYING )
		    {
			if ( !run_olc_editor( d ) )
			    interpret( d->character, d->incomm ); 
		    }
		    else
			nanny( d, d->incomm );
		d->incomm[0]	= '\0';
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character )
			save_char_obj( d->character, FALSE );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined( MSDOS )
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character )
			save_char_obj( d->character, FALSE );
		    dcon.outtop	= 0;
		    close_socket( &dcon );
		}
#if defined( MSDOS )
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined( unix ) || defined(RUN_AS_WIN32SERVICE)
void game_loop_unix( unsigned int control )
{
    static struct timeval null_time;
           struct timeval last_time;

	#ifndef RUN_AS_WIN32SERVICE
    signal( SIGPIPE, SIG_IGN );
	#endif
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

	

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;
	fd_set           in_set;
	fd_set           out_set;
	fd_set           exc_set;

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc	= control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, (int) d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	#if defined(RUN_AS_WIN32SERVICE)
	if ( select( 0, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	#else
	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	#endif
	{
	    continue; // attempt to fix things
	    //perror( "Game_loop: select: poll" );
	    //___exit( 0 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
		new_descr( control );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character )
		    save_char_obj( d->character, FALSE );
		d->outtop	= 0;
		close_socket( d );
                continue;
	    }
	    if ( ( d->auth_fd == -1 ) && ( d->atimes < 70 ) && 
	      !str_cmp( d->user, "(unknown)" ) )
	    {
	      start_auth( d );
	      continue;
	    }
	    if ( d->auth_fd == -1 ) continue;
	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
	      read_auth( d );
	      if ( !d->auth_state ) check_ban( d, TRUE ); 
            }
           else if ( ( FD_ISSET( d->descriptor, &out_set ) ) &&
                 IS_SET( d->auth_state, FLAG_WRAUTH ) )
            {
              send_auth( d );
              if ( !d->auth_state ) check_ban( d, TRUE );
            } 
	}
	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d; d = d_next )
	{
	    d_next	= d->next;
	    d->fcommand	= FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) )
	    {
		if ( d->character )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character )
			save_char_obj( d->character, FALSE );
		    d->outtop	= 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if ( d->character && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    read_from_buffer( d );
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand	= TRUE;
		stop_idling( d->character );

		/* OLC */
		if ( d->showstr_point )
		    show_string( d, d->incomm );
		else
		if ( d->pString )
		    string_add( d->character, d->incomm );
		else
		{
		    char buf[MAX_INPUT_LENGTH];
		    char buf2[MAX_STRING_LENGTH];
		    FILE *fp;

		    sprintf( buf, "comlog%d.txt", port );
/*
		    sprintf( buf2, "%s: %s: %s\n", ctime( &current_time ),
			     d->character ? d->character->name : "(Unknown)",
			     d->incomm );
*/
		    sprintf( buf2, "%s: %s\n", d->character ? d->character->name : "(Unknown)", d->incomm );

		    log_string( buf2, CHANNEL_COMLOG, L_CON);

		    fclose(fpReserve);
		    if ( (fp = fopen( buf, "a" ) ) )
		    {
		      fprintf( fp, "%s: %s: %s\n", ctime( &current_time ),
			       d->character ? d->character->name : "(Unknown)",
			       d->incomm );
		      fclose(fp);
		    }
		    fpReserve = fopen( NULL_FILE, "r" );
		
		    if ( d->connected == CON_PLAYING )
		    {
			if ( !run_olc_editor( d ) )
			    interpret( d->character, d->incomm ); 
		    }
		    else
			nanny( d, d->incomm );
		}
		d->incomm[0]	= '\0';
	    }
	}
	/*
	 * Autonomous game motion.
	 */
	update_handler( );
	
	/*
	 * Output.
	 */
	for ( d = descriptor_list; d; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
		&& FD_ISSET( d->descriptor, &out_set ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character )
			save_char_obj( d->character, FALSE );
		    d->outtop	= 0;
		    close_socket( d );
		}
	    }
	}
	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;
		
	    gettimeofday( &now_time, NULL );
	    usecDelta	= ( (int) last_time.tv_usec )
	                - ( (int)  now_time.tv_usec )
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta	= ( (int) last_time.tv_sec )
	                - ( (int)  now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		#ifndef RUN_AS_WIN32SERVICE
		/* - Not working anymore (?)
		 * - Use sleep instead

		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    ___exit( 0 );
		}

		 * - Ahsile
		 */
		char buf[MAX_STRING_LENGTH];
		sprintf(buf, "sleep %ld.%ld", secDelta, usecDelta/1000);
		system(buf);
		#else
		/* Force sleep on windows machines - Ahsile */
		int sleep_time = (secDelta * 1000) + (usecDelta/1000);
		Sleep(sleep_time);
		#endif
	    }
	}
	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined( unix ) || defined(RUN_AS_WIN32SERVICE)
void new_descr( int control )
{
    static DESCRIPTOR_DATA  d_zero;
           DESCRIPTOR_DATA *dnew;
    struct sockaddr_in      sock;
    struct hostent         *from;
    char                    buf [ MAX_STRING_LENGTH ];
    int                     desc;
    int                     size;

    size = sizeof( sock );
    getsockname( control, (struct sockaddr *) &sock, &size );
    if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 )
    {
	perror( "New_descriptor: accept" );
	return;
    }

#if !defined( FNDELAY )
#if defined( __hpux )
#define FNDELAY O_NONBLOCK
#else
#define FNDELAY O_NDELAY
#endif
#endif

	#ifndef RUN_AS_WIN32SERVICE
    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
	perror( "New_descriptor: fcntl: FNDELAY" );
	return;
    }
	#endif

    /*
     * Cons a new descriptor.
     */
	dnew = new_descriptor();
    *dnew		= d_zero;
    dnew->descriptor	= desc;
    dnew->character     = NULL;
    dnew->connected     = CON_GET_ANSI;
    dnew->showstr_head  = str_dup( "" );
    dnew->showstr_point = 0;
    dnew->showstr_point = NULL;
    dnew->pEdit		= NULL;			/* OLC */
    dnew->pString	= NULL;			/* OLC */
    dnew->editor	= 0;			/* OLC */
    dnew->outsize	= 2000;
    dnew->outbuf	= alloc_mem( dnew->outsize );
    dnew->ansi          = FALSE; 
    dnew->user          = str_dup( "(unknown)" ); 
    dnew->auth_inc      = 0;
    dnew->auth_fd       = -1;

    size = sizeof(sock);
    if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 )
    {
	perror( "New_descriptor: getpeername" );
	dnew->host = str_dup( "(unknown)" );
    }
    else
    {
	/*
	 * Would be nice to use inet_ntoa here but it takes a struct arg,
	 * which ain't very compatible between gcc and system libraries.
	 */
	int addr;
	int a1, a2, a3, a4;
	
	addr = ntohl( sock.sin_addr.s_addr );
	a1 = ( addr >> 24 ) & 0xFF; a2 = ( addr >> 16 ) & 0xFF;
	a3 = ( addr >>  8 ) & 0xFF; a4 = ( addr       ) & 0xFF;
	sprintf( buf, "%d.%d.%d.%d",
	    ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	    ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	    );
	sprintf( log_buf, "Sock.sinaddr:  %s", buf );
	/* hack to not log Batlin's site a17b37.rogerswave.ca
	   204.92.17.37
	   to prevent log spamming
	   6/9/96 REK
	*/
	if ( 
	  ( a1 != 204 && a1 != 130 ) &&
	  ( a2 !=  92 && a2 !=  63 ) &&
	  ( a3 !=  17 && a3 != 122 ) &&
	  ( a4 !=  37 )
	  )
	  log_string( log_buf, CHANNEL_GOD, -1 );
	from = gethostbyaddr( (char *) &sock.sin_addr,
			     sizeof(sock.sin_addr), AF_INET );
	dnew->host = str_dup( from ? from->h_name : buf );
    }
	
    /*
     * Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds.
     */
     if ( check_ban( dnew, FALSE ) ) return; 
    /*
     * Init descriptor data.
     */
    dnew->next			= descriptor_list;
    descriptor_list		= dnew;
    start_auth( dnew ); 

    /*
     * Send the greeting.
     */
    write_to_buffer( dnew, "Do you wish to use ANSI?  (Yes/No): ", 0 );
    return;
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    if ( dclose->descriptor == (unsigned int) maxdesc )
      maxdesc--;

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->original ) )
    {
      if ( !dclose->character )
      {
	sprintf( log_buf, "Close_socket: original without character: %s",
		 (dclose->original->name ? dclose->original->name
		  : "(unknown)" ) );
	bug( log_buf, 0);
	dclose->original->desc = NULL;
      }
      else
      {
	if ( IS_NPC(dclose->character) )
	  do_return( dclose->character, "" );
	else
	  dclose->original->desc = NULL;
      }
    }

    if ( ( ch = dclose->character ) )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf, CHANNEL_LOG, -1 );
	if ( !IS_SET( ch->act, PLR_WIZINVIS )
	    && !IS_AFFECTED( ch, AFF_INVISIBLE ) && !IS_AFFECTED2( ch, AFF_IMPROVED_INVIS ) )
	{
	sprintf( log_buf, "%s has lost %s link.", ch->name,
                 ch->sex == SEX_FEMALE ? "her" :
                 ch->sex == SEX_MALE ? "his" : "its" );
/* hack - Ahsile */
	if (ch->desc->connected == CON_PLAYING)
		ch->desc->connected = -1;
 	log_string( log_buf, CHANNEL_INFO, -1 );
	if (ch->desc->connected == -1)
		ch->desc->connected = CON_PLAYING;
/*end hack - Ahsile */
	}
	{
	  USERL_DATA *ul;
	  
	  for ( ul = user_list; ul; ul = ul->next )
	  {
	    if ( !str_cmp( ch->name, ul->name ) )
	    {
	      sprintf( log_buf, "Last on: %s", ctime( &current_time ) );
	      *(log_buf + strlen(log_buf) - 1) = '\0';
	      free_string(ul->lastlogin);
	      ul->lastlogin = str_dup( log_buf );
	      ul->level = ch->level;
	      break;
	    }
	  }
	}
	if ( dclose->connected == CON_PLAYING )
	{
	    act(AT_GREEN, "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    ch->desc = NULL;
	}
#ifndef SQL_SYSTEM
	else
	{
	    free_ch( dclose->character );
	}
#endif
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

	#ifndef RUN_AS_WIN32SERVICE
    close( dclose->descriptor );
    if ( dclose->auth_fd != -1 )
    { 
      close( dclose->auth_fd );
      if ( dclose->auth_fd == maxdesc ) maxdesc--;
    }
	#else
	closesocket( dclose->descriptor );
    if ( dclose->auth_fd != -1 )
    { 
      closesocket( dclose->auth_fd );
      if ( dclose->auth_fd == maxdesc ) maxdesc--;
    }
	#endif
    /* RT socket leak fix */
	free_mem( dclose->outbuf, dclose->outsize );
	dclose->outtop = 0;
	dclose->outsize = 0;
	dclose->outbuf = NULL;
    free_descriptor( dclose);
	
#if defined( MSDOS ) || defined( macintosh )
    ___exit( 0 );
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen( d->inbuf );
    if ( iStart >= sizeof( d->inbuf ) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf, CHANNEL_GOD, -1 );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0, d );
	return FALSE;
    }

    /* Snarf input. */
#if defined( macintosh )
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof( d->inbuf ) - 10 )
	    break;
    }
#endif

#if defined( MSDOS ) || defined( unix ) || defined( RUN_AS_WIN32SERVICE )
    for ( ; ; )
    {
	int nRead;

	#ifdef RUN_AS_WIN32SERVICE
	{
	nRead = recv(d->descriptor, d->inbuf + iStart, sizeof( d->inbuf ) - 10 - iStart,0);
	}
	#else
	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof( d->inbuf ) - 10 - iStart );
	#endif
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read.", CHANNEL_GOD, -1 );
	    return FALSE;
	}
	#ifndef RUN_AS_WIN32SERVICE
	else if ( errno == EWOULDBLOCK || errno == EAGAIN )
	    break;
	#endif
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    int i;
    int j;
    int k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' )
	return;

    /*
     * Look for at least one new line.
     */
    for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( d->inbuf[i] == '\0' )
	    return;
    }

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0, d );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if ( isascii( d->inbuf[i] ) && isprint( d->inbuf[i] ) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */
    if ( k > 1 || d->incomm[0] == '!' )
    {
    	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if ( ++d->repeat >= 20 )
	    {
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf, CHANNEL_GOD, -1 );
		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0, d );
		strcpy( d->incomm, "quit" );
	    }
	}
    }

    /*
     * Do '!' substitution.
     */
    if ( d->incomm[0] == '!' )
	strcpy( d->incomm, d->inlast );
    else
	strcpy( d->inlast, d->incomm );

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;
    /*
     * Bust a prompt.
     */
    if ( fPrompt && !merc_down && d->connected == CON_PLAYING )	/* OLC */
    {
        if ( d->showstr_point )
	    write_to_buffer( d,
  "[Please type (c)ontinue, (r)efresh, (b)ack, (h)elp, (q)uit, or RETURN]:  ",
			    0 );
	else
	if ( d->pString )
	    write_to_buffer( d, "> ", 2 );
	else
	{
	    CHAR_DATA *ch;
	    CHAR_DATA *victim;

/* XOR */
        /* battle prompt from Rom2 */
	ch = d->character;

        if ((victim = ch->fighting) != NULL)
        {
	  int percent;
	  char wound[100];
	  char buf[MAX_STRING_LENGTH];
 
	  if (victim->max_hit > 0)
	    percent = victim->hit * 100 / victim->max_hit;
	  else
	    percent = -1;
	  
	  if (percent >= 100)
	    sprintf(wound,"is in excellent condition.");
	  else if (percent >= 90)
	    sprintf(wound,"has a few scratches.");
	  else if (percent >= 75)
	    sprintf(wound,"has some small wounds and bruises.");
	  else if (percent >= 50)
	    sprintf(wound,"has quite a few wounds.");
	  else if (percent >= 30)
	    sprintf(wound,"has some big nasty wounds and scratches.");
	  else if (percent >= 15)
	    sprintf(wound,"looks pretty hurt.");
	  else if (percent >= 0)
	    sprintf(wound,"is in awful condition.");
	  else
	    sprintf(wound,"is bleeding to death.");
	  
	  sprintf(buf,"\n\r&r%s %s",
		  IS_NPC(victim) ? victim->short_descr : victim->name,wound);
	  buf[0] = UPPER(buf[0]);
	  write_to_buffer( d, buf, 0);
        }
	else if ( (victim = ch->hunting) && IS_NPC(victim) )
	{
	  hunt_victim( ch );
	}

	    ch = d->original ? d->original : d->character;
	    if ( IS_SET( ch->act, PLR_BLANK     ) )
	        write_to_buffer( d, "\n\r", 2 );

	    if ( IS_SET( ch->act, PLR_PROMPT    ) )
	           bust_a_prompt( d );

	    if ( IS_SET( ch->act, PLR_TELNET_GA ) )
	        write_to_buffer( d, go_ahead_str, 0 );
	}
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by )
    {
	write_to_buffer( d->snoop_by, "% ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */
    if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop, d ) )
    {
	d->outtop = 0;
	free_mem(d->outbuf, d->outsize);
	d->outsize = 1;
	d->outbuf = alloc_mem(d->outsize);
	return FALSE;
    }
    else
    {
	d->outtop = 0;
	free_mem(d->outbuf, d->outsize);
	d->outsize = 10;
	d->outbuf = alloc_mem(d->outsize);
	return TRUE;
    }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( DESCRIPTOR_DATA *d )
{
         CHAR_DATA *ch;
   const char      *str;
   const char      *i;
         char      *point;
         char       buf  [ MAX_STRING_LENGTH ];
         char       buf2 [ MAX_STRING_LENGTH ];

   ch = d->character;
   if( !ch->prompt || ch->prompt[0] == '\0' )
   {
      send_to_char(C_DEFAULT, "\n\r\n\r", ch );
      return;
   }
   i = 0;
   point = buf;
   str = d->original ? d->original->prompt : d->character->prompt;
   send_to_char(C_DEFAULT, "", ch);
   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
         case 'h' :
            sprintf( buf2, "%d", ch->hit                               );
            i = buf2; break;
         case 'H' :
            sprintf( buf2, "%d", ch->max_hit                           );
            i = buf2; break;
         case 'm' :
            if (( ch->class != 9 )&&( ch->class != 11))
               sprintf( buf2, "%d", ch->mana );
            else
               sprintf( buf2, "%d", ch->bp   );
            i = buf2; break;
         case 'M' :
            if (( ch->class != 9 )&&(ch->class != 11))
               sprintf( buf2, "%d", ch->max_mana );
            else
               sprintf( buf2, "%d", ch->max_bp   );
            i = buf2; break;
         case 'v' :
            sprintf( buf2, "%d", ch->move                              ); 
            i = buf2; break;
         case 'V' :
            sprintf( buf2, "%d", ch->max_move                          );
            i = buf2; break;
         case 'x' :
            sprintf( buf2, "%d", ch->exp                               );
            i = buf2; break;
         case 'X' :
	    sprintf( buf2, "%d", 1000 - (ch->exp % 1000)               );
	    i = buf2; break;
         case 'g' :
            sprintf( buf2, "%d", ch->gold                              );
	    i = buf2; break;
         case 'a' :
            if( ch->level >= 5 )
               sprintf( buf2, "%d", ch->alignment                      );
            else
               sprintf( buf2, "%s", IS_GOOD( ch ) ? "good"
		                  : IS_EVIL( ch ) ? "evil" : "neutral" );
            i = buf2; break;
         case 'r' :
            if( ch->in_room )
               sprintf( buf2, "%s", ( !IS_AFFECTED( ch, AFF_BLIND ) 
                                  || IS_SET( ch->act, PLR_HOLYLIGHT ) ) 
                                  ?  ch->in_room->name
                                  : "( You can't see a thing! )"       ); 
                                  /* Blind Code Added by Ahsile */
            else
               sprintf( buf2, " "                                      );
            i = buf2; break;
         case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room )
               sprintf( buf2, "%d", ch->in_room->vnum                  );
            else
               sprintf( buf2, " "                                      );
            i = buf2; break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room )
               sprintf( buf2, "%s", ch->in_room->area->name            );
            else
               sprintf( buf2, " "                                      );
            i = buf2; break;
         case '%' :
            sprintf( buf2, "%%"                                        );
            i = buf2; break;
         case 'c' :		/* OLC */
	    i = olc_ed_name( ch );
	    break;
         case 'C' :		/* OLC */
	    i = olc_ed_vnum( ch );
	    break;
         case 'l' :
            if ( ( IS_SET( ch->act, PLR_WIZINVIS ) ) || ( IS_AFFECTED( ch, AFF_INVISIBLE ) ) || ( IS_AFFECTED2( ch, AFF_IMPROVED_INVIS ) ) )
                sprintf( buf2, "Invisible" );
            else
                sprintf( buf2, "Visible" );
            i = buf2; break; 
         case 'p' :
	   {
	     extern int sAllocPerm;

	     sprintf( buf2, "%d", (sAllocPerm * 10) / (1024 * 1024) );
	     i = buf2; break;
	   }
               
      } 
      ++str;
      while( ( *point = *i ) != '\0' )
         ++point, ++i;      
   }
   write_to_buffer( d, buf, point - buf );
   return;
}


void bust_a_color_prompt( DESCRIPTOR_DATA *d )
{
         CHAR_DATA *ch;
   const char      *str;
   const char      *i;
         char      *point;
         char       buf  [ MAX_STRING_LENGTH ];
         char       buf2 [ MAX_STRING_LENGTH ];

   ch = d->character;
   if( !ch->prompt || ch->prompt[0] == '\0' )
   {
      send_to_char(C_DEFAULT, "\n\r\n\r", ch );
      return;
   }

   point = buf;
   str = d->original ? d->original->prompt : d->character->prompt;
   send_to_char(C_DEFAULT, "< ", ch);
   while( *str != '\0' )
   {
      if( *str != '%' )
      {
         *point++ = *str++;
         continue;
      }
      ++str;
      switch( *str )
      {
         default :
            i = " "; break;
         case 'h' :
            sprintf( buf2, "%dhp ", ch->hit                               );
            send_to_char(AT_YELLOW, buf2, ch);
            i = buf2;
            buf2[0] = '\0'; break;
         case 'm' :
            if (( ch->class != 9 )&&( ch->class != 11))
               {
                sprintf( buf2, "%dm ", ch->mana                              );
                send_to_char(AT_LBLUE, buf2, ch); 
               }
            else
               {
                sprintf( buf2, "%dbp ", ch->bp                              );
                send_to_char(AT_RED, buf2, ch); 
               }
            i = buf2;
            buf2[0] = '\0';break;
         case 'v' :
            sprintf( buf2, "%dmv ", ch->move                              ); 
            send_to_char(AT_GREEN, buf2, ch); 
            i = buf2;
            buf2[0] = '\0';break;
         case 'x' :
            sprintf( buf2, "%dxp ", ch->exp                               );
            send_to_char(AT_WHITE, buf2, ch); 
            i = buf2;
            buf2[0] = '\0';break;
         case 'g' :
            sprintf( buf2, "%dgp ", ch->gold                              );
            send_to_char(AT_YELLOW, buf2, ch); 
            i = buf2; 
            buf2[0] = '\0';break;
         case 'a' :
            if( ch->level < 5 )
               sprintf( buf2, "%d ", ch->alignment                      );
            else
               sprintf( buf2, "%s ", IS_GOOD( ch ) ? "good"
		                  : IS_EVIL( ch ) ? "evil" : "neutral" );
            send_to_char(AT_GREY, buf2, ch); 
            i = buf2; 
            buf2[0] = '\0';break;
         case 'r' :
            if( ch->in_room )
               sprintf( buf2, "%s ", ( !IS_AFFECTED( ch, AFF_BLIND )
                                   || IS_SET( ch->act, PLR_HOLYLIGHT ) )
                                   ?  ch->in_room->name
                                   : "( You can't see a thing! )"      );
                                   /* Blind Code Added by Ahsile */
             else
               sprintf( buf2, " "                                      );
            send_to_char(AT_RED, buf2, ch); 
            i = buf2;
            buf2[0] = '\0';break;
         case 'R' :
            if( IS_IMMORTAL( ch ) && ch->in_room )
               sprintf( buf2, "%d ", ch->in_room->vnum                  );
            else
               sprintf( buf2, " "                                      );
            send_to_char(AT_WHITE, buf2, ch); 
            i = buf2; 
            buf2[0] = '\0';break;
         case 'c' :		/* OLC */
	    send_to_char( AT_GREEN, olc_ed_name( ch ), ch );
	    i = olc_ed_name( ch );
	    break;
         case 'z' :
            if( IS_IMMORTAL( ch ) && ch->in_room )
               sprintf( buf2, "%s ", ch->in_room->area->name            );
            else
               sprintf( buf2, " "                                      );
            send_to_char(AT_RED, buf2, ch); 
            i = buf2;
            buf2[0] = '\0';break;
         case 'l' :
            if ( ( IS_SET( ch->act, PLR_WIZINVIS ) ) || ( IS_AFFECTED( ch, AFF_INVISIBLE ) ) || ( IS_AFFECTED2( ch, AFF_IMPROVED_INVIS ) ) )
                sprintf( buf2, "Invisible " );
            else
                sprintf( buf2, "Visible " );
            send_to_char(AT_BLUE, buf2, ch );
            i = buf2;
            buf2[0] = '\0';break;
      }
      ++str;
      while( ( *point = *i ) != '\0' )
         ++point, ++i;      
   *point = '\0';
   }
   send_to_char(C_DEFAULT, "> ", ch);
   return;
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen( txt );

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]	= '\n';
	d->outbuf[1]	= '\r';
	d->outtop	= 2;
    }

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

	outbuf      = alloc_mem( d->outtop + length + 1 );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf  = outbuf;
	d->outsize = d->outtop + length + 1;
    }

    /*
     * Copy.
     */
    strncpy( d->outbuf + d->outtop, txt, length );	/* OLC */
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */

/* TRI */
/* This is the within string ansi switcher; due to being in 
 * such a low level function, it is available to anything in
 * the mud which sends txt/data to a char.
 * the format for switching colors is:
 *    &color
 * i.e.:  .hahaha&rhahaha   would chat the first three ha's in the default
 *	 		    chat color, and the next 3 in darkred, with no
 *			    space between the 2 sets of 3 ha's. :>
 * 			    make sense? :P.. don't you just love me? *Phtpht*
 * the & is the char which tells the mud to check for a new clr,
 * and color is one letter, the new color.
 *    r :   dark red
 *    R :   bright red
 *    g :   dark green
 *    G :   bright green
 *    b :   dark blue
 *    B :   light blue
 *    etc
 */

bool write_to_descriptor( int desc, char *txt, int length, DESCRIPTOR_DATA *d )
{
    int iStart;
    int nWrite;
    int nBlock;
    /* TRI WHOOHOO!! */
    int clr;
const char *str;
const char *i = NULL;
    char *point;
    char buf1[MAX_STRING_LENGTH];
    char* buf2 = malloc((length * 5) + (MAX_STRING_LENGTH*2));
    int ln = 0;
    int rc = 0;
    /* End TRI */
#if defined( macintosh ) || defined( MSDOS )
    if ( desc == 0 )
	desc = 1;
#endif

memset( buf2, 0, ( (length * 5) + (MAX_STRING_LENGTH*2) ) );

/*  NOTE: it seems to work now, I had couple guys test it
	problem was that (str-txt)>=length thingy, should be <
	uncomment, test it, and do your worst =)
	the thing that doesn't work well is ofind all, buffer size
	may not be enough for VERY long strings... which is why i still
	have this commented out
	REMEMBER to uncomment the length adjustment at the end too when
	testing
*/

	point = buf2;
	str = txt;
	while ((length <= 0) ? (*str != '\0') : ((str-txt) < length))
	{
              if ( *str != '&' )
              {
                 *point++ = *str++;
                 continue;
              }
		if((str-txt+1) >= length)
		{
		  *point++ = *str++;
		  break;
		}
              ln -= 2; 
              ++str;
	      switch( *str )
              {
                default:
		  clr = -1;
		  sprintf(buf1, "&%c", *str);
		  ln += 2; 
		  i = buf1;
		break;
		case '\0':
		case '&':
		  clr = -1;
                   ln += 1;  
		  i = "&"; break;
                case 'r':
                     clr = 1; break;
                case 'R':
                     clr = 9; break;
                case 'g':
                     clr = 2; break;
                case 'G':
                     clr = 10; break;
                case 'b':
                     clr = 4; break;
                case 'B':
                     clr = 12; break;
		case 'c':
		     clr = 6; break;
                case 'C':
                     clr = 14; break;
                case 'W':
                     clr = 15; break;
                case 'Y':
                     clr = 11; break;
                case 'O':
                     clr = 3; break;
                case 'p':
                     clr = 5; break;
                case 'P':
                     clr = 13; break;
                case 'z':
                     clr = 8; break;
                case 'w':
                     clr = 7; break;
                case '.':
                     do
                     {
                       rc = number_percent();
                     }
                     while ( rc < 1 || rc > 70 || rc == 16 );
                     clr = rc;
                     break;
	      }
	  ++str;
	  if(clr == -1)
	  {
	    }
	  else if ( d->character )
	  {
	  if ( IS_SET( d->character->act, PLR_ANSI ) )  
	  {
	    if(clr > 15)
	    {
              sprintf(buf1,"\033[0;%d;5;%dm",(((clr-16)&8)==8),30+((clr-16)&7));
 		ln += strlen(buf1);  
	      i = buf1;
	    }
            else
            {
	      sprintf(buf1,"\033[0;%d;%dm",((clr & 8)==8),30+(clr & 7));
		ln += strlen(buf1);
               i = buf1; 
            }
	  }
          else
          {
            i = "";
          }
	  }
          if ( !(d->character) )
	  {
          if ( d->ansi == TRUE )
          {
            if (clr > 15 )
            {
              sprintf(buf1, "\033[0;%d;5;%dm",(((clr-16)&8)==8),30+((clr-16)&7));
              ln += strlen( buf1 );
              i = buf1;
            }
            else
            {
              sprintf( buf1, "\033[0;%d;%dm",((clr & 8)==8),30+(clr & 7));
              ln += strlen( buf1 );
              i = buf1;
            }
          }
          else
          {
            i = "";
          }
          } 
	  while((*point++ = *i++) != '\0' )
            ;
          point--;
	}
	*point++ = '\n';
	*point++ = '\r';
	*point++ = '\0';
	txt = buf2;

/* End TRI */

    if ( length <= 0 )
      length = strlen(txt);
    else
       length += ln; 
    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	#ifdef RUN_AS_WIN32SERVICE
	if ( ( nWrite = send( desc, txt + iStart, nBlock, 0 ) ) < 0 )
	#else
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	#endif
	    { free(buf2); perror( "Write_to_descriptor" ); return FALSE; }
    } 

    free(buf2);
    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    CHAR_DATA *ch;
    NOTE_DATA *pnote;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    
    char      *pwdnew;
    char      *p;
    char       buf [ MAX_STRING_LENGTH ];
    int        iClass;
    int        notes;
    bool       fOld;
    int        fstr = 0;
    int        fint = 0;
    int        fwis = 0;
    int        fdex = 0;
    int        fcon = 0;
    int        stor_cost = 0;

    while ( isspace( *argument ) )
	argument++;

    /* This is here so we wont get warnings.  ch = NULL anyways - Kahn */
    ch          = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_GET_ANSI:
        if ( argument[0] == '\0' )
        {
	   close_socket( d );
           return;
        }
        if (!str_prefix( argument, "yes" ) )
        {
	   d->ansi = TRUE;
           write_to_buffer( d, "ANSI set.\n\r", 0 );
        }
        else if (!str_prefix( argument, "no" ) )
           write_to_buffer( d, "ANSI not set.\n\r", 0 );
	d->connected = CON_GET_NAME;
	{
	  extern char *help_greeting_one;
	  extern char *help_greeting_two;
	  extern char *help_greeting_three;
	  extern char *help_greeting_four;
	  extern char *help_greeting_five;
	  int rndnumber = number_range(1, 5);

/*	  if ( help_greeting[0] == '.' )
	    write_to_buffer( d, help_greeting + 1, 0 );
	  else
	    write_to_buffer( d, help_greeting, 0 ); 
*/
	    if( rndnumber == 1 )
            {
                write_to_buffer( d, help_greeting_one, 0 );
	    }
	    if( rndnumber == 2 )
	    {
                write_to_buffer( d, help_greeting_two, 0 );
	    }
	    if( rndnumber == 3 )
	    {
                write_to_buffer( d, help_greeting_three, 0 );
            }
	    if( rndnumber == 4 )
	    {
		write_to_buffer( d, help_greeting_four, 0 );
	    }
	    if( rndnumber == 5 )
	    {
		write_to_buffer( d, help_greeting_five, 0 );
	    }
	}
        break;


    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

	argument[0] = UPPER( argument[0] );
        log_string( argument, CHANNEL_NONE , -1 );
	fOld = load_char_obj( d, argument );
	ch   = d->character;

	if ( !check_parse_name( argument ) )
	{
	    if ( !fOld )
	    {
	        write_to_buffer( d,
				"Illegal name, try another.\n\rName: ", 0 );
		if ( d->character )
		  free_ch( d->character );
		d->character = NULL;
		return;
	    }
	    else
	    {
                /*
                 * Trap the "." and ".." logins.
                 * Chars must be > level 1 here
                 */
                if ( ch->level < 1 )
                {
                    write_to_buffer( d,
                                    "Illegal name, try another.\n\rName: ",
                                    0 );
                    return;
                }
		
		sprintf( buf, "Illegal name:  %s", argument );
	        bug( buf, 0 );
	    }
	}

	if ( IS_SET( ch->act, PLR_DENY ) )
	{
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf, CHANNEL_GOD , -1 );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    /* Must be immortal with wizbit in order to get in */
	    if ( wizlock && !IS_CHAMP( ch ) && !ch->wizbit )
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	    if ( ch->level <= numlock && !ch->wizbit && numlock != 0 )
	    {
		write_to_buffer( d,
			"The game is locked to your level character.\n\r\n\r",
				0 );
		if ( ch->level == 0 )
		{
		    write_to_buffer( d,
			"New characters are now temporarily in email ",
				    0 );
		    write_to_buffer( d, "registration mode.\n\r\n\r", 0 );
		    write_to_buffer( d,
			"Please email <implementor addr here> to ", 0 );
		    write_to_buffer( d, "register your character.\n\r\n\r",
				    0 );
		    write_to_buffer( d,
			"One email address per character please.\n\r", 0 );
		    write_to_buffer( d, "Thank you, EnvyMud Staff.\n\r\n\r",
				    0 );
		}
		close_socket( d ) ;
		return;
	    }
	}

        if ( ch->level < 1 ) {
	   fOld = FALSE;
	}

	if ( fOld )
	{
	    /* Old player */
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    CHAR_DATA* nch;
	    bool found = FALSE;
	    for (nch=char_list; nch; nch = nch->next)
	    {
		if (!IS_NPC(nch) && !str_cmp( nch->name, argument ) && nch != d->character && !ch->deleted)
		{
			found=TRUE;
			break;
		}
  	    }

	    if (!found)
	    {
		/* New player */
		sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
		write_to_buffer( d, buf, 0 );
		d->connected = CON_CONFIRM_NEW_NAME;
		return;
	    } else
	    {
		sprintf( buf, "%s is already in the game.\n\r", argument);
		write_to_descriptor( d->descriptor, buf, 0, d);
		close_socket( d ); 
		ch->deleted = TRUE;
		return;
	    }
	}
	break;

    case CON_GET_OLD_PASSWORD:
#if defined( unix )
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );

        if ( check_playing( d, ch->name ) )
	    return;

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	if ( IS_IMMORTAL( ch ) )
	    do_help( ch, "imotd" );
	do_help( ch, "motd" );

        ch->pkill_timer = 0;
	d->connected = CON_READ_MOTD;
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	{

  		if (!str_cmp(d->host, "mudconnector.com") || !str_cmp(d->host, "mudconnect.com"))
  		{
		write_to_descriptor(d->descriptor, "We apologize, but anonymous connections have been disabled on this mud\n\rfor new players.\n\rPlease use a direct connection from software like ZMud or a telnet program\n\rlike [Secure]CRT or PuTTy.\n\rn\rWe also provide a Java telnet client on our website. Please visit\n\rhtp://www.stormgate.ca/ for more details.\n\r", 0, d);
 		close_socket( d );
      		return;
 	 	}


	    sprintf( buf, "New character.\n\rGive me a password for %s: %s",
		    ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;
	}
	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_ch( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined( unix )
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen( argument ) < 5 )
	{
	    write_to_buffer( d,
	       "Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd	= str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined( unix )
        write_to_buffer( d, "\n\r", 2 );
#endif

        if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
        {
            write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
                0 );
            d->connected = CON_GET_NEW_PASSWORD;
            return;
        }

        write_to_buffer( d, echo_on_str, 0 );
        write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
        d->connected = CON_DISPLAY_RACE;
        break;

    case CON_DISPLAY_RACE:
        strcpy( buf, "Select a race\n\r\n\r[" );
        for ( iRace = 0; iRace < MAX_RACE; iRace++ )
        {
            if ( !IS_SET( race_table[ iRace ].race_abilities, RACE_PC_AVAIL ) )
                continue;
            if ( iRace > 0 )
                strcat( buf, "]\n\r[" );
            strcat( buf, race_table[iRace].race_full );
        }
        strcat( buf, "]\n\r\n\rPlease chose: " );
        write_to_buffer( d, buf, 0 );
        d->connected = CON_GET_NEW_RACE;
        break;

    case CON_GET_NEW_RACE:
        for ( iRace = 0; iRace < MAX_RACE; iRace++ )
            if( ( !str_cmp( argument, race_table[iRace].race_full ) ||
		  !str_cmp( argument, race_table[iRace].race_name ) )
                && IS_SET( race_table[ iRace ].race_abilities,
                          RACE_PC_AVAIL ) )
            {
                ch->race = race_lookup( race_table[iRace].race_full );
                break;
            }

        if ( iRace == MAX_RACE )
        {
            write_to_buffer( d,
                            "That is not a race.\n\rWhat IS your race? ", 0 );
            return;
        }

        write_to_buffer( d, "\n\r", 0 );
        do_help( ch, race_table[ch->race].race_full );
        write_to_buffer( d, "Are you sure you want this race? [N]: ", 0 );
        d->connected = CON_CONFIRM_RACE;
        break;

    case CON_CONFIRM_RACE:
        switch ( argument[0] )
        {
          case 'y': case 'Y': break;
          default:
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
              d->connected = CON_DISPLAY_RACE;
              return;
        }

        write_to_buffer( d, "\n\rWhat is your sex (M/F/N)? ", 0 );
        d->connected = CON_GET_NEW_SEX;
        break;
 
    case CON_GET_NEW_SEX:
        switch ( argument[0] )
        {
        case 'm': case 'M': ch->sex = SEX_MALE;    break;
        case 'f': case 'F': ch->sex = SEX_FEMALE;  break;
        case 'n': case 'N': ch->sex = SEX_NEUTRAL; break;
        default:
            write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
            return;
        }

        d->connected = CON_DISPLAY_CLASS;
        write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
        break;

    case CON_DISPLAY_CLASS:
        strcpy( buf, "Select a class:\n\r\n\r[" );
        for ( iClass = 0; iClass < (MAX_CLASS -1); iClass++ )
        {
            if ( iClass > 0 )
                strcat( buf, "]\n\r[" );
            strcat( buf, class_table[iClass].who_long );
        }
        strcat( buf, "]\n\r\n\rPlease chose: " );
        write_to_buffer( d, buf, 0 );
        d->connected = CON_GET_NEW_CLASS;
        break;

    case CON_GET_NEW_CLASS:
        for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        {
            if ( !str_prefix( argument, class_table[iClass].who_long ) ||
		 !str_prefix( argument, class_table[iClass].who_name ) )
            {
                ch->class = iClass;
                break;
            }
        }

        if ( iClass == MAX_CLASS )
        {
            write_to_buffer( d,
                "That's not a class.\n\rWhat IS your class? ", 0 );
            return;
        }

        do_help( ch, (char *)class_table[iClass].who_long );
        write_to_buffer( d, "Is this the class you desire? [N]: ", 0 );
        d->connected = CON_CONFIRM_CLASS;
        break;

    case CON_CONFIRM_CLASS:
        switch ( argument[0] )
        {
          case 'y':
          case 'Y':
	      write_to_buffer( d, "\n\r\n\r", 0);
	      do_help( ch, "startingmulticlass" );
              write_to_buffer( d, "\n\rDo you want to multiclass? [N]: ", 0 );
              d->connected = CON_CHOICE_MULTICLASS;
              break;
          default:
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
              d->connected = CON_DISPLAY_CLASS;
              return;
        }
        break;

    case CON_CHOICE_MULTICLASS:
        switch ( argument[0] )
        {
          case 'y':
          case 'Y':
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
              d->connected = CON_DISPLAY_MULTICLASS;
              break;
          default:
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
	      ch->multied = ch->class;
              d->connected = CON_DISPLAY_RELIGION;
              return;
        }
        break;


    case CON_DISPLAY_MULTICLASS:
        strcpy( buf, "Select a class:\n\r\n\r[" );
        for ( iClass = 0; iClass < (MAX_CLASS-1); iClass++ )
        {
            if ( iClass > 0 )
                strcat( buf, "]\n\r[" );
            strcat( buf, class_table[iClass].who_long );
        }
        strcat( buf, "]\n\r\n\rPlease chose: " );
        write_to_buffer( d, buf, 0 );
        d->connected = CON_GET_MULTICLASS;
        break;

    case CON_GET_MULTICLASS:
        for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        {
            if ( !str_prefix( argument, class_table[iClass].who_name ) ||
		 !str_prefix( argument, class_table[iClass].who_long ) )
            {
                ch->multied = iClass;
                break;
            }
        }

        if ( iClass == MAX_CLASS )
        {
            write_to_buffer( d,
                "That's not a class.\n\rWhat IS your multiclass? ", 0 );
            return;
        }

	if ( iClass == ch->class )
        {
	   write_to_buffer( d,
		"You are already that class.\n\rWhat IS your multiclass? ", 0 );
	   return;
        }

        do_help( ch, (char *)class_table[iClass].who_long );
        write_to_buffer( d, "Is this the class you desire? [N]: ", 0 );
        d->connected = CON_CONFIRM_MULTICLASS;
        break;

    case CON_CONFIRM_MULTICLASS:
        switch ( argument[0] )
        {
          case 'y':
          case 'Y':
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
              d->connected = CON_DISPLAY_RELIGION;
              break;
          default:
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
              d->connected = CON_DISPLAY_MULTICLASS;
              return;
        }
        break;


    case CON_DISPLAY_RELIGION:
        write_to_buffer( d, "Select a Religion:\n\r\n\r", 0 );
	write_to_buffer( d, "A. The Order of Truth, Honour and Chivalry\n\r", 0 );
	write_to_buffer( d, "B. The Order of Law, Order and Justice\n\r", 0 );
	write_to_buffer( d, "C. The Order of Life and Nature\n\r", 0 );
	write_to_buffer( d, "D. The Order of Lies, Manipulation and Deceit\n\r", 0 );
	write_to_buffer( d, "E. The Order of Chaos, Strife and Injustice\n\r", 0 );
	write_to_buffer( d, "F. The Order of Death and Decay\n\r", 0 );
	write_to_buffer( d, "G. The Order of Time and Fate\n\r\r", 0 );
        write_to_buffer( d, "\n\rPlease chose (A-G): ", 0 );
        d->connected = CON_CHOSE_RELIGION;
        break;

    case CON_CHOSE_RELIGION:
        switch ( argument[0] )  
        {
	case 'A': case 'a': 
		  ch->religion = 1;
		  break;
	case 'B': case 'b': 
		  ch->religion = 2;
		  break;
	case 'C': case 'c':
		  ch->religion = 3;
		  break;
	case 'D': case 'd':
		  ch->religion = 4;
		  break;
	case 'E': case 'e':
		  ch->religion = 5;
		  break;
	case 'F': case 'f':
		  ch->religion = 6;
		  break;
	case 'G': case 'g':
		  ch->religion = 7;
		  break;
        default:
            write_to_buffer( d, "Thats not a religion.  What IS your religion? ", 0 );
            return;
        }
        
	write_to_buffer( d, "\n\r", 0 );
	write_to_buffer( d, get_religion_index( ch->religion )->shortdesc, 0 );
	write_to_buffer( d, "\n\r", 0 );
	write_to_buffer( d, get_religion_index( ch->religion )->description, 0 );
        write_to_buffer( d, "\n\r\n\rIs this the religion you desire? [N]: ", 0 );
	d->connected = CON_CONFIRM_RELIGION;
        break;

   case CON_CONFIRM_RELIGION:
        switch ( argument[0] )
        {
          case 'y':
          case 'Y':
	  {
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
              d->connected = CON_DISPLAY_ATTRIBUTES;
              break;
	  }
          default:
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
              d->connected = CON_DISPLAY_RELIGION;
              return;
        }
        break;

    case CON_DISPLAY_ATTRIBUTES:
	fstr = 16 + race_table[ch->race].str_mod;
	fint = 16 + race_table[ch->race].int_mod;
	fwis = 16 + race_table[ch->race].wis_mod;
	fdex = 16 + race_table[ch->race].dex_mod;
	fcon = 16 + race_table[ch->race].con_mod;

        if ( ch->sex == SEX_MALE )
        {
	    fstr += 1;
        }
        if ( ch->sex == SEX_FEMALE )
	{
	    fdex += 1;
        }

	write_to_buffer( d, "              |A   |B   |C   |D\n\r", 0 );
	write_to_buffer( d, "--------------+----+----+----+----\n\r", 0 );
        sprintf( buf, "Strength      |%d  |%d  |%d  |%d \n\r", fstr, fstr - 2, fstr + 3, fstr -2 );
        write_to_buffer( d, buf, 0 );
        sprintf( buf, "Intelligence  |%d  |%d  |%d  |%d \n\r", fint, fint + 3, fint - 2, fint );
        write_to_buffer( d, buf, 0 );
        sprintf( buf, "Wisdom        |%d  |%d  |%d  |%d \n\r", fwis, fwis + 3, fwis - 2, fwis -2 );
        write_to_buffer( d, buf, 0 );
        sprintf( buf, "Dexterity     |%d  |%d  |%d  |%d \n\r", fdex, fdex - 2, fdex - 2, fdex + 4 );
        write_to_buffer( d, buf, 0 );
        sprintf( buf, "Constitution  |%d  |%d  |%d  |%d \n\r", fcon, fcon - 2, fcon + 3, fcon );
        write_to_buffer( d, buf, 0 );
        write_to_buffer( d, "\n\rPlease chose (A-D): ", 0 );
        d->connected = CON_CHOSE_ATTRIBUTES;
        break;    

    case CON_CHOSE_ATTRIBUTES:
        switch ( argument[0] )
        {
        case 'A': case 'a':
                ch->pcdata->perm_str = 16 + race_table[ch->race].str_mod;
                ch->pcdata->perm_int = 16 + race_table[ch->race].int_mod;
                ch->pcdata->perm_wis = 16 + race_table[ch->race].wis_mod;
                ch->pcdata->perm_dex = 16 + race_table[ch->race].dex_mod;
                ch->pcdata->perm_con = 16 + race_table[ch->race].con_mod;
		break;
	case 'B': case 'b':
                ch->pcdata->perm_str = 16 + race_table[ch->race].str_mod - 2;
                ch->pcdata->perm_int = 16 + race_table[ch->race].int_mod + 3;
                ch->pcdata->perm_wis = 16 + race_table[ch->race].wis_mod + 3;
                ch->pcdata->perm_dex = 16 + race_table[ch->race].dex_mod - 2;
                ch->pcdata->perm_con = 16 + race_table[ch->race].con_mod - 2;
		break;
	case 'C': case 'c':
                ch->pcdata->perm_str = 16 + race_table[ch->race].str_mod + 3;
                ch->pcdata->perm_int = 16 + race_table[ch->race].int_mod - 2;
                ch->pcdata->perm_wis = 16 + race_table[ch->race].wis_mod - 2;
                ch->pcdata->perm_dex = 16 + race_table[ch->race].dex_mod - 2;
                ch->pcdata->perm_con = 16 + race_table[ch->race].con_mod + 3;
		break;
	case 'D': case 'd':
                ch->pcdata->perm_str = 16 + race_table[ch->race].str_mod - 2;
                ch->pcdata->perm_int = 16 + race_table[ch->race].int_mod + 0;
                ch->pcdata->perm_wis = 16 + race_table[ch->race].wis_mod - 2;
                ch->pcdata->perm_dex = 16 + race_table[ch->race].dex_mod + 4;
                ch->pcdata->perm_con = 16 + race_table[ch->race].con_mod + 0;
                break;

        default:
            write_to_buffer( d, "Thats not valid choice  What IS your choice? ", 0 );
            return;
        }
        
        write_to_buffer( d, "\n\rAre these the attributes you desire? [N]: ", 0 );
        d->connected = CON_CONFIRM_ATTRIBUTES;
        break;

   case CON_CONFIRM_ATTRIBUTES:
        switch ( argument[0] )
        {
          case 'y':
          case 'Y':
	      if ( ch->sex == SEX_MALE )
	      {
		   ch->pcdata->perm_str += 1;
	      }
	      if ( ch->sex == SEX_FEMALE )
	      {
		   ch->pcdata->perm_dex += 1;
	      }
	      write_to_buffer( d, "\n\r\n\r", 0);
	      do_help( ch, "startingpk" );
              write_to_buffer( d, "\n\rDo you want to PKill? [Y]: ", 0 );
              d->connected = CON_GET_PKILL;
              break;
          default:
              write_to_buffer( d, "\n\rPress Return to continue:\n\r", 0 );
              d->connected = CON_DISPLAY_ATTRIBUTES;
              return;
        }
        break;
    

     case CON_GET_PKILL:
        switch (argument[0]) {
            case 'n':
            case 'N':
              write_to_buffer(d, "\n\rAre you sure you want to be peaceful? [N]: ", 0 );
              ch->pkill = FALSE;
              d->connected = CON_CONFIRM_PKILL;
              break;
            default:
	      if( ch->class == ch->multied )
	      {
		write_to_buffer(d, "\n\rYou can choose peaceful, and later remort to a pkill character.", 0 );
	      }
              write_to_buffer(d, "\n\rAre you sure you want to PKill? [N]: ", 0 );
              ch->pkill = TRUE;
              d->connected = CON_CONFIRM_PKILL;
        }
        break;
     case CON_CONFIRM_PKILL:
        switch ( argument[0] )
        {
          case 'y':
          case 'Y':
	      break;
	  default:
              write_to_buffer( d, "\n\rDo you want to PKill? [Y]: ", 0 );
              d->connected = CON_GET_PKILL;
              return;
	}

	sprintf( log_buf, "%s!%s@%s new player.", ch->name, d->user, d->host );
	log_string( log_buf, CHANNEL_GOD, -1 );
	sprintf( log_buf, "%s is a new player to the Mud!", ch->name );
	log_string( log_buf, CHANNEL_INFO, -1 );
	ch->pcdata->pagelen = 60;
	ch->pcdata->clean = TRUE;
	do_help( ch, "motd" );
	ch->pcdata->pagelen = 20;

	d->connected = CON_READ_MOTD;
	break;

    case CON_AUTHORIZE_NAME:
    case CON_AUTHORIZE_NAME1:
    case CON_AUTHORIZE_NAME2:
    case CON_AUTHORIZE_NAME3:
      write_to_buffer(d, "Please wait for an immortal to authorize you.\n\r", 0);
      sprintf(log_buf, "%s!%s@%s needs be to AUTHORIZED.", ch->name, d->user, d->host);
      log_string(log_buf, CHANNEL_LOG, -1 );
      d->connected++;
      if(d->connected == CON_AUTHORIZE_LOGOUT)
      {
        write_to_buffer( d, "Auto exit to prevent spam.\n\r", 0 );
        sprintf(log_buf, "%s!%s@%s auto logged off.", ch->name, d->user, d->host);
        log_string(log_buf, CHANNEL_LOG, -1 );
	close_socket( d );
	return;
      }
    break;

/* END */
    case CON_READ_MOTD:
	{
	
	/* Reload char after motd to avoid
	 * the duping bug
	 */
	if (ch->level > 1)
	{
		char* char_name = str_dup(ch->name);
		free_ch( ch );
		load_char_obj( d, char_name );
		free_string(char_name);
	}

	ch->next	= char_list;
	char_list	= ch;
	d->connected	= CON_PLAYING;

#ifdef SQL_SYSTEM
	if (!ch->playing)
		link_char(ch);
#endif

	send_to_char(AT_GREEN, "\n\rWelcome to the Realm.  May your visit here be adventurous.", ch );
	if( !IS_NPC ( ch ) && IS_SET( ch->act, PLR_SOUND ) )
	{
		send_to_char( AT_GREEN, "!!SOUND(login1.wav V=100 L=1 P=55 T=Logon)", ch );
	}
	send_to_char(AT_GREEN, "\n\r", ch );
	userl_update( ch->desc, TRUE );  

	if ( ch->level == 0 )
	{
	    OBJ_DATA *obj;
	    RELIGION_DATA *pReligion;
	    pReligion = get_religion_index( ch->religion );
	    pReligion->members++;
	    ch->level	= 1;
	    ch->exp	= 1000;
	    ch->updated = CURRENT_PLAYER_UPDATE;
	    ch->hit	= ch->max_hit;
	    ch->mana	= ch->max_mana;
	    ch->bp      = ch->max_bp;
	    ch->move	= ch->max_move;
	    ch->pcdata->awins        = 0; /* arena wins           */
	    ch->pcdata->alosses      = 0; /* arena losses         */
	    ch->pcdata->mobkills     = 0; /* number of mobs killed */

	    SET_BIT (ch->act, PLR_AUTOLOOT);
	    SET_BIT (ch->act, PLR_AUTOEXIT);
	    SET_BIT (ch->act, PLR_AUTOGOLD);
	    SET_BIT (ch->act, PLR_AUTOSAC);

#if defined (AUTOLEARN)                         /* Set default language */
            ch->speaking = race_table[ch->race].race_lang;
            ch->language[race_table[ch->race].race_lang] = 100;
	    ch->language[COMMON] = 100;
            ch->pcdata->learn = 5;
#endif

	    sprintf( buf, "the %s",
		    title_table [ch->class] [ch->level]
		    [ch->sex == SEX_FEMALE ? 1 : 0] );
	    set_title( ch, buf );
            ch->pcdata->who_text = str_dup( "@" );
	    if (( ch->class != 9 )&&( ch->class != 11))
	       ch->prompt = str_dup( "<&Y%hhp &C%mm &G%vmv&w> " );
	    else
	       ch->prompt = str_dup( "<&Y%hhp &R%mbp &G%vmv&w> " ); 
	    obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_BANNER ), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_LIGHT );

	    obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_VEST   ), 0 );
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_BODY );

	    if( ch->race != 13 && ch->class != CLASS_BARBARIAN && ch->multied != CLASS_BARBARIAN )
	    {
	    obj = create_object(get_obj_index( OBJ_VNUM_SCHOOL_DAGGER ),0);
	    obj_to_char( obj, ch );
	    equip_char( ch, obj, WEAR_WIELD );
	    }

            if (!get_religion_index(ch->religion)->start)
	    	char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
	    else
	    	char_to_room( ch, get_room_index( get_religion_index( ch->religion )->start ) );

	}
	else if ( ch->in_room )
	{
	    char_to_room( ch, ch->in_room );
	}
	else if ( IS_IMMORTAL( ch ) )
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
	}
	else
	{
	    char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	}

	if ( ch->updated != CURRENT_PLAYER_UPDATE )
	{
	/* This is where the update character crap will go before login */

	    OBJ_DATA* obj;

	    send_to_char(AT_RED, "Please wait while your pfile is updated.\n\r\n\r", ch );

	    if( ch->updated == 0 )
	    {
		if(ch->pcdata->bankaccount > 50000000 )
		{
		    ch->pcdata->bankaccount = 50000000;
		    send_to_char(AT_RED, "Completed update to 1.01.\n\r", ch );
		}
		ch->updated = 101;
	    }
	    if( ch->updated == 101 )
	    {
		ch->nextquest = 0;
		ch->countdown = 0;
		ch->questobj = 0;
		ch->questmob = 0;
		send_to_char(AT_RED, "Completed update to 1.02.\n\r", ch );
		ch->updated = 102;
	    }
            if( ch->updated == 102 )
            {
		int cnt;
                ch->rquestpoints = 0;
                ch->rnextquest = 0;
                ch->rcountdown = 0;
                for (cnt=0;cnt<MAX_LEVEL;cnt++)
		{
			ch->rquestobj[cnt] = 0;
                	ch->rquestmob[cnt] = 0;
		}
                send_to_char(AT_RED, "Completed update to 1.03.\n\r", ch );
                ch->updated = 103;
            }
	    if( ch->updated == 103 )
	    {
		ch->clan = 0;
		ch->clev = 0;
		send_to_char(AT_RED, "Completed update to 1.04.\n\r", ch );
		ch->updated = 104;
	    }
	    if( ch->updated == 104 || ch->updated == 105 || ch->updated == 106)
	    {
		RELIGION_DATA *pReligion;

		pReligion = get_religion_index( ch->religion );
		pReligion->members++;
		send_to_char(AT_RED, "Completed update to 1.07.\n\r", ch );
		ch->updated = 107;
	    }
            if (ch->updated == 107)
	    { 
		DeadObjPrntOnly = FALSE;
		clean_player_objects(ch); ch->updated = 108; 
		DeadObjPrntOnly = TRUE;
		ch->pcdata->switched = FALSE;
		send_to_char(AT_RED, "Completed update to 1.08.\n\r", ch );
	    }
	    if( ch->updated == 108 )
	    {
		ch->position = POS_STANDING;
		send_to_char(AT_RED, "Completed update to 1.09.\n\r", ch );

		ch->updated = 109;
	    }
            if( ch->updated == 109 )
	    {
		if( ch->level >= 50 )
		{
		    SET_BIT    ( ch->act, PLR_GHOST );
		}
		send_to_char(AT_RED, "Completed update to 1.10.\n\r", ch );
		ch->updated = 110;
	    }
	    if ( ch->updated == 110)
	    {
		do_astrip(ch, "self");
		send_to_char(AT_RED, "Completed update to 1.11.\n\r", ch );
		ch->updated = 111;
	    }
	    if ( ch->updated == 111 || ch->updated == 112 )
	    {
		ch->clan = 0;
		ch->clev = 0;
		send_to_char(AT_RED, "Completed update to 1.13.\n\r", ch );
		ch->updated = 113;
            }
            if ( ch->updated == 113 || ch->updated == 114 )
            {
                for( paf = ch->affected; paf; paf = paf_next )
                {
                    paf_next = paf->next;
                    affect_remove( ch, paf );
                }
                for( paf = ch->affected2; paf; paf = paf_next )
                {
                    paf_next = paf->next;
                    affect_remove2( ch, paf );
                }
                for( paf = ch->affected3; paf; paf = paf_next )
                {
                    paf_next = paf->next;
                    affect_remove3( ch, paf );
                }
                for( paf = ch->affected4; paf; paf = paf_next )
                {
                    paf_next = paf->next;
                    affect_remove4( ch, paf );
                }
                for( paf = ch->affected_powers; paf; paf = paf_next )
                {
                    paf_next = paf->next;
                    affect_remove_powers( ch, paf );
                }
                for( paf = ch->affected_weaknesses; paf; paf = paf_next )
                {
                    paf_next = paf->next;
                    affect_remove_weaknesses( ch, paf );
                }
    
                ch->affected_by = 0;
                ch->affected_by2 &= CODER;
                ch->affected_by3 = 0;
                ch->affected_by4 = 0;
                ch->affected_by_powers = 0;
                ch->affected_by_weaknesses = 0;
                ch->shields = 0;
                ch->updated = 115;
		send_to_char(AT_RED, "\n\rYOU HAVE BEEN STRIPPED OF ALL AFFECTS!\n\r", ch );
		send_to_char(AT_RED, "Completed update to 1.15.\n\r", ch );
	    }
            if ( ch->updated == 115 )
	    {
		DeadObjPrntOnly = FALSE;
		clean_player_objects(ch);
		ch->updated = 116;
		DeadObjPrntOnly = TRUE;
		ch->pcdata->switched = FALSE;
		send_to_char(AT_RED, "Completed update to 1.16.\n\r", ch );
	    }
	    if ( ch->updated == 116 )
	    {
		ch->pcdata->bankaccount /= 1000;
		ch->gold /= 1000;
		ch->updated = 117;
 		send_to_char(AT_RED, "Completed update to 1.17.\n\r", ch );
	    }
	    if ( ch->updated == 117 )
	    {
		ch->pcdata->mobkills = 0;
		ch->updated = 118;
		send_to_char(AT_RED, "Completed update to 1.18\n\r", ch );
	    }
	    if ( ch->updated == 118 )
	    {
		ch->mounted = 0;
		ch->mountshort = "";
		ch->mountcharmed = 0;
		ch->updated = 119;
		send_to_char(AT_RED, "Completed update to 1.19\n\r", ch );
	    }
            if ( ch->updated == 119 )
	    {
		ch->questpoints += 2500;
		send_to_char(AT_RED, "&RCongratulations to all of us!  Stormgate has now been in existance for\n\r\n\r****************\n\r* &WEIGHT &RYEARS! *\n\r****************\n\r\n\rYou have been awarded 2500qp as a thank-you for playing!\n\r", ch );
		ch->updated = 120;
	    }
	    if ( ch->updated == 120 )
	    {
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;
		int count = 1;
		char buf[MAX_STRING_LENGTH];

		for ( obj = ch->pcdata->storage; obj; obj = obj_next, count++ )
		{
		    obj_next = obj->next_content;
			sprintf( buf, "The bank has repossessed %s from your storage.\n\r", obj->short_descr );
			send_to_char( AT_RED, buf, ch );
 			obj_from_storage( obj );
			extract_obj( obj );
		}
		ch->updated = 121;
		send_to_char(AT_RED, "Completed update to 1.21\n\r", ch );
	    }
	    if ( ch->updated == 121 || ch->updated == 122 )
	    {
		for ( obj = ch->carrying; obj; obj = obj->next_content )
		{  
		    if( obj->wear_loc != WEAR_NONE)
			unequip_char( ch, obj);
		}  
      
		for( paf = ch->affected; paf; paf = paf_next )
		{
		    paf_next = paf->next;
		    affect_remove( ch, paf );
		}
		for( paf = ch->affected2; paf; paf = paf_next )
		{
		    paf_next = paf->next;
		    affect_remove2( ch, paf );
		}
		for( paf = ch->affected3; paf; paf = paf_next )
		{  
		    paf_next = paf->next;
		    affect_remove3( ch, paf );
		}
		for( paf = ch->affected4; paf; paf = paf_next )
		{  
 		    paf_next = paf->next;
		    affect_remove4( ch, paf );
		}
		for( paf = ch->affected_powers; paf; paf = paf_next )
		{
		    paf_next = paf->next;
		    affect_remove_powers( ch, paf );
		}
		for( paf = ch->affected_weaknesses; paf; paf = paf_next )
		{
		    paf_next = paf->next;
		    affect_remove_weaknesses( ch, paf );
		}
                
		ch->affected_by = 0;
		ch->affected_by2 &= CODER;
		ch->affected_by3 = 0;
		ch->affected_by4 = 0;
		ch->affected_by_powers = 0;
		ch->affected_by_weaknesses = 0;
		ch->shields = 0;
		ch->armor = 0;

		ch->updated = 123;

		send_to_char(AT_RED, "\n\rALL OF YOUR EQUIPMENT HAS BEEN REMOVED.\n\rALL OF YOUR SPELL EFFECTS HAVE BEEN STRIPPED.\n\r\n\r", ch );
		send_to_char(AT_RED, "Completed update to 1.23\n\r", ch );
	    }
	    if( ch->updated == 123 )
	    {
		if( ch->race == 13 )
		{
		    ch->pcdata->perm_str -= 4;
		    ch->pcdata->perm_int -= 7;
		    ch->pcdata->perm_wis -= 2;
		    ch->pcdata->perm_dex += 2;
		    ch->pcdata->perm_con -= 5;
		    ch->race = 20;
		}
	   	ch->updated = 124;
		send_to_char(AT_RED, "Completed update to 1.24.\n\r", ch );
	    }
	    if( ch->updated == 124 )
	    {
		ch->clev = 0;
		ch->clan = 0;
		send_to_char(AT_RED, "Completed update to 1.25.\n\r", ch );
		ch->updated = 125;
	    }
            if( ch->updated == 125 )
            {
               /* Set race language to 100% due to bug - Ahsile*/
               ch->language[race_table[ch->race].race_lang] = 100;
               send_to_char(AT_RED, "Completed update to 1.26.\n\r",ch );
               ch->updated = 126;
            }
	    if( ch->updated == 126 )
	    {
		/* Set current poison level to zero and strip poison effects */
		affect_strip( ch, skill_lookup("poison") );
		ch->poison_level = 0;
		send_to_char(AT_RED, "Completed update to 1.27.\n\r", ch );
		ch->updated = 127;
	    }
	    if ( ch->updated == 127 )
	    {
		ch->clev = 0;
		ch->clan = 0;
		send_to_char(AT_RED, "Completed update to 1.28.\n\r", ch );
		ch->updated = 128;
	    }
	    if( ch->updated == 128 )
	    {	
		int sn;
		for ( sn = 0; skill_table[sn].name[0] != '\0'; sn++ )
		{
		    if ( skill_table[sn].name )
		    {
			if ( skill_table[sn].skill_level[ch->class] <= LEVEL_DEMIGOD ||
			  skill_table[sn].skill_level[ch->multied] <= LEVEL_DEMIGOD )
			    ch->pcdata->learned[sn] = 0;
		    else
			if ( skill_table[sn].skill_level[ch->class] <= get_trust( ch ) ||
			  skill_table[sn].skill_level[ch->multied] <= get_trust( ch ) )
			    ch->pcdata->learned[sn] = 0;
		    }
		}
		ch->questpoints += 150;
		ch->practice += 250;
		char_from_room( ch );
		char_to_room( ch, get_room_index( get_religion_index(ch->religion)->recall ) );
		send_to_char(AT_RED, "Completed update to 1.29.\n\rPlease see the forums at www.stormgate.ca, under Announcements, for the update to 1.29 for an explanation.\n\r", ch );
		ch->updated = 129;
	    }
	    if (ch->updated == 129)
	    {
		if (ch->level >= 50)
		{
			SET_BIT( ch->act, PLR_GHOST );
		}
		send_to_char(AT_RED, "Completed update to 1.30\n\r", ch );
		ch->updated = 130;
	    }
	    send_to_char(AT_RED, "\n\rCompleted fully.  See HELP PLAYERUPDATE for more information.\n\r\n\r", ch );
	    ch->updated = CURRENT_PLAYER_UPDATE;
	}
	if ( !IS_SET( ch->act, PLR_WIZINVIS )
	    && !IS_AFFECTED( ch, AFF_INVISIBLE ) && !IS_AFFECTED2( ch, AFF_IMPROVED_INVIS ) )
	{
	    act(AT_GREEN, "$n has returned to the game.", ch, NULL, NULL, TO_ROOM );

	    sprintf( log_buf, "%s has returned to the game.", ch->name );
	    broadcast_channel( ch, log_buf, CHANNEL_INFO, "!!SOUND(holy2.wav V=100 L=1 P=50 T=Logon)");
	}


	if ( !IS_CODER( ch ) )
	{
    		sprintf(log_buf,"%s!%s@%s has connected in room vnum %d.", ch->name, d->user, d->host, ch->in_room->vnum );
    		log_string(log_buf, CHANNEL_LOG, ch->level - 1 );
	}

        if ( ch->pcdata->corpses >= 2 )
        {
        char strfng [ MAX_INPUT_LENGTH ];
        ch->pcdata->corpses = 0;
#if !defined( macintosh ) && !defined( MSDOS ) 
    sprintf( strfng, "%s%s%s%s.cps", PLAYER_DIR, 
	initial( ch->name ), "/", capitalize( ch->name ) );
#else
   sprintf( strfng, "%s%s.cps", PLAYER_DIR, capitalize( ch->name );
#endif
        if ( remove( strfng ) != 0 )
        perror( strfng );
        }

        // initial inventory count - Ahsile
        ch->carry_number = ch_invcount( ch );

	if ( !IS_NPC( ch ) && ch->pcdata->storage )
	{
	  OBJ_DATA *obj;
	  OBJ_DATA *obj_next;
	  int count = 1;
	  char buf[MAX_STRING_LENGTH];

	  for ( obj = ch->pcdata->storage; obj; obj = obj_next, count++ )
	  {
	    obj_next = obj->next_content;
	    if ( ch->pcdata->bankaccount < count * 100 )
	    {
	      sprintf( buf,
		      "The bank has repossessed %s from your storage.\n\r",
		      obj->short_descr );
	      send_to_char( AT_RED, buf, ch );
	      obj_from_storage( obj );
	      extract_obj( obj );
	    }
	  }
          #if defined BANK_INVEST
             stor_cost = ( ( 100 + share_value) * ch->pcdata->storcount )/10;
          #else
             stor_cost = 25 * ch->pcdata->storcount;
          #endif
 
	  sprintf( buf,
    "The bank deducts %dgp from your account for the storage of %d items.\n\r",
		  stor_cost, ch->pcdata->storcount );
	  send_to_char( AT_PINK, buf, ch );
	  ch->pcdata->bankaccount -= stor_cost;;
	}

	if (ch->fixed_error)
		send_to_char(AT_RED, "\n\r*** Corrupted items have been found on your character.\n\r*** They have been removed to prevent a crash.\n\r\n\r", ch);

	do_look( ch, "auto" );
	/* check for new notes */
	notes = 0;

	for ( pnote = note_list; pnote; pnote = pnote->next )
	    if ( is_note_to( ch, pnote ) && str_cmp( ch->name, pnote->sender )
		&& pnote->date_stamp > ch->last_note && pnote->on_board == 0 )
	        notes++;
	if ( notes == 1 )
	{
	    send_to_char(AT_WHITE, "\n\rYou have one new note waiting.", ch );
	}
	else
	{
	    if ( notes > 1 )
	    {
		sprintf( buf, "\n\rYou have %d new notes waiting.", notes );
		send_to_char(AT_WHITE, buf, ch );
	    }
	}
	if( notes >= 1 )
	{
	    if(!IS_NPC( ch ) && IS_SET( ch->act, PLR_SOUND ) )
	    {
		send_to_char(AT_WHITE, "!!SOUND(newmsg1.wav V=100 L=1 P=100 T=Logon)", ch );
	    }
	    send_to_char(AT_WHITE, "\n\r", ch );
	}
	break;
	}
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    /*
     * Reserved words.	OLC 1.1b
     */
    if ( is_name( name, "all auto immortal self someone none" ) )
	return FALSE;

    /*
     * Length restrictions.
     */
    if ( strlen( name ) <  3 )
	return FALSE;

#if defined( MSDOS )
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined( macintosh ) || defined( unix )
    if ( strlen( name ) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha( *pc ) )
		return FALSE;
	    if ( LOWER( *pc ) != 'i' && LOWER( *pc ) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	extern MOB_INDEX_DATA *mob_index_hash [ MAX_KEY_HASH ];
	       MOB_INDEX_DATA *pMobIndex;
	       int             iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    OBJ_DATA  *obj;
    CHAR_DATA *ch;

    for ( ch = char_list; ch; ch = ch->next )
    {
	if ( !IS_NPC( ch )
	    && ( !fConn || !ch->desc )
	    && !str_cmp( d->character->name, ch->name ) && !ch->deleted )
	{
	    if ( fConn == FALSE )
	    {
			if (d->character != ch )
			{
				free_string( d->character->pcdata->pwd );
				d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
			}
	    }
	    else
	    {
		free_ch( d->character );
		d->character = ch;
		ch->desc     = d;
		ch->timer    = 0;
		send_to_char(AT_GREEN, "Reconnecting.\n\r", ch );
		act(AT_GREEN, "$n has reconnected.", ch, NULL, NULL, TO_ROOM );
		sprintf( log_buf, "%s!%s@%s reconnected in room vnum %d.", ch->name, d->user, d->host, ch->in_room->vnum );
		log_string( log_buf, CHANNEL_LOG, ch->level -1 );
	if ( !IS_SET( ch->act, PLR_WIZINVIS )
	    && !IS_AFFECTED( ch, AFF_INVISIBLE ) && !IS_AFFECTED2( ch, AFF_IMPROVED_INVIS ) )
		{
		sprintf( log_buf, "%s reconnected.", ch->name);
		log_string( log_buf, CHANNEL_INFO, ch->level -1 );
		}
		d->connected = CON_PLAYING;
		{
		  USERL_DATA *ul;
		  
		  for ( ul = user_list; ul; ul = ul->next )
		  {
		    if ( !str_cmp( ch->name, ul->name ) )
		    {
		      sprintf( log_buf, "On Since %s", ctime( &current_time ) );
		      *( log_buf + strlen( log_buf ) - 1 ) = '\0';
		      free_string( ul->lastlogin );
		      ul->lastlogin = str_dup( log_buf );
		      ul->class = ch->class;
                      ul->multi = ch->multied;
		      break;
		    }
		  }
		}
		/*
		 * Contributed by Gene Choi
		 */
		if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) )
		    && obj->item_type == ITEM_LIGHT
		    && obj->value[2] != 0
		    && ch->in_room )
		    ++ch->in_room->light;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	    && dold->character
	    && dold->connected != CON_GET_NAME
	    && dold->connected != CON_GET_OLD_PASSWORD
	    && !str_cmp( name, dold->original
			? dold->original->name : dold->character->name )
	    && !dold->character->deleted )
	{
	if (dold->character->level == 1) {
	    write_to_buffer( d, "Already playing.\n\rName: ", 0 );
	    d->connected = CON_GET_NAME;
	    if ( d->character )
	    {
		free_ch( d->character );
		d->character = NULL;
	    }
	    return TRUE;
	}
            if ( dold->connected == CON_PLAYING )
            {
			  #ifndef SQL_SYSTEM
              free_ch(d->character);
              d->character = (dold->original ? dold->original :
                        dold->character );
			  #else
			  if (d->character != dold->character)
			  {
				  free_ch(d->character);
				  d->character = (dold->original ? dold->original :
                        dold->character );
			  }
			  #endif
              if ( dold->original )
                dold->original->desc = NULL;
              dold->character->desc = NULL;
              d->character->desc = d;
              dold->original = NULL;
              dold->character = NULL;
              write_to_buffer(dold, "Kicking off old link.\n\r", 0);
              close_socket(dold);

              if ( !IS_SET( d->character->act, PLR_WIZINVIS ) )
                 act( AT_GREEN, "A ghostly aura briefly embodies $n.",
                   d->character, NULL, NULL, TO_ROOM );
              send_to_char( AT_GREEN, "You arise from netdeath and continue"
                            " playing.\n\r", d->character );
              sprintf(log_buf, "%s connects, kicking off old link.",
                      d->character->name);  
              log_string(log_buf, CHANNEL_LOG, d->character->level);
	      sprintf(log_buf, "%s has returned to the game.", d->character->name );
	      log_string( log_buf, CHANNEL_INFO, d->character->level - 1);
              d->connected = CON_PLAYING;
              return TRUE;
            }
            write_to_buffer( d, "Already playing.\n\r Name: ", 0 );   
            d->connected = CON_GET_NAME;
            if ( d->character )
            {
                free_ch( d->character );
                d->character = NULL;
            }
            return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if (   !ch
	|| !ch->desc
	||  ch->desc->connected != CON_PLAYING
	|| !ch->was_in_room
	||  ch->in_room != get_room_index( ROOM_VNUM_LIMBO ) )
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act(AT_GREEN, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}

/*
 * Write to all characters.
 */
void send_to_all_char( const char *text )
{
    DESCRIPTOR_DATA *d;

    if ( !text )
        return;
    for ( d = descriptor_list; d; d = d->next )
        if ( d->connected == CON_PLAYING )
	    send_to_char(AT_YELLOW, text, d->character );

    return;
}

/*
 * Write to one char.
 */
void send_to_char(int AType, const char *txt, CHAR_DATA *ch )
{
    if ( !txt || !ch->desc )
        return;

    free_string( ch->desc->showstr_head );
    ch->desc->showstr_head  = str_dup( txt );
    ch->desc->showstr_point = ch->desc->showstr_head;
    set_char_color( AType, ch );
    show_string( ch->desc, "" );

}

void set_char_color( int AType, CHAR_DATA *ch )
{
    char buf[32];

    if ( IS_SET(ch->act,PLR_ANSI) && ch->desc )
    {
	if (AType>15)
	  sprintf( buf,"\033[0;%d;5;%dm",(((AType-16) & 8)==8),30+((AType-16) & 7) );
	else
	  sprintf( buf,"\033[0;%d;%dm",((AType & 8)==8),30+(AType & 7) );
	write_to_buffer( ch->desc, buf, strlen(buf) );
    }
    return;
}

/* OLC, new pager for editing long descriptions. */
/* ========================================================================= */
/* - The heart of the pager.  Thanks to N'Atas-Ha, ThePrincedom for porting  */
/*   this SillyMud code for MERC 2.0 and laying down the groundwork.         */
/* - Thanks to Blackstar, hopper.cs.uiowa.edu 4000 for which the improvements*/
/*   to the pager was modeled from.  - Kahn                                  */
/* - Safer, allows very large pagelen now, and allows to page while switched */
/*   Zavod of jcowan.reslife.okstate.edu 4000.                               */
/* ========================================================================= */

void show_string( DESCRIPTOR_DATA *d, char *input )
{
    char               *start, *end;
    char                arg[MAX_INPUT_LENGTH];
    int                 lines = 0, pagelen;

    if ( !d ) return;

    /* Set the page length */
    /* ------------------- */

    pagelen = d->original ? d->original->pcdata->pagelen
                          : d->character->pcdata->pagelen;

    /* Check for the command entered */
    /* ----------------------------- */

    one_argument( input, arg );

    switch( UPPER( *arg ) )
    {
        /* Show the next page */

        case '\0':
        case 'C': lines = 0;
                  break;
        
        /* Scroll back a page */

        case 'B': lines = -2 * pagelen;
                  break;

        /* Help for show page */

        case 'H': write_to_buffer( d, "B     - Scroll back one page.\n\r", 0 );
                  write_to_buffer( d, "C     - Continue scrolling.\n\r", 0 );
                  write_to_buffer( d, "H     - This help menu.\n\r", 0 );
                  write_to_buffer( d, "R     - Refresh the current page.\n\r",
                                   0 );
                  write_to_buffer( d, "Enter - Continue Scrolling.\n\r", 0 );
                  return;

        /* refresh the current page */

        case 'R': lines = -1 - pagelen;
                  break;

        /* stop viewing */

        default:  free_string( d->showstr_head );
                  d->showstr_head  = NULL;
                  d->showstr_point = NULL;
                  return;
    }

    /* do any backing up necessary to find the starting point */
    /* ------------------------------------------------------ */

    if ( lines < 0 )
    {
        for( start= d->showstr_point; start > d->showstr_head && lines < 0;
             start-- )
            if ( *start == '\r' )
                lines++;
    }
    else
        start = d->showstr_point;

    /* Find the ending point based on the page length */
    /* ---------------------------------------------- */

    lines  = 0;

    for ( end= start; *end && lines < pagelen; end++ )
	{
        if ( *end == '\r' )
            lines++;
	}

    d->showstr_point = end;

    if ( end - start )
        write_to_buffer( d, start, end - start );

    /* See if this is the end (or near the end) of the string */
    /* ------------------------------------------------------ */

    for ( ; isspace( *end ); end++ );

    if ( !*end )
    {
        free_string( d->showstr_head );
        d->showstr_head  = NULL;
        d->showstr_point = NULL;
    }

    return;
}


/*
 * The primary output interface for formatted output.
 */
void act(int AType, const char *format, CHAR_DATA *ch, const void *arg1,
	 const void *arg2, int type )
{
           OBJ_DATA        *obj1        = (OBJ_DATA  *) arg1;
	   OBJ_DATA        *obj2        = (OBJ_DATA  *) arg2;
	   CHAR_DATA       *to;
	   CHAR_DATA       *vch         = (CHAR_DATA *) arg2;
    static char *    const  he_she  [ ] = { "it",  "he",  "she" };
    static char *    const  him_her [ ] = { "it",  "him", "her" };
    static char *    const  his_her [ ] = { "its", "his", "her" };
    const  char            *str;
    const  char            *i;
           char            *point;
           char             buf     [ MAX_STRING_LENGTH ];
           char             buf2    [ MAX_STRING_LENGTH ];
           char             fname   [ MAX_INPUT_LENGTH  ];

    /*
     * Discard null and zero-length messages.
     */
    if ( !format || format[0] == '\0' )
	return;
    if (!ch->in_room)
	{
	ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
	}

    to = ch->in_room->people;
    if ( type == TO_VICT )
    {
	if ( !vch )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}
	to = vch->in_room->people;
    }
    
    for ( ; to; to = to->next_in_room )
    {
	if ( ( to->deleted )
	    || !IS_AWAKE( to ) )
	    continue;

	if ( type == TO_CHAR    && to != ch )
	    continue;
	if ( type == TO_VICT    && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM    && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;
	if ( type == TO_COMBAT &&
	    (to == ch || to == vch || !IS_SET(to->act, PLR_COMBAT ) ) )
	  continue;
	
	point	= buf;
	str	= format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;

	    if ( !arg2 && *str >= 'A' && *str <= 'Z' )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		sprintf( log_buf, "String: %s.", format );
		bug( log_buf, 0 );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
			  i = " <@@@> ";				break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;				break;
		case 'T': i = (char *) arg2;          			break;
		case 'n': i = PERS( ch,  to  );				break;
		case 'N': i = PERS( vch, to  );				break;
		case 'e': i = he_she  [URANGE( 0, ch  ->sex, 2 )];	break;
		case 'E': i = he_she  [URANGE( 0, vch ->sex, 2 )];	break;
		case 'm': i = him_her [URANGE( 0, ch  ->sex, 2 )];	break;
		case 'M': i = him_her [URANGE( 0, vch ->sex, 2 )];	break;
		case 's': i = his_her [URANGE( 0, ch  ->sex, 2 )];	break;
		case 'S': i = his_her [URANGE( 0, vch ->sex, 2 )];	break;

		case 'p':
		    i = can_see_obj( to, obj1 )
			    ? obj1->short_descr
			    : "something";
		    break;

		case 'P':
		    i = can_see_obj( to, obj2 )
			    ? obj2->short_descr
			    : "something";
		    break;

		case 'd':
		    if ( !arg2 || ( (char *) arg2 )[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }
		
	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}

	*point++ = '\n';
	*point++ = '\r';
        *point++ = '\0';

	buf[0]   = UPPER( buf[0] ); 
	if (IS_SET(to->act,PLR_ANSI))
	{
	  if (AType>15)
	    sprintf(buf2,"\033[0;%d;5;%dm",(((AType-16) & 8)==8),30+((AType-16) & 7));
	  else
	    sprintf(buf2,"\033[0;%d;%dm",((AType & 8)==8),30+(AType & 7));
	  strcat(buf2, buf);
	}
	else
	  strcpy(buf2, buf);
	if ( to->desc )
	    write_to_buffer( to->desc, buf2, strlen(buf2) );
	if (MOBtrigger)
	    mprog_act_trigger( buf, to, ch, obj1, vch );
    }
    MOBtrigger = TRUE;
    return;
}



/*
 * Macintosh support functions.
 */
#if defined( macintosh ) || defined( RUN_AS_WIN32SERVICE)
int gettimeofday( struct timeval *tp, struct timezone *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
	return 0;
}
#endif

void do_authorize(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int mode = 0;

  if(argument[0] != '\0')
  {
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
         if(arg2[0] == '\0' || !str_prefix(arg2, "yes"))
      mode = 1;
    else if(!str_prefix(arg2, "no"))
      mode = 2;
    else
      mode = 3;
  }
  else
  {
    send_to_char(AT_PURPLE,"People waiting for authorization:\n\r",ch);
    send_to_char(AT_PURPLE,"---------------------------------\n\r",ch);
  }
  for(d = descriptor_list;d;d = d_next)
  {
    d_next	= d->next;
    if(!d->character) continue;
    if(!d->character->name) continue;
    if(mode == 0)
    {
      if(d->connected >= CON_AUTHORIZE_NAME
        && d->connected <= CON_AUTHORIZE_LOGOUT)
      {
        sprintf(buf, "  %s!%s@%s\n\r", d->character->name, d->user, d->host);
        send_to_char(C_DEFAULT, buf, ch);
      }
    }
    else if(d->connected >= CON_AUTHORIZE_NAME
     && d->connected <= CON_AUTHORIZE_LOGOUT
     && is_name(d->character->name, arg1))
    {
      if(mode == 1)
      {
        send_to_char(C_DEFAULT, "Character authorized.\n\r", ch);
        d->connected = CON_READ_MOTD;
        write_to_buffer(d, "You have been authorized.  Press return.\n\r", 0 );
	sprintf(buf, "%s!%s@%s AUTHORIZED by %s.",
	 d->character->name, d->user, d->host, ch->name);
	log_string(buf, CHANNEL_LOG, -1 );
        return;
      }
      else
      if(mode == 2)
      {
        send_to_char(C_DEFAULT, "Character denied.\n\r", ch);
        write_to_buffer(d, "Please choose a more medieval name.\n\r", 0 );
	sprintf(buf, "%s!%s@%s denied by %s.",
	 d->character->name, d->user, d->host, ch->name);
	log_string( buf, CHANNEL_LOG, -1 );
        close_socket(d);
        return;
      }
      else
      {
        send_to_char(C_DEFAULT, "Ok.\n\r", ch);
        sprintf(buf, "%s %s.\n\r", arg2, argument);
        write_to_buffer(d, buf, 0);
        close_socket(d);
        return;
      }
    }
  }
  if(mode)
    send_to_char(C_DEFAULT, "No such unauthorized person.\n\r", ch);
  return;
}

void send_to_area( AREA_DATA *pArea, char *txt )
{
  DESCRIPTOR_DATA *d;
  
  for ( d = descriptor_list; d; d = d->next )
  {
    if ( d->connected != CON_PLAYING )
      continue;
    if ( d->character )
    {
    if ( !( d->character->in_room ) || ( d->character->deleted ) 
      || !IS_OUTSIDE( d->character ) || !IS_AWAKE( d->character ) )
      continue;
    if ( d->character->in_room->area == pArea )
      send_to_char( C_DEFAULT, txt, d->character );
    }
  }
  return;
}

bool check_ban( DESCRIPTOR_DATA *dnew, bool loggedin )
{
  BAN_DATA *pban;
  DESCRIPTOR_DATA *dremove;
  
  for ( pban = ban_list; pban; pban = pban->next )
  {
    if ( ( !str_suffix( pban->name, dnew->host ) && !pban->user )
        || ( pban->user && !str_cmp( pban->user, dnew->user ) 
            && !str_suffix( pban->name, dnew->host ) ) )
    {
      if ( pban->user && !str_cmp( pban->user, dnew->user ) )
      if ( !str_suffix( pban->name, dnew->host ) )
      {
        write_to_descriptor( dnew->descriptor,
          "You have been banned from the Mud.\n\r", 0, dnew );
      }
      if ( !str_suffix( pban->name, dnew->host ) && 
         !pban->user )
      {
        write_to_descriptor( dnew->descriptor,
          "Your site has been banned from the Mud.\n\r", 0, dnew );
      }
      sprintf( log_buf, "%s!%s@%s being denied access.",
          dnew->character ? dnew->character->name : "(unknown)",
          dnew->user,
          dnew->host );

      bug( log_buf, 0 );
      close( dnew->descriptor );
      if ( loggedin )
      {
        for ( dremove = descriptor_list; dremove; dremove = dremove->next )
          if ( dremove->next == dnew ) break;
          
        dremove->next = dnew->next;
      }
      if ( dnew->character )
      {
        extract_char( dnew->character, TRUE );
      }
      free_mem( dnew->outbuf, dnew->outsize );
      free_descriptor(dnew);
      return TRUE;
    }
  }

  return FALSE;
}

void send_to_al( int clr, int level, char *text )
{
  CHAR_DATA *ch;
  
  for ( ch = char_list; ch; ch = ch->next )
  {
    if ( !ch->desc )
      continue;
    
    if ( get_trust( ch ) > level )
      send_to_char( clr, text, ch );
  }
  return;
}

bool is_pkillable( CHAR_DATA *ch, CHAR_DATA *victim ) {

    if ( is_safe( ch, victim ) ) {
    	send_to_char(AT_WHITE, "You cannot.\n\r", ch);
    	return FALSE;
    }

    if ( IS_AFFECTED4(victim, AFF_BURROW) )
    {
	send_to_char(AT_WHITE, "You cannot attack someone who is burrowed!\n\r",ch);
	return FALSE;
    }

    if ( IS_NPC(ch)  || IS_NPC(victim) ) {
    	return TRUE;
    }

    if( IS_SET(ch->in_room->room_flags, ROOM_PKILL ) && ch->level >= 30 && victim->level >= 30 && ( ( ch->clan != victim->clan || ch->clan == 0 || victim->clan == 0 ) ) )
    {
	victim->pkill_timer = 0;
	ch->pkill_timer = 0;
	return TRUE;
    }

    if (ch == victim)
	return TRUE;

    if (!ch->pkill) {
    	send_to_char(AT_WHITE, "You are peaceful, you cannot murder other players.\n\r", ch);
    } else if (!victim->pkill) {
    	send_to_char(AT_WHITE, "You cannot attack a peaceful player.\n\r", ch);
    } else if ( victim->pkill_timer > 0 ) {
        send_to_char(AT_WHITE, "You cannot attack someone who has just been killed.\n\r", ch);
    } else if ( ( ch->level < 30 ) || ( victim->level < 30 ) ) {
	send_to_char(AT_WHITE, "You can not murder anyone under level 30.\n\r", ch);
    } else if ( ( ch->level - PKILL_RANGE > victim->level ) || ( ch->level + PKILL_RANGE < victim->level ) ) {
	    send_to_char(AT_WHITE, "That is not in the pkill range... valid range is +/- 8 levels.\n\r", ch );
    } else if( ( victim->clan == ch->clan ) && ch->clan != 0 ) {
	send_to_char(C_DEFAULT, "You can't attack someone in the your clan.\n\r", ch );
    } else {
    	return TRUE;
    }
    return FALSE;
}

bool IS_SHIELDABLE( CHAR_DATA *ch ) {

    int temp1;
    int temp2;
    int temp3;

    temp1 = class_table[ch->class].no_shields;
    temp2 = class_table[ch->multied].no_shields;
    temp3 = 5;

    if (temp1 > temp2)
    {
	temp3 = temp2;
    }
    if (temp2 > temp1 )
    {
	temp3 = temp1;
    }
    if (temp1 == temp2)
    {
        temp3 = temp1;
    }
    if(ch->shields >= temp3)
    {
	send_to_char(AT_WHITE, "You have the maximum number of shields allowed for your class.\n\r", ch);
        return FALSE;
    }
    else
    {
	return TRUE;
    }
}

void ___exit( int arg )
{
	exit( arg );
}

#if !defined( RUN_AS_WIN32SERVICE )

int main( int argc, char **argv )
{
    struct timeval now_time;

	#if defined( unix )
		int control;
	#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
	#if defined( macintosh )
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
	#endif

    /*
     * Reserve one channel for our use.
     */
    if ( !( fpReserve = fopen( NULL_FILE, "r" ) ) )
    {
	perror( NULL_FILE );
	___exit( 0 );
    }

    /*
     * Get the port number.
     */
    port = 1234;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    ___exit( 0 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    ___exit( 0 );
	}
    }

    /*
     * Run the game.
     */
	#if defined( macintosh )  || defined( MSDOS )
    boot_db( );
    log_string( "OmegaMud is ready to rock.", CHANNEL_NONE, -1 );
    game_loop_mac_msdos( );
	#endif

	#if defined( unix )
    boot_db( );
    note_cleanup( );
    control = init_socket( port );
    sprintf( log_buf, "OmegmaMud is ready to rock on port %d.", port );
    log_string( log_buf, CHANNEL_NONE, -1 );
    {
      FILE *fp;
      char buf[MAX_INPUT_LENGTH];

		#if !defined( RUN_AS_WIN32SERVICE )
	sprintf(buf, "cp -f comlog%d.019 comlog%d.020", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.018 comlog%d.019", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.017 comlog%d.018", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.016 comlog%d.017", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.015 comlog%d.016", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.014 comlog%d.015", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.013 comlog%d.014", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.012 comlog%d.013", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.011 comlog%d.012", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.010 comlog%d.011", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.009 comlog%d.010", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.008 comlog%d.009", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.007 comlog%d.008", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.006 comlog%d.007", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.005 comlog%d.006", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.004 comlog%d.005", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.003 comlog%d.004", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.002 comlog%d.003", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.001 comlog%d.002", port, port );
      system( buf );
      sprintf(buf, "cp -f comlog%d.txt comlog%d.001", port, port );
      system( buf );
		#else
	  char from[MAX_STRING_LENGTH];
	  char to[MAX_STRING_LENGTH];
	  int i;
	  for(i = 19; i >=0 ; i--)
	  {
		  if (i==0)
		  {
			sprintf(from, "comlog%d.txt", port);
		  } else
		  {
			  sprintf(from, "comlog%d.%03d", port, i);
		  }
		  sprintf(to, "comlog%d.%03d", port, i+1);
		  copyfile(from, to);
	  }
		#endif
      sprintf(buf, "comlog%d.txt", port );

      fclose(fpReserve);
      if ( !(fp = fopen( buf, "w" ) ) )
	perror( buf );
      else
      {
	fprintf( fp, "%s: %s\n", ctime( &current_time ), log_buf );
	fclose(fp);
      }
      fpReserve = fopen(NULL_FILE, "r" );
    }

    game_loop_unix( control );
    close( control );
	#endif

	free_allocated_mem();
    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game.", CHANNEL_NONE, -1 );
    ___exit( 0 );
    return 0;
}

#else

void	ServiceMain(int argc, char** argv); 
void	ControlHandler(DWORD request);
int		WriteToLog(char* str);
int		InitService();

SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   hStatus;

int RegSvc()
{
	char strDir[1024];
	HANDLE schSCManager,schService;
	SC_ACTION restart_my_svc;
	char lpszBinaryPathName[MAX_STRING_LENGTH];
	SERVICE_FAILURE_ACTIONS my_service_actions;
	GetCurrentDirectory(1024,strDir);

	strcat(strDir,"\\Stormgate.exe"); 


	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS); 


	if (schSCManager == NULL)
	{
		return 0;
	}

	strcpy(lpszBinaryPathName,strDir);

	schService = CreateService(schSCManager,"Stormgate","Stormgate", // service name to display 
					SERVICE_ALL_ACCESS, // desired access 
					SERVICE_WIN32_OWN_PROCESS, // service type 
					SERVICE_DEMAND_START, // start type 
					SERVICE_ERROR_NORMAL, // error control type 
					lpszBinaryPathName, // service's binary 
					NULL, // no load ordering group 
					NULL, // no tag identifier 
					NULL, // no dependencies 
					NULL, // LocalSystem account 
					NULL); // no password 

	if (schService == NULL) 
		return 0; 

	
	restart_my_svc.Delay = 0;
	restart_my_svc.Type = SC_ACTION_RESTART;

	my_service_actions.dwResetPeriod = INFINITE;
	my_service_actions.lpRebootMsg = NULL;
	my_service_actions.lpCommand = NULL;
	my_service_actions.cActions = 1;
	my_service_actions.lpsaActions = &restart_my_svc;

	ChangeServiceConfig2(schService, SERVICE_CONFIG_FAILURE_ACTIONS, &my_service_actions);

	CloseServiceHandle(schService); 

	return -1;
}

int UnRegSvc()
{
	HANDLE schSCManager;
	SC_HANDLE hService;

	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);


	if (schSCManager == NULL) 
		return -1; 

	hService=OpenService(schSCManager,"Stormgate",SERVICE_ALL_ACCESS);

	if (hService == NULL) 
		return -1;

	if(DeleteService(hService)==0)
		return -1;

	if(CloseServiceHandle(hService)==0)
		return -1;
	else
		return 0;
}

int main(int argc, char** argv) 
{ 
	char buf[MAX_PATH];
	SERVICE_TABLE_ENTRY ServiceTable[2];
	if (argc > 1)
	{
		if(strcmp(argv[1],"\\r")==0)
		{
			printf("Registering Stormgate Service...");
			return RegSvc();
		} else if (strcmp(argv[1],"\\u")==0)
		{
			printf("Unregistering Stormgate Service...");
			return UnRegSvc();
		} else if (!isdigit(argv[1][0]))
		{
			return -1;
		} 
	}

   if (argc > 1)
   {
	   sprintf(buf, "Stormgate%s", argv[1]);
   } else
   {
	   strcpy(buf, "Stormgate");
   }
  
   ServiceTable[0].lpServiceName = buf;
   ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

   ServiceTable[1].lpServiceName = NULL;
   ServiceTable[1].lpServiceProc = NULL;
   // Start the control dispatcher thread for our service
   StartServiceCtrlDispatcher(ServiceTable); 
   
   return 0;
}

void copyfile(char* from, char* to)
{
	FILE* in = fopen(from,"r");
	char buf[MAX_STRING_LENGTH];
	if (in)
	{
		FILE* out = fopen(to, "w");
		if (out)
		{
			int count=0;
			while(!feof(in))
			{
				count = fread(buf, sizeof(char), MAX_STRING_LENGTH, in);
				fwrite(buf, sizeof(char), count, out);
			};
			fclose(out);
		}
		fclose(in);
	}
}

void ServiceMain(int argc, char** argv) 
{  
   char buf[MAX_STRING_LENGTH];
   HANDLE schSCManager;
   HANDLE hService;
   DWORD size;
   DWORD realsize;
   QUERY_SERVICE_CONFIG* conf;
   FILE* lognum;
   int ln;
   int control;
   struct timeval now_time;
   //Sleep(30000);

   ServiceStatus.dwServiceType				= SERVICE_WIN32; 
   ServiceStatus.dwCurrentState				= SERVICE_START_PENDING; 
   ServiceStatus.dwControlsAccepted			= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
   ServiceStatus.dwWin32ExitCode			= 0; 
   ServiceStatus.dwServiceSpecificExitCode	= 0; 
   ServiceStatus.dwCheckPoint				= 0; 
   ServiceStatus.dwWaitHint					= 0; 
 
   //Sleep(20000);

   if (argc > 1)
   {
	   sprintf(buf, "Stormgate%s", argv[1]);
   } else
   {
	   strcpy(buf, "Stormgate");
   }

   hStatus = RegisterServiceCtrlHandler(buf, (LPHANDLER_FUNCTION)ControlHandler); 
   if (hStatus == (SERVICE_STATUS_HANDLE)0) 
   { 
      // Registering Control Handler failed
      return; 
   }

   schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
   hService = OpenService(schSCManager, buf, SERVICE_ALL_ACCESS);

   QueryServiceConfig(hService, NULL, 0, &size);

   realsize = size;
   conf = malloc(size);

   QueryServiceConfig(hService, conf, realsize, &size);

   memset(buf, 0, MAX_STRING_LENGTH);
   strncpy(buf, conf->lpBinaryPathName, (int)((strrchr(conf->lpBinaryPathName, '\\') - conf->lpBinaryPathName + 1)));
  
   _chdir(buf);
   
   free(conf);

   CloseServiceHandle(hService);
   CloseServiceHandle(schSCManager);

   ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
   SetServiceStatus (hStatus, &ServiceStatus);

   lognum = fopen("lognum.txt","r");
   ln = 1000;
   if (lognum)
   {
	   memset(buf,0,MAX_STRING_LENGTH);
	   fread(buf, sizeof(char),MAX_STRING_LENGTH,lognum);
	   fclose(lognum);
	   ln = atoi(buf);
	   ln++;
   }
   sprintf(buf,"%d",ln);
   lognum = fopen("lognum.txt","w");
   fwrite(buf,sizeof(char),strlen(buf),lognum);
   fclose(lognum);
   sprintf(buf,"../log/%d.log", ln);
   __stderr = freopen(buf,"w",stderr);

   /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Reserve one channel for our use.
     */
    if ( !( fpReserve = fopen( NULL_FILE, "r" ) ) )
    {
	perror( NULL_FILE );
	___exit( 0 );
    }

    /*
     * Get the port number.
     */
    port = 1234;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( __stderr, "Usage: %s [port #]\n", argv[0] );
	    ___exit( 0 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( __stderr, "Port number must be above 1024.\n" );
	    ___exit( 0 );
	}
    }

    /*
     * Run the game.
     */
    boot_db( );
    note_cleanup( );
    control = init_socket( port );
    sprintf( log_buf, "OmegmaMud is ready to rock on port %d.", port );
    log_string( log_buf, CHANNEL_NONE, -1 );
    {
      FILE *fp;
      char buf[MAX_INPUT_LENGTH];

	  char from[MAX_STRING_LENGTH];
	  char to[MAX_STRING_LENGTH];
	  int i;
	  for(i = 19; i >=0 ; i--)
	  {
		  if (i==0)
		  {
			sprintf(from, "comlog%d.txt", port);
		  } else
		  {
			  sprintf(from, "comlog%d.%03d", port, i);
		  }
		  sprintf(to, "comlog%d.%03d", port, i+1);
		  copyfile(from, to);
	  }

      sprintf(buf, "comlog%d.txt", port );
      fclose(fpReserve);
      if ( !(fp = fopen( buf, "w" ) ) )
	perror( buf );
      else
      {
	fprintf( fp, "%s: %s\n", ctime( &current_time ), log_buf );
	fclose(fp);
      }
      fpReserve = fopen(NULL_FILE, "r" );
    }

    game_loop_unix( control );

	#ifdef RUN_AS_WIN32SERVICE
	closesocket( control );
	#else
    close( control );
	#endif
	
	free_allocated_mem();

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game.", CHANNEL_NONE, -1 );
    ___exit( 0 );
    return;
}

void ControlHandler(DWORD request) 
{ 
   switch(request) 
   { 
      case SERVICE_CONTROL_STOP:
         ServiceStatus.dwWin32ExitCode = 0; 
         ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
         SetServiceStatus (hStatus, &ServiceStatus);
         return; 
 
      case SERVICE_CONTROL_SHUTDOWN: 
         ServiceStatus.dwWin32ExitCode = 0; 
         ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
         SetServiceStatus (hStatus, &ServiceStatus);
         return; 
        
      default:
         break;
    } 
 
    // Report current status
    SetServiceStatus (hStatus, &ServiceStatus);
 
    return; 
}

#endif

