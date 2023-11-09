/**
 * Author: Joilnen Batista Leite
 * email: joilnen@gmail.com
 * Lincense MIT
 */

/*format
 * la{l d} ca{"ff-file manager"} ra{s}
 * a0{sy{+}f } a1{sy{+}f } a2{sy{+}f}
 * la{l d} ca{"ff-file manager"} ra{s}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>

#define FF_NUMBER_OF_AREAS 3
#define FF_AREA_WIDTH_FACTOR 16

#define FF_BUF_SIZE 500
#define FF_TIME_SCREEN_UPDATE 100000000

#define S_CSCREEN "\033[H\033[J"
#define S_CM "\033[24;62H"
#define S_CMR "\033[24;58H"
#define S_UNDERL "\033[1m"
#define S_AZUL "\033[44m\033[37m"
#define S_VERM "\033[41m\033[37m"
#define S_INV  "\033[7m"
#define S_NORM "\033[0m"
#define S_INFO "\033[24;0H\033[7m"
#define S_INFO_ST "\033[1;65H"
#define S_INFO_ST_1 "\033[2;65H"
#define S_CARD "\033[0;60H"
#define S_CHKLST "\033[%d;%dH%s\033[0m"
#define S_CHKLST_DONE "\033[46m\033[30m\033[%d;%dH%s\033[0m"
#define S_INFO2 "\033[46m\033[30m\033[24;0H"
#define S_LIST "\033[%d;78H%d"
#define S_FILEN "\033[0;55H"
#define S_VENN S_CHKLST

typedef enum tscreen {TMONO, TCOLOR} type_of_screen;

#define CLEAR_SCREEN printf(S_CSCREEN)
#define TYPE_OF_TERMINAL TCOLOR
#define INFO(x) printf("%s%s%s", S_INFO, x, S_NORM)
#define INFO2(x) printf("%s%s%s", S_INFO2, x, S_NORM)
#define INFO_ST(x) printf("%s%s%s%s", S_INV, S_INFO_ST, x, S_NORM)
#define CM printf("%s", m); fflush(stdout); getc(stdin)
#define CMR(x) printf("%s", S_CMR"q + * tecle enter *"); fflush(stdout); x = getc(stdin)
#define CKLSITEM(l,c,b,x) \
    if (b) printf(S_CHKLST_DONE, l, c, x); \
    else printf(S_CHKLST, l, c, x)
#define INFO_FILE printf("%s%s%sf:%s%s", S_INFO_ST, __FILE__, S_INFO_ST_1, __FUNCTION__, S_NORM); fflush(stdout);

#define FF_MIN(a, b) (a < b) ? (a) : (b)
#define FF_MAX(a, b) !FF_MIN(a, b)
#define FF_ABS(a) (a < 0) ? -(a) : a

char m[] = "STOP TEST MESSAGE ";

struct ff_file
{
    char *name;
    int perms;
};

struct ff_node
{
    struct ff_file file;
    struct ff_node *prev, *next;
};

struct ff_list {
    struct ff_node *head, *last, *nownode;
    unsigned int sz;
    char is_sorted;
};

struct ff_screen
{
    int width, height;
    struct ff_list header, message, bottom;
    struct ff_list area0, area1, area2;
    int header_width, area_width[3], area_height;
    int message_width, bottom_width, area_now;
    int xcursor, ycursor;
};

struct ff_settings {

    struct w {
        unsigned short r, c;
        unsigned short xp, yp;
    } w;

    struct ff_screen screen_sets;
    struct termios term, tterm;
    struct ff_file dir;

    int (*move)(struct ff_settings *self, char c);
    int (*screen_update)(struct ff_settings *self);

} setts;

static void ff_sort(struct ff_settings *self)
{

}

static int ff_list_init(struct ff_settings *self)
{
    self->screen_sets.area0.head =
    self->screen_sets.area1.last =
    self->screen_sets.area2.nownode = NULL;

    return 0;
}

static int ff_list_insert(struct ff_settings *self, struct ff_node *n)
{


    return 0;
}

static void get_screen_size(struct ff_settings *self)
{
    ioctl(0, TIOCGWINSZ, &self->w);
    self->screen_sets.width = self->w.c;
    self->screen_sets.height = self->w.r;
}

static void ff_filter(struct ff_settings *self, char *s)
{
    if (strlen(s) > self->screen_sets.area_width[0])
    {
        s[self->screen_sets.area_width[0] - 2] = '>';
        s[self->screen_sets.area_width[0] - 1] = '\0';
    }
}

static int setts_move(struct ff_settings *self, char c)
{

    if (c == 'j') strcpy(m, "DOWN ");
    if (c == 'k') strcpy(m, "UP ");
    if (c == 'h') strcpy(m, "LEFT ");
    if (c == 'l') strcpy(m, "RIGHT ");
    if (c == 'c') strcpy(m, "STOP TEST MESSAGE ");

    return 0;
}

static int setts_screen_update(struct ff_settings *self)
{
    int i = 0, area_now;
    printf("%s\n", self->dir.name);
    DIR *dp = NULL;
    struct stat st;
    struct dirent *ep = NULL;
    dp = opendir(self->dir.name);
    if (dp)
    {
        while (i < self->screen_sets.area_height && (ep = readdir(dp)))
        {
            ff_filter(self, ep->d_name);
            printf("%s ", ep->d_name);
            stat(ep->d_name, &st);
            ff_list_insert(self, ep->d_name);
            if ((st.st_mode & S_IFMT) == S_IFREG)
                printf("file\n");
            else  if ((st.st_mode & S_IFMT) == S_IFDIR)
                printf("dir\n");
            else
                printf("\n");
            ++i;
        }
        (void) closedir(dp);
    }
    else
    {
        perror("* erro\n");
        return -1;
    }

    for (area_now = self->screen_sets.area_now; area_now < 3; area_now++)
    {

    }

    return 0;
}

static char getch()
{
    int r;
    unsigned char c;
    fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    if ((r = read(STDIN_FILENO, &c, sizeof(c))) < 0)
        return r;
    else
        return c;
}


static int ff_loop(struct ff_settings *self)
{
    struct timespec ts;
    ts.tv_sec = 0; ts.tv_nsec = FF_TIME_SCREEN_UPDATE;
    struct termios ttmp, torig;
    tcgetattr(STDIN_FILENO, &torig);
    ttmp = torig;
    ttmp.c_lflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP |
        INLCR | IGNCR | ICRNL | IXON | ECHOE);
    for(;;)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &ttmp);
        char c = getch();
        self->move(self, c);
        tcsetattr(STDIN_FILENO, TCSANOW, &torig);
        CLEAR_SCREEN;
        self->screen_update(self);
        CM;
        nanosleep(&ts, &ts);
        if (c == 'q') break;
    }
    CLEAR_SCREEN;

    return 0;
}

static int ff_screen(struct ff_settings *s)
{
    char buf[FF_BUF_SIZE];
    s->dir.name = getcwd(buf, FF_BUF_SIZE - 1);
    if (s->dir.name)
    {
        printf("%s\n", s->dir.name);
        s->dir.name = (char *) malloc (sizeof(char) * FF_BUF_SIZE);
        strcpy(s->dir.name, buf);
    }

    return 0;
}

static int ff_init(struct ff_settings *s)
{
    ioctl(0, TIOCGWINSZ, &s->w);
    s->screen_sets.width = s->w.c;
    s->screen_sets.height = s->w.r;
    s->move = setts_move;
    s->screen_update = setts_screen_update;
    int piece = (int) ceil(s->screen_sets.width / FF_AREA_WIDTH_FACTOR);
    s->screen_sets.area_width[0] = 4 * piece;
    s->screen_sets.area_width[1] = s->screen_sets.area_width[2] =
        6 * piece;
    s->screen_sets.area_height = s->screen_sets.height - 2;
    s->screen_sets.area_now = 0;
    s->screen_sets.xcursor = s->screen_sets.ycursor = 0;

    return 0;
}

int main(int argc, char *argv[], char *env[])
{
    ff_init(&setts);
    ff_screen(&setts);
    ff_list_init(&setts);
    ff_loop(&setts);

    return 0;
}

