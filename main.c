#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define EDITOR_VERSION "0.1"
#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}

enum editorReadKey
{
    ARROW_UP = 1000,
    ARROW_LEFT,
    ARROW_DOWN,
    ARROW_RIGHT,
}

struct editorConfig
{
    int cx, cy;
    int screenrows;
    int screencols;
    struct termios term;
};

struct abuf
{
    char *b;
    int len;
};

struct editorConfig editor;

void die(const char *s)
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.term) == -1)
    {
        die("tcsetattr");
    }
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &editor.term) == -1)
    {
        die("tcgetattr");
    }

    atexit(disableRawMode);

    struct termios raw = editor.term;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        die("tcsetattr");
    }
}

int editorReadKey()
{
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
        {
            die("read");
        }

        if (c == '\x1b')
        {
            char seq[3];

            if (read(STDIN_FILENO, &seq[0], 1) != 1)
            {
                return '\x1b';
            }

            if (read(STDIN_FILENO, &seq[1], 1) != 1)
            {
                return '\x1b';
            }

            if (seq[0] == '[')
            {
                switch (seq[1])
                {
                    case 'A':
                        return ARROW_UP;
                    case 'B':
                        return ARROW_DOWN;
                    case 'C':
                        return ARROW_RIGHT;
                    case 'D':
                        return ARROW_LEFT;
                }
            }
            return '\x1b';
        }
        else
        {
            return c;
        }
    }
}

int getCursorPosition(int *rows, int *cols)
{
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    {
        return -1;
    }

    while (i < sizeof(buf) - 1)
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
        {
            break;
        }
        if (buf[i] == 'R')
        {
            break;
        }
        i++;
    }

    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
    {
        return -1;
    }
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    {
        return -1;
    }

    return 0;
}

int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
        {
            return -1;
        }
        return getCursorPosition(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void abAppend(struct abuf *ab, const char *s, int len)
{
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL)
    {
        return;
    }
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab)
{
    free(ab->b);
}

void editorDrawRows(struct abuf *ab)
{
    for (int y = 0; y < editor.screenrows; y++)
    {
        if (y == editor.screenrows / 3)
        {
            char welcome[80];
            int welcomeLength = snprintf(welcome, sizeof(welcome), "Text Editor Version: %", EDITOR_VERSION);

            if (welcomeLength > editor.screencols)
            {
                welcomeLength = editor.screencols;
            }

            int padding = (editor.screencols - welcomeLength) / 2;

            if (padding)
            {
                abAppend(ab, "~", 1);
                padding--;
            }

            while (padding--)
            {
                abAppend(ab, " ", 1);
            }
            abAppend(ab, welcome, welcomeLength);
        }
        else
        {
            abAppend(ab, "~", 1);
        }

        abAppend(ab, "\x1b[K", 3);
        if (y < editor.screenrows - 1)
        {
            abAppend(ab, "\r\n", 2);
        }
    }
}

void editorRefreshScreen()
{
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.cy + 1, editor.cx + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[H", 3);
    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void editorMoveCursor(int key)
{
    switch (key)
    {
        case ARROW_LEFT:
            if (editor.cx != 0)
            {
                editor.cx--;
            }
            break;
        case ARROW_RIGHT:
            if (editor.cx != editor.screencols - 1)
            {
                editor.cx++;
            }
            break;
        case ARROW_UP:
            if (editor.cy != 0)
            {
                editor.cy--;
            }
            break;
        case ARROW_DOWN:
            if (editor.cy != editor.screenrows - 1)
            {
                editor.cy++;
            }
            break;
    }
}

void editorProcessKeypress()
{
    int c = editorReadKey();

    switch (c)
    {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
    }
}

void initEditor()
{
    editor.cx = 0;
    editor.cy = 0;

    if (getWindowSize(&editor.screenrows, &editor.screencols) == -1)
    {
        die("getWindowSize");
    }
}

int main(void)
{
    enableRawMode();

    while (1)
    {
        editorRefreshScreen();
        editorProcessKeypress();
    }

    return(0);
}