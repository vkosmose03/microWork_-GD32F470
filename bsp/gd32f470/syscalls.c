/**
 * @file  syscalls.c
 * @brief Minimal newlib system-call stubs.
 *
 * Providing our own stubs (instead of linking --specs=nosys.specs) keeps the
 * link free of the "<call> is not implemented" warnings that the library stubs
 * emit, so the firmware links with zero warnings (task requirement #7).
 *
 * Standard I/O is not used by this firmware (output goes through hal_uart), so
 * the file operations are inert.
 */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Provided by the linker script at the end of the .bss region. */
extern char end;

caddr_t _sbrk(int incr) {
    static char *heap = &end;
    char *prev = heap;
    heap += incr;
    return (caddr_t) prev;
}

int _write(int file, const char *ptr, int len) {
    (void) file;
    (void) ptr;
    return len; /* pretend everything was written */
}

int _read(int file, char *ptr, int len) {
    (void) file;
    (void) ptr;
    (void) len;
    return 0; /* EOF */
}

int _close(int file) {
    (void) file;
    return -1;
}

int _lseek(int file, int ptr, int dir) {
    (void) file;
    (void) ptr;
    (void) dir;
    return 0;
}

int _fstat(int file, struct stat *st) {
    (void) file;
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file) {
    (void) file;
    return 1;
}

int _getpid(void) {
    return 1;
}

int _kill(int pid, int sig) {
    (void) pid;
    (void) sig;
    errno = EINVAL;
    return -1;
}

void _exit(int status) {
    (void) status;
    for (;;) {
    }
}
