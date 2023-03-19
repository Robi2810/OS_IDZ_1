#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

const int buf_size = 200;

int main(int argc, char ** argv) {
    if (argc < 3) {
        printf("Usage: main <input> <output>\n");
        exit(0);
    }
    int fin;
    int fout;
    int fifo_desc;
    int fifo_desc_2;
    int result;
    char* pipe_read_calc = "prc.fifo";
    char* pipe_calc_write = "pcw.fifo";
    char buffer[buf_size];
    char buffer_2[buf_size];
    ssize_t read_bytes;
    ssize_t written_bytes;

    mknod(pipe_read_calc, S_IFIFO | 0666, 0);
    mknod(pipe_calc_write, S_IFIFO | 0666, 0);

        /* Reading Process */
        if((fifo_desc = open(pipe_read_calc, O_WRONLY)) < 0){
            printf("[READ+WRITE]: Can\'t open FIFO for writting\n");
            exit(-1);
        }
	
        printf("[READ+WRITE]: Reading from file %s...\n", argv[1]);
        fin = open(argv[1], O_RDONLY);
	fout = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
        if (fin < 0) {
            printf("[READ+WRITE]: Can\'t open file\n");
            exit(1);
        }
	while((read_bytes = read(fin, buffer, buf_size)) > 0){
            printf("[READ+WRITE]: Writing to pipe %ld bytes\n", read_bytes);
            written_bytes = write(fifo_desc, buffer, read_bytes);
            if (written_bytes != read_bytes) {
                printf("[READ+WRITE]: Can\'t write all string to pipe\n");
                exit(-1);
            }
	    
	}
        
        close(fin);
        close(fifo_desc);
        printf("[READ+WRITE]: Finished job\n");
	if((fifo_desc_2 = open(pipe_calc_write, O_RDONLY)) < 0){
            printf("[READ+WRITE]: Can\'t open FIFO for reading\n");
            exit(-1);
        }
	while((read_bytes = read(fifo_desc_2, buffer, buf_size)) > 0){
        printf("[READ+WRITE]: Writing to file %s %ld bytes\n", argv[2], read_bytes);
        written_bytes = write(fout, buffer, read_bytes);
	}
        close(fout);
	close(fifo_desc_2);
        printf("[READ+WRITE]: Finished job\n");

    return 0;
}