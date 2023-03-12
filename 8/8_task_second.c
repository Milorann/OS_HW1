#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

const int buf_size = 5000;

int main(int argc, char *argv[])
{
    int first_to_second_fd;
    int second_to_first_fd;
    int result;
    ssize_t size;
    char str_buf[buf_size];
    char name_f2s[] = "f2s.fifo";
    char name_s2f[] = "s2f.fifo";

    if (mknod(name_s2f, S_IFIFO | 0666, 0) < 0)
    {
        printf("Can\'t create the second-to-first FIFO (it can already exist)\n");
        exit(-1);
    }

    if ((first_to_second_fd = open(name_f2s, O_RDONLY)) < 0)
    {
        printf("second: Can\'t open the first-to-second FIFO for reading\n");
        exit(-1);
    }

    size = read(first_to_second_fd, str_buf, buf_size);
    if (size < 0)
    {
        printf("second: Can\'t read string from first-to-second FIFO\n");
        exit(-1);
    }

    if (close(first_to_second_fd) < 0)
    {
        printf("second: Can\'t close the first-to-second FIFO\n");
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
    if ((second_to_first_fd = open(name_s2f, O_WRONLY)) < 0)
    {
        printf("second: Can\'t open the second-to-first FIFO for writing\n");
        exit(-1);
    }
    size = write(second_to_first_fd, numbers, sizeof(int) * 10);
    if (size != sizeof(int) * 10)
    {
        printf("second: Can\'t write all numbers to second-to-first FIFO\n");
        exit(-1);
    }
    if (close(second_to_first_fd) < 0)
    {
        printf("second: Can\'t close the second-to-first FIFO\n");
        exit(-1);
    }

    printf("second: exit, the text is read and processed\n");

    return 0;
}
