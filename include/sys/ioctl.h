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

#ifndef TIOCGWINSZ
#define TIOCGWINSZ 0x5413
#endif
#ifndef TIOCSWINSZ
#define TIOCSWINSZ 0x5414
#endif
#ifndef TIOCGPGRP
#define TIOCGPGRP  0x540F
#endif
#ifndef TIOCSPGRP
#define TIOCSPGRP  0x5410
#endif
#ifndef TIOCSTI
#define TIOCSTI    0x5412
#endif
#ifndef FIONREAD
#define FIONREAD   0x541B
#endif
#ifndef FIONBIO
#define FIONBIO    0x5421
#endif

int ioctl(int fd, unsigned long request, ...);
void set_terminal_size(int cols, int rows);

#ifdef __cplusplus
}
#endif

#endif /* _WASI_SYS_IOCTL_H */
