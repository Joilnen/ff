/**
 * Author: Joilnen Batista Leite
 * email: joilnen@gmail.com
 * Lincense MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>

#define FF_BUF_SIZE 500

struct ff_wsize {
    unsigned short rows, cols;
    unsigned short xpixel, ypixel;
} ff_wsize;

struct ff_file
{
    char *name;
    int perms;
};

struct ff_node
{
    struct ff_file file;
    struct ff_node *left, *right;
};

struct ff_tree
{
    struct ff_node *root, *last, *lvisited;
};

struct ff_list
{
    struct ff_node *head, *last, *lvisited;
};

struct ff_screen
{
    struct ff_list *header, *message, *bottom;
    struct ff_list *area0, *area1, *area2;
};

int ff_ldir(struct ff_file *f)
{
    printf("%s %s\n", __FUNCTION__, f->name);
    DIR *dp;
    struct dirent *ep;
    dp = opendir(f->name);
    if (dp)
    {
        while ((ep = readdir(dp)))
            puts(ep->d_name);
        (void) closedir(dp);
    }
    else
    {
        perror("* erro\n");
        return -1;
    }

    return 0;
}

int ff_screen(struct termios *t, struct ff_file *f)
{
    char buf[FF_BUF_SIZE];
    f->name = getcwd(buf, FF_BUF_SIZE - 1);
    if (f->name)
    {
        printf("%s\n", f->name);
        f->name = (char *) malloc (sizeof(char) * FF_BUF_SIZE);
        strcpy(f->name, buf);
    }

    return 0;
}

int ff_init(struct termios *t, struct ff_file *f)
{

    ioctl( 0, TIOCGWINSZ, &ff_wsize);
    int limit = ff_wsize.rows * ff_wsize.cols;
    printf("Lines %d and Columns %d\n", ff_wsize.rows, ff_wsize.cols);
    for( int i = 0; i < limit; i++ )
        putchar( '+' );

    return 0;
}

char getch()
{
    int r;
    unsigned char c;
    fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    if ((r = read(STDIN_FILENO, &c, sizeof(c))) < 0)
        return r;
    else
        return c;
}

int ff_loop()
{

    struct termios ttmp, torig;
    tcgetattr(STDIN_FILENO, &torig);
    ttmp.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP |
        INLCR | IGNCR | ICRNL | IXON);
    tcsetattr(STDIN_FILENO, TCSANOW, &ttmp);
    for(;;)
    {
        char c = getch();
        if (c == 'q') break;
        if (c == 'j') printf("PRA BAIXO\n");
        if (c == 'k') printf("PRA CIMA\n");
        if (c == 'h') printf("PRA ESQUERDA\n");
        if (c == 'l') printf("PRA ESQUERDA\n");
        // printf("%c\n", c);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &torig);

    return 0;
}

int main(int argc, char *argv[], char *env[])
{
    struct termios term;
    struct ff_file f;
    ff_init(&term, &f);
    ff_screen(&term, &f);
    ff_ldir(&f);
    ff_loop();

    return 0;
}
