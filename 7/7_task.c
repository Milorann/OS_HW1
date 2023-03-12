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

    int parent_to_child_fd;
    int child_to_parent_fd;
    int result;
    ssize_t size;
    char str_buf[buf_size];
    char name_p2ch[] = "p2ch.fifo";
    char name_ch2p[] = "ch2p.fifo";

    if (mknod(name_p2ch, S_IFIFO | 0666, 0) < 0)
    {
        printf("Can\'t create the parent-to-child FIFO (it can already exist)\n");
        exit(-1);
    }

    if (mknod(name_ch2p, S_IFIFO | 0666, 0) < 0)
    {
        printf("Can\'t create the child-to-parent FIFO (it can already exist)\n");
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

        if ((parent_to_child_fd = open(name_p2ch, O_WRONLY)) < 0)
        {
            printf("parent: Can\'t open the parent-to-child FIFO for writting\n");
            exit(-1);
        }

        size = write(parent_to_child_fd, text, read_bytes);
        if (size != read_bytes)
        {
            printf("parent: Can\'t write the whole string to parent-to-child FIFO\n");
            exit(-1);
        }
        if (close(parent_to_child_fd) < 0)
        {
            printf("parent: Can\'t close the parent-to-child FIFO\n");
            exit(-1);
        }

        // reading from child
        int numbers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        if ((child_to_parent_fd = open(name_ch2p, O_RDONLY)) < 0)
        {
            printf("parent: Can\'t open the child-to-parent FIFO for reading\n");
            exit(-1);
        }
        size = read(child_to_parent_fd, numbers, sizeof(int) * 10);
        if (size != sizeof(int) * 10)
        {
            printf("parent: Can\'t read the whole array from child-to-parent FIFO\n");
            exit(-1);
        }

        if (close(child_to_parent_fd) < 0)
        {
            printf("parent: Can\'t close the child-to-parent FIFO\n");
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

        if ((parent_to_child_fd = open(name_p2ch, O_RDONLY)) < 0)
        {
            printf("child: Can\'t open the parent-to-child FIFO for reading\n");
            exit(-1);
        }

        size = read(parent_to_child_fd, str_buf, buf_size);
        if (size < 0)
        {
            printf("child: Can\'t read string from parent-to-child FIFO\n");
            exit(-1);
        }

        if (close(parent_to_child_fd) < 0)
        {
            printf("child: Can\'t close the parent-to-child FIFO\n");
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
        if ((child_to_parent_fd = open(name_ch2p, O_WRONLY)) < 0)
        {
            printf("child: Can\'t open the child-to-parent FIFO for writing\n");
            exit(-1);
        }
        size = write(child_to_parent_fd, numbers, sizeof(int) * 10);
        if (size != sizeof(int) * 10)
        {
            printf("child: Can\'t write all numbers to child-to-parent FIFO\n");
            exit(-1);
        }
        if (close(child_to_parent_fd) < 0)
        {
            printf("child: Can\'t close the child-to-parent FIFO\n");
            exit(-1);
        }

        printf("child: exit, the text is read and processed\n");
    }

    return 0;
}
