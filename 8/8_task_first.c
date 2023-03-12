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

    int first_to_second_fd;
    int second_to_first_fd;
    int result;
    ssize_t size;
    char str_buf[buf_size];
    char name_f2s[] = "f2s.fifo";
    char name_s2f[] = "s2f.fifo";

    if (mknod(name_f2s, S_IFIFO | 0666, 0) < 0)
    {
        printf("Can\'t create the first-to-second FIFO (it can already exist)\n");
        exit(-1);
    }

    int rfd;
    char text[buf_size];
    ssize_t read_bytes;
    if ((rfd = open(argv[1], O_RDONLY)) < 0)
    {
        printf("first: Can\'t open the input file\n");
        exit(-1);
    }
    read_bytes = read(rfd, text, buf_size);
    if (read_bytes == -1)
    {
        printf("first: Can\'t read the input file\n");
        exit(-1);
    }
    text[read_bytes] = '\0';
    // printf("%s\n", text);
    if (close(rfd) < 0)
    {
        printf("first: Can\'t close the input file\n");
    }

    // for an empty file
    if (read_bytes == 0)
    {
        ++read_bytes;
    }

    if ((first_to_second_fd = open(name_f2s, O_WRONLY)) < 0)
    {
        printf("first: Can\'t open the first-to-second FIFO for writting\n");
        exit(-1);
    }

    size = write(first_to_second_fd, text, read_bytes);
    if (size != read_bytes)
    {
        printf("first: Can\'t write the whole string to first-to-second FIFO\n");
        exit(-1);
    }
    if (close(first_to_second_fd) < 0)
    {
        printf("first: Can\'t close the first-to-second FIFO\n");
        exit(-1);
    }

    // reading from second
    int numbers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if ((second_to_first_fd = open(name_s2f, O_RDONLY)) < 0)
    {
        printf("first: Can\'t open the second-to-first FIFO for reading\n");
        exit(-1);
    }
    size = read(second_to_first_fd, numbers, sizeof(int) * 10);
    if (size != sizeof(int) * 10)
    {
        printf("first: Can\'t read the whole array from second-to-first FIFO\n");
        exit(-1);
    }

    if (close(second_to_first_fd) < 0)
    {
        printf("first: Can\'t close the second-to-first FIFO\n");
        exit(-1);
    }

    int wfd;
    int str_sz;
    ssize_t wrsize;
    char string[100];
    if ((wfd = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0)
    {
        printf("first: Can\'t open the output file\n");
        exit(-1);
    }
    for (int i = 0; i < 10; i++)
    {
        str_sz = sprintf(string, "%d: %d\n", i, numbers[i]);
        wrsize = write(wfd, string, str_sz);
        if (wrsize != str_sz)
        {
            printf("first: Can\'t write all numbers\n");
            exit(-1);
        }
    }
    if (close(wfd) < 0)
    {
        printf("first: Can\'t close the output file\n");
        exit(-1);
    }

    printf("first: exit, the number of digits is written to the output file\n");

    return 0;
}
