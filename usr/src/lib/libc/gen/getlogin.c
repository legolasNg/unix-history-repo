#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getlogin.c	5.3 (Berkeley) %G%";
#endif LIBC_SCCS and not lint

#include <utmp.h>

static	char UTMP[]	= "/etc/utmp";
static	struct utmp ubuf;

char *
getlogin()
{
	register int me, uf;
	register char *cp;

	if (!(me = ttyslot()))
		return(0);
	if ((uf = open(UTMP, 0)) < 0)
		return (0);
	lseek (uf, (long)(me*sizeof(ubuf)), 0);
	if (read(uf, (char *)&ubuf, sizeof (ubuf)) != sizeof (ubuf)) {
		close(uf);
		return (0);
	}
	close(uf);
	ubuf.ut_name[sizeof (ubuf.ut_name)] = ' ';
	for (cp = ubuf.ut_name; *cp++ != ' '; )
		;
	*--cp = '\0';
	return (ubuf.ut_name);
}
