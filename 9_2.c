#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

const int buf_size = 200;
const int buf_size_2 = 5000;

int main(int argc, char ** argv) {
    int fin;
    int fout;
    int fifo_desc;
    int result;
    char* pipe_read_calc = "prc.fifo";
    char* pipe_calc_write = "pcw.fifo";
    char buffer[buf_size];
    char buffer_2[buf_size_2];
    ssize_t read_bytes;
    ssize_t written_bytes;

    mknod(pipe_read_calc, S_IFIFO | 0666, 0);
    mknod(pipe_calc_write, S_IFIFO | 0666, 0);

    
        if((fifo_desc = open(pipe_read_calc, O_RDONLY)) < 0){
            printf("[PROC]: Can\'t open FIFO for reading\n");
            exit(-1);
        }
	size_t size2 = 0;
	while((read_bytes = read(fifo_desc, buffer, buf_size)) > 0){
		memcpy(buffer_2 + size2, buffer, read_bytes);
		size2 += read_bytes;
	}
        printf("[PROC]: Processing string of %ld bytes...\n", size2);
        for (int i = 0; i < size2 / 2; i++) {
                char tmp = buffer_2[i];
                buffer_2[i] = buffer_2[size2 - i - 1];
                buffer_2[size2 - i - 1] = tmp;
        }
        close(fifo_desc);
        if((fifo_desc = open(pipe_calc_write, O_WRONLY)) < 0){
            printf("[PROC]: Can\'t open FIFO for writting\n");
            exit(-1);
        }
	int shit = 0;
	while(shit < size2){
		written_bytes = write(fifo_desc, buffer_2 + shit, size2);
		printf("[PROC]: Writing to pipe %ld bytes\n", written_bytes);
		shit += written_bytes;
	}
        close(fifo_desc);
        printf("[PROC]: Finished job\n");
    return 0;
}