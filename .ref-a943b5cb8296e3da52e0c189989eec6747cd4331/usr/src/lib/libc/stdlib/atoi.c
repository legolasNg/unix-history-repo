#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)atoi.c	5.2 (Berkeley) %G%";
#endif LIBC_SCCS and not lint

atoi(p)
register char *p;
{
	register int n;
	register int f;

	n = 0;
	f = 0;
	for(;;p++) {
		switch(*p) {
		case ' ':
		case '\t':
			continue;
		case '-':
			f++;
		case '+':
			p++;
		}
		break;
	}
	while(*p >= '0' && *p <= '9')
		n = n*10 + *p++ - '0';
	return(f? -n: n);
}
