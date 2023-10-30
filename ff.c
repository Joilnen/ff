/**
 * Author: Joilnen Batista Leite
 * email: joilnen@gmail.com
 * Lincense MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define FF_BUF_SIZE 500

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

int ff_ldir(struct ff_file *f)
{
    printf("%s %s\n", __FUNCTION__, f->name);
    DIR *dp;
    struct dirent *ep;
    dp = opendir(f->name);
    if (dp)
    {
        while (ep = readdir(dp))
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

int ff_init(struct ff_file *f)
{
    char buf[FF_BUF_SIZE];
    f->name = getcwd(buf, FF_BUF_SIZE - 1);
    if (f->name)
    {
        printf("%s\n", f->name);
        f->name = (char *) malloc (sizeof(char) * strlen(buf) + 1);
        strcpy(f->name, buf);
    }
    return 0;
}

char getch()
{
    int r;
    unsigned char c;
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    if ((r = read(0, &c, sizeof(c))) < 0)
        return r;
    else
        return c;
}

int ff_loop()
{
    for(;;)
    {
        char c = getch();
        if (c == 'q') break;
        // printf("%c\n", c);
    }
}

int main(int argc, char *argv[], char *env[])
{
    struct ff_file f;
    ff_init(&f);

    printf("%s\n", f.name);

    ff_ldir(&f);
    ff_loop();

    return 0;
}

