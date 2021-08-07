
#include <windows.h>
#include <errno.h>

static unsigned long long filetime_to_unix_epoch( const FILETIME *ft )
{
    unsigned long long res = (unsigned long long) ft->dwHighDateTime << 32;
    res |= ft->dwLowDateTime;
    res /= 10;
    res -= 11644473600000000ULL;	/* from Win epoch to Unix epoch */
    return res;
}

int gettimeofday( struct timeval *tv, void *tz )
{
    FILETIME  ft;
    unsigned long long tim;

    if(!tv)
    {
        errno = EINVAL; return -1;
    }

    GetSystemTimeAsFileTime (&ft);
    tim = filetime_to_unix_epoch (&ft);
    tv->tv_sec  = (long) (tim / 1000000L);
    tv->tv_usec = (long) (tim % 1000000L);
    return 0;
}
