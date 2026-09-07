#ifndef FUTURA_XORRISO_COMPAT_H
#define FUTURA_XORRISO_COMPAT_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/select.h>
#include <sys/statvfs.h>
#include <pthread.h>
/* FuturaOS pthread API compatibility.
 * Some xorriso translation units are built with the toolchain's minimal
 * pthread header, which may not expose the detach-state API.  FuturaOS
 * implements these symbols in libc, so provide the declarations here. */
#ifndef PTHREAD_CREATE_JOINABLE
#define PTHREAD_CREATE_JOINABLE 0
#endif
#ifndef PTHREAD_CREATE_DETACHED
#define PTHREAD_CREATE_DETACHED 1
#endif
#ifndef FUTURA_XORRISO_PTHREAD_DETACH_DECL
#define FUTURA_XORRISO_PTHREAD_DETACH_DECL 1
extern int pthread_attr_setdetachstate(pthread_attr_t *, int);
#endif
#ifndef HAVE_TM_GMTOFF
#define HAVE_TM_GMTOFF 0
#endif
/* FuturaOS libc has no global timezone variable. Its time conversion
 * keeps the zone offset in struct tm, so use UTC as the legacy fallback. */
#ifndef Libburnia_timezonE
#define Libburnia_timezonE 0
#endif
#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif
int mkfifo(const char *, mode_t);
#endif
