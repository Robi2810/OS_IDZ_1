#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

const int buf_size = 5000;

int main(int argc, char ** argv) {
    if (argc < 3) {
        printf("Usage: main <input> <output>\n");
        exit(0);
    }
    int input;
    int out;
    int result;
    int pipe_read[2];
    int pipe_write[2];
    char buffer[buf_size];
    ssize_t read_b;
    ssize_t written_b;

    if (pipe(pipe_read) < 0) {
        printf("Can\'t open pipe\n");
        exit(-1);
    }
    if (pipe(pipe_write) < 0) {
        printf("Can\'t open pipe\n");
        exit(-1);
    }

    result = fork();
    if (result < 0) {
        printf("Can\'t fork child\n");
        exit(-1);
    } else if (result > 0) {
        /* Reading Process */
        close(pipe_read[0]);
        close(pipe_write[1]);
        printf("[READ]: Reading from file %s...\n", argv[1]);
        input = open(argv[1], O_RDONLY);
        if (input < 0) {
            printf("[READ]: Can\'t open file\n");
            exit(1);
        }
        read_b = read(input, buffer, buf_size);
        if (read_b > 0) {
            printf("[READ]: Writing to pipe %ld bytes\n", read_b);
            written_b = write(pipe_read[1], buffer, read_b);
            if (written_b != read_b) {
                printf("[READ]: Can\'t write all string to pipe\n");
                exit(-1);
            }
        }
        close(input);
        close(pipe_read[1]);
        printf("[READ]: Finished job\n");
	out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
            if (out < 0) {
                printf("[WRITE]: Can\'t create file\n");
                exit(1);
            }
            printf("[WRITE]: Reading from pipe...\n");
            read_b = read(pipe_write[0], buffer, buf_size);
            printf("[WRITE]: Writing to file %s %ld bytes\n", argv[2], read_b);
            written_b = write(out, buffer, read_b);
            close(out);
            close(pipe_write[0]);
            printf("[WRITE]: Finished job\n");
	close(pipe_write[0]);
    } else {
            /* Processing process */
            close(pipe_read[1]);
            close(pipe_write[0]);
            read_b = read(pipe_read[0], buffer, buf_size);
            printf("[PROC]: Processing string of %ld bytes...\n", read_b);
            for (int i = 0; i < read_b / 2; i++) {
                char tmp = buffer[i];
                buffer[i] = buffer[read_b - i - 1];
                buffer[read_b - i - 1] = tmp;
            }
            printf("[PROC]: Writing to pipe %ld bytes\n", read_b);
            written_b = write(pipe_write[1], buffer, read_b);
            close(pipe_read[1]);
            close(pipe_read[0]);
            printf("[PROC]: Finished job\n");
    }
    return 0;
}