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
    raw.c_lflag &= ~(ECHO);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void)
{
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    return(0);
}