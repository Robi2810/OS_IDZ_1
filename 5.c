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
    int fifo_desc;
    int result;
    char* pipe_read = "prc.fifo";
    char* pipe_write = "pcw.fifo";
    char buffer[buf_size];
    ssize_t read_b;
    ssize_t written_b;

    mknod(pipe_read, S_IFIFO | 0666, 0);
    mknod(pipe_write, S_IFIFO | 0666, 0);

    result = fork();
    if (result < 0) {
        printf("Can\'t fork child\n");
        exit(-1);
    } else if (result > 0) {
        /* Reading Process */
        if((fifo_desc = open(pipe_read, O_WRONLY)) < 0){
            printf("[READ]: Can\'t open FIFO for writting\n");
            exit(-1);
        }
        printf("[READ]: Reading from file %s...\n", argv[1]);
        input = open(argv[1], O_RDONLY);
        if (input < 0) {
            printf("[READ]: Can\'t open file\n");
            exit(1);
        }
        read_b = read(input, buffer, buf_size);
        if (read_b > 0) {
            printf("[READ]: Writing to pipe %ld bytes\n", read_b);
            written_b = write(fifo_desc, buffer, read_b);
            if (written_b != read_b) {
                printf("[READ]: Can\'t write all string to pipe\n");
                exit(-1);
            }
        }
        close(input);
        close(fifo_desc);
        printf("[READ]: Finished job\n");
    } else {
        result = fork();
        if (result < 0) {
            printf("Can\'t fork child\n");
            exit(-1);
        } else if (result > 0) {
            /* Writing process */
            out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
            if (out < 0) {
                printf("[WRITE]: Can\'t create file\n");
                exit(1);
            }
            if((fifo_desc = open(pipe_write, O_RDONLY)) < 0){
                printf("[WRITE]: Can\'t open FIFO for reading\n");
                exit(-1);
            }
            printf("[WRITE]: Reading from pipe...\n");
            read_b = read(fifo_desc, buffer, buf_size);
            printf("[WRITE]: Writing to file %s %ld bytes\n", argv[2], read_b);
            written_b = write(out, buffer, read_b);
            close(out);
            close(fifo_desc);
            printf("[WRITE]: Finished job\n");
        } else {
            /* Processing process */
            if((fifo_desc = open(pipe_read, O_RDONLY)) < 0){
                printf("[PROC]: Can\'t open FIFO for reading\n");
                exit(-1);
            }
            read_b = read(fifo_desc, buffer, buf_size);
            printf("[PROC]: Processing string of %ld bytes...\n", read_b);
            for (int i = 0; i < read_b / 2; i++) {
                char tmp = buffer[i];
                buffer[i] = buffer[read_b - i - 1];
                buffer[read_b - i - 1] = tmp;
            }
            close(fifo_desc);
            if((fifo_desc = open(pipe_write, O_WRONLY)) < 0){
                printf("[PROC]: Can\'t open FIFO for writting\n");
                exit(-1);
            }
            printf("[PROC]: Writing to pipe %ld bytes\n", read_b);
            written_b = write(fifo_desc, buffer, read_b);
            close(fifo_desc);
            printf("[PROC]: Finished job\n");
        }
    }
    return 0;
}