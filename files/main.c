#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

int size(char* name)
{
    FILE* file = fopen(name,"r");
    if (file != NULL)
    {
        int count = 0;
        char a;
        int key = 0;
        fscanf(file,"%c", &a);
        while (!feof(file))
        {
            if (((a == '\n') || (a == ' ')) && (key != 1))
            {
                ++count;
                key = 1;
            }
            if ((a != '\n') && (a != ' ') && (key == 1))
            {
                key = 0;
            }
            fscanf(file, "%c", &a);
        }
        return count;
    }
    else
    {
        fprintf(stderr, "Can't open file %s\n");
        return -1;
    }
    return 0;
}

void process_dir(char* name, short flag_r, short flag_s, int count_space)
{
    DIR* dir = opendir(name);
    struct dirent* a  = readdir(dir);
    struct stat status;
    char final[PATH_MAX +1 ];
    char link[PATH_MAX + 1];
    FILE* h;
    sprintf(final, "%s/%s", name, a->d_name);
    lstat(final, &status);
    while (a != NULL)
    {
        if ((S_ISDIR(status.st_mode) && (strcmp(a->d_name, ".") != 0) && (strcmp(a->d_name, "..") != 0)))
        {
            for (int i=0; i<count_space;i++)
            {
                printf("|    ");
            }
            printf("%s \033[34mdirect\033[0m\n", a->d_name);
            if ((flag_r == 0) || ((flag_r == 2) && (count_space < 2)))
            {
                process_dir(final, flag_r, flag_s, count_space+1);
            }
        }
        if (S_ISREG(status.st_mode))
        {
            for (int i=0; i<count_space;i++)
            {
                printf("|    ");
            }
            printf("%s \033[35mfile\033[0m -- \033[36m%d\033[0m\n", a->d_name, size(final));
        }
        if (S_ISLNK(status.st_mode))
        {
            for (int i=0; i<count_space;i++)
            {
                printf("|    ");
            }
            
            if (readlink(final, link, PATH_MAX) != -1)
            {
                printf("%s \033[32mlink\033[0m -> %s  ", a->d_name, link);
                if (flag_s == 0)
                {
                    if (size(link) >= 0)
                    {
                        printf("%d",size(link));
                    }
                }
                printf("\n");
            }
            else
            {
                printf("%s \033[32mlink\033[0m \033[31minvalid link\033[0m\n", a->d_name);
            }
        }
        a = readdir(dir);
        if (a != NULL)
        {
            sprintf(final, "%s/%s", name, a->d_name);
            if (lstat(final, &status) != 0)
            {
                fprintf(stderr, "Can't open file\n");
            }
        }
    }
}
int main(int argc, char* argv[])
{
    process_dir(argv[1], *argv[3] - '0', (argc == 5? 1 : 0),1);
    return 0;
}
