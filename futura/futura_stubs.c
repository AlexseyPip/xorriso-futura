#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

/* FuturaOS currently has no FIFO VFS node creation syscall. xorriso can still
   read/write regular ISO image files; FIFO restoration simply reports ENOSYS. */
int mkfifo(const char *path, mode_t mode)
{
    (void)path; (void)mode;
    errno = ENOSYS;
    return -1;
}

/* This xorriso release contains the signal-manager hook but no libdax API
   chain implementation. Returning -2 tells the caller to keep the signal
   handler from forcing an exit. */
int libdax_api_handle_abort(void *api_chain, char *msg, int flag)
{
    (void)api_chain; (void)msg; (void)flag;
    return -2;
}
