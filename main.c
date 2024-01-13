#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios term;

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
}

void enableRawMode()
{
    tcgetattr(STDIN_FILENO, &term);
    atexit(disableRawMode);

    struct termios raw = term;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void)
{
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    {
        if (iscntrl(c))
        {
            printf("%d\r\n", c);
        }
        else
        {
            printf("%d ('%c')\r\n", c, c);
        }
    }
    return(0);
}