#include "terminal.h"

#include "../base/base.h"
#include "../ptable/ptable.h"

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

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

enum editor_key {
ARROW_LEFT = 1000,
ARROW_RIGHT,
ARROW_UP,
ARROW_DOWN,
DEL_KEY,
HOME_KEY,
END_KEY,
PAGE_UP,
PAGE_DOWN
};

struct cursor_params {
    int x, y;
};

/* Data */
struct erow {
    size_t size;
    char* chars;
};

struct terminal_config {
    struct cursor_params c_params;
    int32_t screen_rows;
    int32_t screen_cols;
    int32_t numrows;
    struct erow row;
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

uint32_t terminal_read_key() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) critical_die("read");
    }

    if (c == '\x1b') {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch(seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == '0') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }

        return '\x1b';
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

/* file io */
int32_t terminal_open(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return -1;

    char* line = NULL;
    size_t linecap = 0;
    int32_t linelen = getline(&line, &linecap, fp);
    if (linelen != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r'))
            linelen--;

        t_config.row.size = linelen;
        t_config.row.chars = malloc(linelen + 1);
        memcpy(t_config.row.chars, line, linelen);
        t_config.row.chars[linelen] = '\0';
        t_config.numrows = 1;
    }

    free(line);
    fclose(fp);

    return linelen;
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
        if (y >= t_config.numrows) {
            if (t_config.numrows == 0 && y == t_config.screen_rows / 3) {
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

        } else {
            size_t len = t_config.row.size;
            if (len > (size_t) t_config.screen_cols) len = t_config.screen_cols;
            ab_append(ab, t_config.row.chars, len);
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

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", t_config.c_params.y + 1, t_config.c_params.x + 1);
    ab_append(&ab, buf, strlen(buf));
    ab_append(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);

    ab_free(&ab);
}

/* input */

void terminal_move_cursor(uint32_t key) {
    switch (key) {
        case ARROW_LEFT:
            if (t_config.c_params.x != 0) t_config.c_params.x--;
            break;
        case ARROW_RIGHT:
            if (t_config.c_params.x != t_config.screen_cols - 1) t_config.c_params.x++;
            break;
        case ARROW_DOWN:
            if (t_config.c_params.y != t_config.screen_rows - 1) t_config.c_params.y++;
            break;
        case ARROW_UP:
            if (t_config.c_params.y != 0) t_config.c_params.y--;
            break;
    }
}

uint32_t terminal_process_keypress() {
    uint32_t c = terminal_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            return 0;
        case HOME_KEY:
            t_config.c_params.x = 0;
            break;
        case END_KEY:
            t_config.c_params.x = t_config.screen_cols - 1;
            break;
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case ARROW_DOWN:
        case ARROW_UP:
            terminal_move_cursor(c);
            break;
        case PAGE_UP:
        case PAGE_DOWN:
        {
            int32_t times = t_config.screen_rows;
            while (times--) {
                terminal_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
        } break;
    }

    return 1;
}


/* Init  */
void terminal_init() {
    t_config.c_params.x = 0;
    t_config.c_params.y = 0;
    t_config.numrows = 0;

    if (get_window_size(&t_config.screen_rows, &t_config.screen_cols) == -1) critical_die("get_window_size");
}


/* Main Loop */
int32_t terminal_loop(lua_State* L) {
    unused(L);

    enable_raw_mode();
    terminal_init();
    if (terminal_open("test.lua") < 0) return -1;

    do {
        terminal_refresh_screen();
    } while (terminal_process_keypress());

    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);


    return 0;
}
