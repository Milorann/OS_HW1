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

    int first_to_second_fd, result1;
    ssize_t size1;
    char str_buf[buf_size];
    char name1[] = "listerenko1.fifo";

    mknod(name1, S_IFIFO | 0666, 0);

    result1 = fork();
    if (result1 < 0)
    {
        printf("Can\'t fork child\n");
        exit(-1);
    }
    else if (result1 > 0)
    { // first process
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

        if ((first_to_second_fd = open(name1, O_WRONLY)) < 0)
        {
            printf("Can\'t open the first FIFO for writting\n");
            exit(-1);
        }

        size1 = write(first_to_second_fd, text, read_bytes);
        if (size1 != read_bytes)
        {
            printf("first: Can\'t write the whole string to the first FIFO\n");
            exit(-1);
        }
        if (close(first_to_second_fd) < 0)
        {
            printf("first: Can\'t close the first FIFO\n");
            exit(-1);
        }
        printf("first: exit\n");
        // printf("first: text:\n%s\n", text);
    }
    else
    { // second process
        int second_to_third_fd, result2;
        ssize_t size2;
        char name2[] = "listerenko2.fifo";

        mknod(name2, S_IFIFO | 0666, 0);

        result2 = fork();
        if (result2 < 0)
        {
            printf("second: Can\'t fork child\n");
            exit(-1);
        }
        else if (result2 > 0)
        {
            if ((first_to_second_fd = open(name1, O_RDONLY)) < 0)
            {
                printf("Can\'t open the first FIFO for reading\n");
                exit(-1);
            }

            size1 = read(first_to_second_fd, str_buf, buf_size);
            if (size1 < 0)
            {
                printf("second: Can\'t read string from the first FIFO\n");
                exit(-1);
            }

            if (close(first_to_second_fd) < 0)
            {
                printf("second: Can\'t close the first FIFO\n");
                exit(-1);
            }

            if ((second_to_third_fd = open(name2, O_WRONLY)) < 0)
            {
                printf("Can\'t open the second FIFO for writing\n");
                exit(-1);
            }

            // printf("second: str_buf:\n%s\n", str_buf);
            int numbers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            for (int i = 0; i < size1; i++)
            {
                if (str_buf[i] >= 48 && str_buf[i] <= 57)
                {
                    numbers[str_buf[i] - 48]++;
                }
            }

            size2 = write(second_to_third_fd, numbers, sizeof(int) * 10);
            if (size2 != sizeof(int) * 10)
            {
                printf("second: Can\'t write all numbers to the second FIFO\n");
                exit(-1);
            }
            if (close(second_to_third_fd) < 0)
            {
                printf("second: Can\'t close the second FIFO\n");
                exit(-1);
            }

            printf("second: exit, the text is read\n");
        }
        else
        { // third process
            if ((second_to_third_fd = open(name2, O_RDONLY)) < 0)
            {
                printf("Can\'t open the second FIFO for reading\n");
                exit(-1);
            }
            int numbers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

            size2 = read(second_to_third_fd, numbers, sizeof(int) * 10);
            if (size2 != sizeof(int) * 10)
            {
                printf("third: Can\'t read the whole array from the second FIFO\n");
                exit(-1);
            }

            if (close(second_to_third_fd) < 0)
            {
                printf("third: Can\'t close reading side of the second FIFO\n");
                exit(-1);
            }

            int wfd;
            int str_sz;
            ssize_t size;
            char string[100];
            if ((wfd = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0)
            {
                printf("third: Can\'t open the output file\n");
                exit(-1);
            }
            for (int i = 0; i < 10; i++)
            {
                str_sz = sprintf(string, "%d: %d\n", i, numbers[i]);
                size = write(wfd, string, str_sz);
                if (size != str_sz)
                {
                    printf("third: Can\'t write all numbers\n");
                    exit(-1);
                }
            }
            if (close(wfd) < 0)
            {
                printf("third: Can\'t close the output file\n");
                exit(-1);
            }

            printf("third: exit, the number of digits is written to the output file\n");
        }
    }

    return 0;
}
