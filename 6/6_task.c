#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

const int buf_size = 5000;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Wrong number of command line arguments\n");
        exit(-1);
    }

    int parent_to_child_fd[2];
    int child_to_parent_fd[2];
    int result;
    ssize_t size;
    char str_buf[buf_size];

    if (pipe(parent_to_child_fd) < 0)
    {
        printf("Can\'t open parent-to-child pipe\n");
        exit(-1);
    }

    if (pipe(child_to_parent_fd) < 0)
    {
        printf("Can\'t open child-to-parent pipe\n");
        exit(-1);
    }

    result = fork();
    if (result < 0)
    {
        printf("Can\'t fork child\n");
        exit(-1);
    }
    else if (result > 0)
    { // parent process
        if (close(parent_to_child_fd[0]) < 0)
        {
            printf("parent: Can\'t close reading side of parent-to-child pipe\n");
            exit(-1);
        }

        int rfd;
        char text[buf_size];
        ssize_t read_bytes;
        if ((rfd = open(argv[1], O_RDONLY)) < 0)
        {
            printf("parent: Can\'t open the input file\n");
            exit(-1);
        }
        read_bytes = read(rfd, text, buf_size);
        if (read_bytes == -1)
        {
            printf("parent: Can\'t read the input file\n");
            exit(-1);
        }
        text[read_bytes] = '\0';
        // printf("%s\n", text);
        if (close(rfd) < 0)
        {
            printf("parent: Can\'t close the input file\n");
        }

        // for an empty file
        if (read_bytes == 0)
        {
            ++read_bytes;
        }

        size = write(parent_to_child_fd[1], text, read_bytes);
        if (size != read_bytes)
        {
            printf("parent: Can\'t write the whole string to parent-to-child pipe\n");
            exit(-1);
        }
        if (close(parent_to_child_fd[1]) < 0)
        {
            printf("parent: Can\'t close writing side of parent-to-child pipe\n");
            exit(-1);
        }

        // reading from child
        if (close(child_to_parent_fd[1]) < 0)
        {
            printf("parent: Can\'t close writing side of child-to-parent pipe\n");
            exit(-1);
        }
        int numbers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        size = read(child_to_parent_fd[0], numbers, sizeof(int) * 10);
        if (size != sizeof(int) * 10)
        {
            printf("parent: Can\'t read the whole array from child-to-parent pipe\n");
            exit(-1);
        }

        if (close(child_to_parent_fd[0]) < 0)
        {
            printf("parent: Can\'t close reading side of child-to-parent pipe\n");
            exit(-1);
        }

        int wfd;
        int str_sz;
        ssize_t wrsize;
        char string[100];
        if ((wfd = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0)
        {
            printf("parent: Can\'t open the output file\n");
            exit(-1);
        }
        for (int i = 0; i < 10; i++)
        {
            str_sz = sprintf(string, "%d: %d\n", i, numbers[i]);
            wrsize = write(wfd, string, str_sz);
            if (wrsize != str_sz)
            {
                printf("parent: Can\'t write all numbers\n");
                exit(-1);
            }
        }
        if (close(wfd) < 0)
        {
            printf("parent: Can\'t close the output file\n");
            exit(-1);
        }

        printf("parent: exit, the number of digits is written to the output file\n");
    }
    else
    { // child process

        if (close(parent_to_child_fd[1]) < 0)
        {
            printf("child: Can\'t close writing side of parent-to-child pipe\n");
            exit(-1);
        }

        size = read(parent_to_child_fd[0], str_buf, buf_size);
        if (size < 0)
        {
            printf("child: Can\'t read string from parent-to-child pipe\n");
            exit(-1);
        }

        if (close(child_to_parent_fd[0]) < 0)
        {
            printf("child: Can\'t close reading side of child-to-parent pipe\n");
            exit(-1);
        }

        int numbers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        for (int i = 0; i < size; i++)
        {
            if (str_buf[i] >= 48 && str_buf[i] <= 57)
            {
                numbers[str_buf[i] - 48]++;
            }
        }

        size = write(child_to_parent_fd[1], numbers, sizeof(int) * 10);
        if (size != sizeof(int) * 10)
        {
            printf("child: Can\'t write all numbers to child-to-parent pipe\n");
            exit(-1);
        }
        if (close(child_to_parent_fd[1]) < 0)
        {
            printf("child: Can\'t close writing side of child-to-parent pipe\n");
            exit(-1);
        }

        if (close(parent_to_child_fd[0]) < 0)
        {
            printf("child: Can\'t close writing side of parent-to-child pipe\n");
            exit(-1);
        }

        printf("child: exit, the text is read\n");
    }

    return 0;
}
