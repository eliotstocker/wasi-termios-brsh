# wasi-termios-brsh

> Drop-in POSIX `termios` and terminal I/O shim for `wasi-libc` and [brsh](https://github.com/eliotstocker/Brsh-JS) WebAssembly environments.

Standard `wasi-libc` omits `<termios.h>` and `<sys/ioctl.h>`. This library provides a drop-in implementation that allows interactive terminal applications, games (such as `doom-ascii`), and command-line tools to compile cleanly to WebAssembly (`wasm32-wasip1`) without modifying their source code.

## Features

- **Full POSIX `<termios.h>` API**:
  - `tcgetattr`, `tcsetattr` (`TCSANOW`, `TCSADRAIN`, `TCSAFLUSH`)
  - `cfmakeraw`
  - `cfgetispeed`, `cfgetospeed`, `cfsetispeed`, `cfsetospeed`
  - `tcflush`, `tcdrain`, `tcsendbreak`, `tcflow`
- **Dynamic Non-blocking Polling**:
  - Automatically bridges non-canonical `VMIN == 0 && VTIME == 0` settings in `tcsetattr` to WASI `O_NONBLOCK` (`FDFLAGS_NONBLOCK`), enabling real-time games to poll keystrokes without blocking the frame loop.
- **Terminal Dimensions (`TIOCGWINSZ` & `set_terminal_size`)**:
  - Provides `<sys/ioctl.h>` with `ioctl(fd, TIOCGWINSZ, &ws)` reading dynamic `COLUMNS` and `LINES` environment variables provided by `brsh` (defaulting to 80×24), plus exported `set_terminal_size(cols, rows)` for live resize updates.
- **Host Signal Discipline Inspection (`get_termios_lflag`)**:
  - Exports `get_termios_lflag()` so host runtimes (`brsh`) can inspect `c_lflag` (`ISIG`, `ICANON`, `ECHO`) and accurately distinguish cbreak mode (`ISIG` enabled, `Ctrl+C` sends `SIGINT`) from full raw mode (`~ISIG`, `Ctrl+C` delivered as byte `0x03`).
- **Weak Symbols**:
  - All implementations use `__attribute__((weak))` so host or application-specific overrides link without conflicts.

## Installation / Usage

### 1. In your C / C++ project

Include the `include/` directory in your compiler search path and link `src/termios.c` (or `libwasi-termios.a`):

```bash
# Using wasi-sdk / clang:
clang --target=wasm32-wasip1 \
  -I/path/to/wasi-termios-brsh/include \
  /path/to/wasi-termios-brsh/src/termios.c \
  main.c -o app.wasm
```

### 2. In Makefiles / CMake

```makefile
CFLAGS += -I$(WASI_TERMIOS_DIR)/include
LIBS   += -L$(WASI_TERMIOS_DIR) -lwasi-termios
```

## Running Tests

```bash
make test
```

## License

MIT © [Eliot Stocker](https://github.com/eliotstocker)
