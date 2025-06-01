#include "terminal.h"

#include "../base/base.h"

#include <asm-generic/ioctls.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

/* Defines */
#define CTRL_KEY(k) ((k) & 0x1f)

/* Data */
struct terminal_config {
    int32_t screen_rows;
    int32_t screen_cols;
    struct termios orig_termios;
};

struct terminal_config t_config;
/* Terminal */
void critical_die(const char* s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_config.orig_termios) == -1 ) {
        critical_die("tcsetattr");
    }
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &t_config.orig_termios) == - 1) critical_die("tcgetattr");
    atexit(disable_raw_mode);

    struct termios raw = t_config.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) critical_die("tcsetattr");
}

char terminal_read_key() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) critical_die("read");
    }

    return c;
}

int32_t get_window_size(int32_t* rows, int32_t* cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    }

    *cols = ws.ws_col;
    *rows = ws.ws_row;

    return 0;
}

/* output */

void terminal_draw_rows() {
    for (int y = 0; y < t_config.screen_cols; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void terminal_refresh_screen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    terminal_draw_rows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/* input */

uint32_t terminal_process_keypress() {
    char c = terminal_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            return 0;
    }

    return 1;
}


/* Init  */
void terminal_init() {
    if (get_window_size(&t_config.screen_rows, &t_config.screen_cols) == -1) critical_die("get_window_size");
}


/* Main Loop */
int32_t terminal_loop(lua_State* L) {
    unused(L);

    enable_raw_mode();
    terminal_init();

    do {
        terminal_refresh_screen();
    } while (terminal_process_keypress());

    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);


    return 0;
}
