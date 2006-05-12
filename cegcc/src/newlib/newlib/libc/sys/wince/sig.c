/* libc/sys/wince/signal.c */

int sigmask(int signum)
{
    return 1 << signum;
}

int sigblock(int mask)
{
	return 0;
}

int sigsetmask(int newmask)
{
	return 0;
}
