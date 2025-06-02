#include "terminal.h"

#include "../base/base.h"

#include <asm-generic/ioctls.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

/* Defines */
#define EDITOR_VERSION "0.0.1"

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

int32_t get_cursor_position(int32_t *rows, int32_t* cols) {
    char buf[32];
    uint32_t i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) -1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d,%d", rows, cols) != 2) return -1;

    //terminal_read_key();

    return 0;
}

int32_t get_window_size(int32_t* rows, int32_t* cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return get_cursor_position(rows, cols);
    }

    *cols = ws.ws_col;
    *rows = ws.ws_row;

    return 0;
}

/* append buffer / temp buffer / pre piece table */
struct abuf {
    char* b;
    size_t len;
};

#define ABUF_INIT {NULL, 0}

void ab_append(struct abuf *ab, const char* s, size_t len) {
    char* new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void ab_free(struct abuf* ab) {
    free(ab->b);
}

/* output */

void terminal_draw_rows(struct abuf* ab) {

    for (int y = 0; y < t_config.screen_rows; y++) {
        if (y == t_config.screen_rows / 3) {
            char welcome[80];
            size_t welcome_len = snprintf(welcome, sizeof(welcome),
                                          "Welcome to Lumerie %s", EDITOR_VERSION);

            if ((int) welcome_len > t_config.screen_cols) welcome_len = t_config.screen_cols;
            int padding = (t_config.screen_cols - welcome_len) / 2;
            if (padding) {
                ab_append(ab, "~", 1);
                padding--;
            }
            while (padding--) ab_append(ab, " ", 1);

            ab_append(ab, welcome, welcome_len);
        } else {
            ab_append(ab, "~", 1);
        }


        ab_append(ab, "\x1b[K", 3);
        if (y < t_config.screen_rows -1) {
            ab_append(ab, "\r\n", 2);
        }

    }
}

void terminal_refresh_screen() {
    struct abuf ab = ABUF_INIT;

    ab_append(&ab, "\x1b[?25l", 6);
    ab_append(&ab, "\x1b[H", 3);

    terminal_draw_rows(&ab);

    ab_append(&ab, "\x1b[H", 3);
    ab_append(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);

    ab_free(&ab);
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
