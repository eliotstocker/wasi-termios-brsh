#include "termios.h"
#include "sys/ioctl.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WEAK __attribute__((weak))

/*
 * Default terminal state in WASI/brsh:
 * Starts in canonical mode with echo and standard control characters.
 */
static struct termios g_termios = {
    .c_iflag = ICRNL | IXON,
    .c_oflag = OPOST | ONLCR,
    .c_cflag = CS8 | CREAD | CLOCAL | B38400,
    .c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN,
    .c_line = 0,
    .c_cc = {
        [VINTR] = 3,     /* ^C */
        [VQUIT] = 28,    /* ^\ */
        [VERASE] = 127,  /* DEL */
        [VKILL] = 21,    /* ^U */
        [VEOF] = 4,      /* ^D */
        [VTIME] = 0,
        [VMIN] = 1,      /* 1 char minimum for canonical/blocking read */
        [VSTART] = 17,   /* ^Q */
        [VSTOP] = 19,    /* ^S */
        [VSUSP] = 26,    /* ^Z */
    },
    .__c_ispeed = B38400,
    .__c_ospeed = B38400,
};

static struct winsize g_winsize = {
    .ws_row = 24,
    .ws_col = 80,
    .ws_xpixel = 0,
    .ws_ypixel = 0
};

WEAK speed_t cfgetospeed(const struct termios *t) {
    return t ? t->__c_ospeed : B38400;
}

WEAK speed_t cfgetispeed(const struct termios *t) {
    return t ? t->__c_ispeed : B38400;
}

WEAK int cfsetospeed(struct termios *t, speed_t speed) {
    if (!t) { errno = EINVAL; return -1; }
    t->__c_ospeed = speed;
    return 0;
}

WEAK int cfsetispeed(struct termios *t, speed_t speed) {
    if (!t) { errno = EINVAL; return -1; }
    t->__c_ispeed = speed;
    return 0;
}

WEAK int tcgetattr(int fd, struct termios *t) {
    (void)fd;
    if (!t) { errno = EINVAL; return -1; }
    *t = g_termios;
    return 0;
}

WEAK int tcsetattr(int fd, int optional_actions, const struct termios *t) {
    (void)optional_actions;
    if (!t) { errno = EINVAL; return -1; }
    g_termios = *t;

    /*
     * Bridge termios VMIN/VTIME to WASI file descriptor flags:
     * When canonical mode is disabled (~ICANON):
     * If VMIN == 0 and VTIME == 0, the program expects non-blocking polling.
     * Translate this to O_NONBLOCK so brsh returns ERRNO_AGAIN immediately.
     */
    if ((t->c_lflag & ICANON) == 0 && t->c_cc[VMIN] == 0 && t->c_cc[VTIME] == 0) {
        int fl = fcntl(fd, F_GETFL, 0);
        if (fl >= 0 && (fl & O_NONBLOCK) == 0) {
            fcntl(fd, F_SETFL, fl | O_NONBLOCK);
        }
    } else {
        int fl = fcntl(fd, F_GETFL, 0);
        if (fl >= 0 && (fl & O_NONBLOCK)) {
            fcntl(fd, F_SETFL, fl & ~O_NONBLOCK);
        }
    }

    return 0;
}

WEAK void cfmakeraw(struct termios *t) {
    if (!t) return;
    t->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    t->c_oflag &= ~OPOST;
    t->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    t->c_cflag &= ~(CSIZE | PARENB);
    t->c_cflag |= CS8;
    t->c_cc[VMIN] = 1;
    t->c_cc[VTIME] = 0;
}

WEAK int tcsendbreak(int fd, int duration) {
    (void)fd;
    (void)duration;
    return 0;
}

WEAK int tcdrain(int fd) {
    (void)fd;
    return 0;
}

WEAK int tcflush(int fd, int queue_selector) {
    (void)fd;
    (void)queue_selector;
    return 0;
}

WEAK int tcflow(int fd, int action) {
    (void)fd;
    (void)action;
    return 0;
}

/*
 * Terminal ioctl handling (TIOCGWINSZ, TIOCSWINSZ, FIONBIO)
 */
WEAK int ioctl(int fd, unsigned long request, ...) {
    va_list ap;
    va_start(ap, request);

    switch (request) {
        case TIOCGWINSZ: {
            struct winsize *ws = va_arg(ap, struct winsize *);
            va_end(ap);
            if (!ws) { errno = EINVAL; return -1; }

            /* Read dynamic terminal size from environment variables set by brsh */
            const char *env_cols = getenv("COLUMNS");
            const char *env_lines = getenv("LINES");
            if (env_cols && *env_cols) {
                int c = atoi(env_cols);
                if (c > 0) g_winsize.ws_col = (unsigned short)c;
            }
            if (env_lines && *env_lines) {
                int r = atoi(env_lines);
                if (r > 0) g_winsize.ws_row = (unsigned short)r;
            }

            *ws = g_winsize;
            return 0;
        }

        case TIOCSWINSZ: {
            const struct winsize *ws = va_arg(ap, const struct winsize *);
            va_end(ap);
            if (!ws) { errno = EINVAL; return -1; }
            g_winsize = *ws;
            return 0;
        }

        case FIONBIO: {
            const int *opt = va_arg(ap, const int *);
            va_end(ap);
            if (!opt) { errno = EINVAL; return -1; }
            int fl = fcntl(fd, F_GETFL, 0);
            if (fl < 0) return -1;
            return fcntl(fd, F_SETFL, *opt ? (fl | O_NONBLOCK) : (fl & ~O_NONBLOCK));
        }

        default:
            va_end(ap);
            errno = ENOTTY;
            return -1;
    }
}
