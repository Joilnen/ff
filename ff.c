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

typedef enum tscreen {TMONO, TCOLOR} type_of_screen;
enum EV {NONE, UP, DOWN, LEFT, RIGHT, DONE, TIMEOUT, NEVENTS};

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

    unsigned int ev;
} setts;

static struct ff_node *ff_create_node(char *name)
{
    struct ff_node *n = (struct ff_node*) malloc(sizeof(struct ff_node));
    n->file.name = calloc(strlen(name) + 1, sizeof(char));
    strncpy(n->file.name, name, strlen(name));
    n->prev = n->next = NULL;
    return n;
}

static void ff_sort(struct ff_settings *self)
{

}

static void ff_print_list(struct ff_settings *self)
{
    int i = 0;
    struct ff_node *tmp = NULL;
    tmp = self->screen_sets.area0.head;
    for (;tmp && i < self->screen_sets.area_height; tmp = tmp->next)
    {
        printf("%s %s\n", __FUNCTION__, tmp->file.name);
        ++i;
    }
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
    if (!self->screen_sets.area0.head)
    {
        self->screen_sets.area0.head =
        self->screen_sets.area0.last =
        self->screen_sets.area0.nownode = n;
    }
    else
    {
        if (self->screen_sets.area0.head->file.name[0] > n->file.name[0])
        {
            n->next = self->screen_sets.area0.head;
            self->screen_sets.area0.head->prev = n;
            self->screen_sets.area0.nownode = n;
            self->screen_sets.area0.head = n;
        }
        else
        {
            struct ff_node *tmp;
            n->prev = self->screen_sets.area0.head;
            tmp = self->screen_sets.area0.head->next;
            self->screen_sets.area0.head->next = n;
            self->screen_sets.area0.nownode = n;
            n->next = tmp;
        }
    }

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
    if (c == 'j') {
        strcpy(m, "DOWN ");
        self->ev = DOWN;
    }

    if (c == 'k') {
        strcpy(m, "UP ");
        self->ev = UP;
    }

    if (c == 'h') {
        strcpy(m, "LEFT ");
        self->ev = LEFT;
    }

    if (c == 'l') {
        strcpy(m, "RIGHT ");
        self->ev = RIGHT;
    }

    if (c == 'c') {
        strcpy(m, "STOP TEST MESSAGE ");
        self->ev = NONE;
    }

    return 0;
}

static int setts_screen_update(struct ff_settings *self)
{
    int i = 0, area_now;
    printf("%s\n", self->dir.name);
    DIR *dp = NULL;
    struct stat st;
    struct dirent *ep = NULL;
    switch(self->ev)
    {
        default:
            // update;
            break;
        case DOWN:
            dp = opendir(self->dir.name);
            if (dp)
            {
                while (i < self->screen_sets.area_height && (ep = readdir(dp)))
                {
                    ff_filter(self, ep->d_name);
                    printf("%s ", ep->d_name);
                    stat(ep->d_name, &st);
                    if ((st.st_mode & S_IFMT) == S_IFREG)
                        printf("file\n");
                    else  if ((st.st_mode & S_IFMT) == S_IFDIR)
                        printf("dir\n");
                    else
                        printf("\n");
                    ++i;
                }
            }
            else
            {
                perror("* erro\n");
                return -1;
            }
            (void) closedir(dp);
        break;
        case UP:
            ff_print_list(self);
    }

    /****
     * update areas
    for (area_now = self->screen_sets.area_now; area_now < 3; area_now++)
    {

    }
    ****/

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

static void ff_destroy_areas(struct ff_settings *self)
{
    struct ff_node *tmp, *todel;
    tmp = self->screen_sets.area0.head;
    for (; tmp; tmp = tmp->next)
        if (tmp->file.name) free(tmp->file.name);
    tmp = self->screen_sets.area0.head;
    for (; tmp;)
    {
        todel = tmp;
        if (todel) {
            tmp = tmp->next;
            free(todel);
        }
    }
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
        self->screen_update(self);
        CM;
        nanosleep(&ts, &ts);
        CLEAR_SCREEN;
        if (c == 'q') break;
    }
    ff_destroy_areas(self);

    return 0;
}

static int ff_screen(struct ff_settings *self)
{
    char buf[FF_BUF_SIZE];
    self->dir.name = getcwd(buf, FF_BUF_SIZE - 1);
    if (self->dir.name)
    {
        printf("%self\n", self->dir.name);
        self->dir.name = (char *) malloc (sizeof(char) * FF_BUF_SIZE);
        strcpy(self->dir.name, buf);
    }

    return 0;
}

static int ff_create_areas(struct ff_settings *self)
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
            stat(ep->d_name, &st);
            ff_list_insert(self, ff_create_node(ep->d_name));
        }
        (void) closedir(dp);
    }
    else
    {
        perror("* erro\n");
        return -1;
    }

    return 0;
}

static int ff_init(struct ff_settings *self)
{
    ioctl(0, TIOCGWINSZ, &self->w);
    self->screen_sets.width = self->w.c;
    self->screen_sets.height = self->w.r;
    self->move = setts_move;
    self->screen_update = setts_screen_update;
    int piece = (int) ceil(self->screen_sets.width / FF_AREA_WIDTH_FACTOR);
    self->screen_sets.area_width[0] = 4 * piece;
    self->screen_sets.area_width[1] = self->screen_sets.area_width[2] =
        6 * piece;
    self->screen_sets.area_height = self->screen_sets.height - 2;
    self->screen_sets.area_now = 0;
    self->screen_sets.xcursor = self->screen_sets.ycursor = 0;

    self->ev = NONE;

    return 0;
}

int main(int argc, char *argv[], char *env[])
{
    ff_init(&setts);
    ff_screen(&setts);
    ff_list_init(&setts);
    ff_create_areas(&setts);
    ff_loop(&setts);

    return 0;
}

