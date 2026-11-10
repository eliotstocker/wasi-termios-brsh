#include "termios.h"
#include "sys/ioctl.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    printf("Running wasi-termios-brsh test suite...\n");

    /* 1. Test tcgetattr default state */
    struct termios t;
    int rc = tcgetattr(0, &t);
    assert(rc == 0);
    assert(t.c_lflag & ICANON);
    assert(t.c_lflag & ECHO);
    assert(cfgetispeed(&t) == B38400);
    assert(cfgetospeed(&t) == B38400);
    printf("  [PASS] tcgetattr default state\n");

    /* 2. Test cfsetispeed and cfsetospeed */
    rc = cfsetispeed(&t, B115200);
    assert(rc == 0);
    assert(cfgetispeed(&t) == B115200);
    rc = cfsetospeed(&t, B115200);
    assert(rc == 0);
    assert(cfgetospeed(&t) == B115200);
    printf("  [PASS] baud rate get/set\n");

    /* 3. Test cfmakeraw */
    cfmakeraw(&t);
    assert((t.c_lflag & (ICANON | ECHO | ISIG)) == 0);
    assert(t.c_cc[VMIN] == 1);
    assert(t.c_cc[VTIME] == 0);
    printf("  [PASS] cfmakeraw\n");

    /* 4. Test tcsetattr with VMIN=0, VTIME=0 sets O_NONBLOCK */
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 0;
    rc = tcsetattr(0, TCSANOW, &t);
    assert(rc == 0);
    int flags = fcntl(0, F_GETFL, 0);
    if (flags >= 0) {
        assert(flags & O_NONBLOCK);
    }
    printf("  [PASS] tcsetattr bridges VMIN=0/VTIME=0 to O_NONBLOCK\n");

    /* 5. Test restoring canonical / blocking mode clears O_NONBLOCK */
    t.c_lflag |= ICANON;
    t.c_cc[VMIN] = 1;
    rc = tcsetattr(0, TCSANOW, &t);
    assert(rc == 0);
    flags = fcntl(0, F_GETFL, 0);
    if (flags >= 0) {
        assert((flags & O_NONBLOCK) == 0);
    }
    printf("  [PASS] tcsetattr restores blocking mode\n");

    /* 6. Test ioctl TIOCGWINSZ */
    struct winsize ws;
    rc = ioctl(0, TIOCGWINSZ, &ws);
    assert(rc == 0);
    assert(ws.ws_col > 0);
    assert(ws.ws_row > 0);
    printf("  [PASS] ioctl TIOCGWINSZ (%ux%u)\n", ws.ws_col, ws.ws_row);

    /* 7. Test tcflush, tcdrain, tcsendbreak */
    assert(tcflush(0, TCIFLUSH) == 0);
    assert(tcdrain(0) == 0);
    assert(tcsendbreak(0, 0) == 0);
    printf("  [PASS] tcflush, tcdrain, tcsendbreak stubs\n");

    /* 8. Test cfsetspeed, tcgetpgrp, tcsetpgrp, tcgetsid */
    rc = cfsetspeed(&t, B57600);
    assert(rc == 0);
    assert(cfgetispeed(&t) == B57600);
    assert(cfgetospeed(&t) == B57600);
    assert(tcgetpgrp(0) > 0);
    assert(tcsetpgrp(0, 1) == 0);
    assert(tcgetsid(0) > 0);
    printf("  [PASS] cfsetspeed, tcgetpgrp, tcsetpgrp, tcgetsid\n");

    /* 9. Test ioctl FIONREAD */
    int bytes_avail = -1;
    rc = ioctl(0, FIONREAD, &bytes_avail);
    assert(rc == 0);
    assert(bytes_avail == 0);
    printf("  [PASS] ioctl FIONREAD\n");

    /* 10. Test dynamic set_terminal_size */
    extern void set_terminal_size(int cols, int rows);
    set_terminal_size(120, 45);
    rc = ioctl(0, TIOCGWINSZ, &ws);
    assert(rc == 0);
    assert(ws.ws_col == 120);
    assert(ws.ws_row == 45);
    printf("  [PASS] dynamic set_terminal_size (120x45)\n");

    printf("\nAll wasi-termios-brsh tests passed successfully!\n");
    return 0;
}
