CC ?= clang
AR ?= llvm-ar
CFLAGS ?= -Wall -Wextra -O2 -Iinclude

TARGET_LIB = libwasi-termios.a
SRCS = src/termios.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: test/test_termios.c $(TARGET_LIB)
	$(CC) $(CFLAGS) test/test_termios.c $(TARGET_LIB) -o test/test_runner
	./test/test_runner

clean:
	rm -f src/*.o test/test_runner $(TARGET_LIB)

.PHONY: all test clean
