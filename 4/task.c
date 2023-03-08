// 02-parent-child.c
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

const int buf_size = 5000;
// const int buf_size = 10;
const int mes_size = 5000;
// const int mes_size = 5;

int main()
{
    int fd[2], result;
    size_t size;
    char str_buf[buf_size];

    if (pipe(fd) < 0)
    {
        printf("Can\'t open pipe\n");
        exit(-1);
    }

    result = fork();
    if (result < 0)
    {
        printf("Can\'t fork child\n");
        exit(-1);
    }
    else if (result > 0)
    { /* Parent process */
        if (close(fd[0]) < 0)
        {
            printf("parent: Can\'t close reading side of pipe\n");
            exit(-1);
        }

        // read from file
        FILE *myfile;
        myfile = fopen("input.txt", "r");
        char text[mes_size];
        int i = 0;
        while ((text[i] = fgetc(myfile)) != EOF || i < buf_size)
        {
            i++;
        }
        text[i] = '\0';
        text[buf_size - 1] = '\0';
        printf("%s\n", text);
        fclose(myfile);

        size = write(fd[1], text, mes_size);
        if (size != mes_size)
        {
            printf("Can\'t write all string to pipe\n");
            exit(-1);
        }
        if (close(fd[1]) < 0)
        {
            printf("parent: Can\'t close writing side of pipe\n");
            exit(-1);
        }
        printf("Parent exit\n");
    }
    else
    { /* Child process */

        result = fork();
        if (result < 0)
        {
            printf("Can\'t fork grandchild\n");
            exit(-1);
        }
        else if (result > 0)
        {
            size = read(fd[0], str_buf, mes_size);
            if (size < 0)
            {
                printf("Can\'t read string from pipe\n");
                exit(-1);
            }

            if (close(fd[0]) < 0)
            {
                printf("child: Can\'t close reading side of pipe\n");
                exit(-1);
            }

            size = write(fd[1], "34", 16);
            if (size != 16)
            {
                printf("Can\'t write all string to pipe\n");
                exit(-1);
            }
            if (close(fd[1]) < 0)
            {
                printf("child: Can\'t close writing side of pipe\n");
                exit(-1);
            }

            printf("Child exit, str_buf: %s\n", str_buf);
        }
        else
        { /* grandchild */
            if (close(fd[1]) < 0)
            {
                printf("grandchild: Can\'t close writing side of pipe\n");
                exit(-1);
            }
            size = read(fd[0], str_buf, 16);
            if (size < 0)
            {
                printf("Can\'t read string from pipe\n");
                exit(-1);
            }
            printf("grandChild exit, str_buf: %s\n", str_buf);
            if (close(fd[0]) < 0)
            {
                printf("grandchild: Can\'t close reading side of pipe\n");
                exit(-1);
            }
        }
    }

    return 0;
}
