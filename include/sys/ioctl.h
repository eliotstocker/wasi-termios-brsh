#ifndef _WASI_SYS_IOCTL_H
#define _WASI_SYS_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

struct winsize {
    unsigned short ws_row;    /* rows, in characters */
    unsigned short ws_col;    /* columns, in characters */
    unsigned short ws_xpixel; /* horizontal size, pixels (unused) */
    unsigned short ws_ypixel; /* vertical size, pixels (unused) */
};

#define TIOCGWINSZ 0x5413
#define TIOCSWINSZ 0x5414
#define FIONREAD   0x541B
#define FIONBIO    0x5421

int ioctl(int fd, unsigned long request, ...);

#ifdef __cplusplus
}
#endif

#endif /* _WASI_SYS_IOCTL_H */
