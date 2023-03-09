#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

const int buf_size = 5000;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Wrong number of command line arguments\n");
        exit(-1);
    }

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
    { // first process
        if (close(fd[0]) < 0)
        {
            printf("first: Can\'t close reading side of pipe\n");
            exit(-1);
        }

        // read from file
        FILE *myfile;
        myfile = fopen(argv[1], "r");
        if (myfile == NULL)
        {
            printf("Couldn't open the input file\n");
            exit(-1);
        }
        char text[buf_size];
        int i = 0;
        while ((text[i] = fgetc(myfile)) != EOF || i < buf_size)
        {
            i++;
        }
        text[i] = '\0';
        text[buf_size - 1] = '\0';
        fclose(myfile);

        size = write(fd[1], text, buf_size);
        if (size != buf_size)
        {
            printf("first: Can\'t write the whole string to pipe\n");
            exit(-1);
        }
        if (close(fd[1]) < 0)
        {
            printf("first: Can\'t close writing side of pipe\n");
            exit(-1);
        }
        printf("first: exit\n");
    }
    else
    { // second process
        result = fork();
        if (result < 0)
        {
            printf("second: Can\'t fork child\n");
            exit(-1);
        }
        else if (result > 0)
        {
            size = read(fd[0], str_buf, buf_size);
            if (size < 0)
            {
                printf("second: Can\'t read string from pipe\n");
                exit(-1);
            }

            if (close(fd[0]) < 0)
            {
                printf("second: Can\'t close reading side of pipe\n");
                exit(-1);
            }

            int numbers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            for (int i = 0; i < buf_size; i++)
            {
                if (str_buf[i] >= 48 && str_buf[i] <= 57)
                {
                    numbers[str_buf[i] - 48]++;
                }
            }

            size = write(fd[1], numbers, sizeof(int) * 10);
            if (size != sizeof(int) * 10)
            {
                printf("second: Can\'t write all numbers to pipe\n");
                exit(-1);
            }
            if (close(fd[1]) < 0)
            {
                printf("second: Can\'t close writing side of pipe\n");
                exit(-1);
            }

            printf("second: exit, the text is read\n");
        }
        else
        { // third process
            if (close(fd[1]) < 0)
            {
                printf("third: Can\'t close writing side of pipe\n");
                exit(-1);
            }
            int numbers[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

            size = read(fd[0], numbers, sizeof(int) * 10);
            if (size < 0)
            {
                printf("third: Can\'t read string from pipe\n");
                exit(-1);
            }

            if (close(fd[0]) < 0)
            {
                printf("third: Can\'t close reading side of pipe\n");
                exit(-1);
            }

            FILE *myfile;
            myfile = fopen(argv[2], "w");
            if (myfile == NULL)
            {
                printf("Couldn't open the output file\n");
                exit(-1);
            }
            for (int i = 0; i < 10; i++)
            {
                fprintf(myfile, "%d: %d\n", i, numbers[i]);
            }
            fclose(myfile);

            printf("third: exit, the number of digits is written to the output file\n");
        }
    }

    return 0;
}
