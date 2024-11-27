#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

void handler(int signo) {
    fprintf(stderr, "Signal %d Recieved \n", signo);
    exit(signo);
}

// Want to clarify this function name is a joke
int sigsegv_generator() {
    fprintf(stderr, "Executing Test #1 (write to r/o mmap): \n");

    for (int i = 0; i < 32; i++) {
        signal(i, handler);
    }

    // Test #1: PROT_READ
    int* c = mmap(0, 5 * sizeof(int), PROT_READ, MAP_ANONYMOUS, -1, 0);
    // Writing to this region

    fprintf(stderr, "Writing int(8) to protected region \n");
    *c = 8;

    if (*c == 8) {
        fprintf(stderr, "Write sucessful \n");
        return 0;
    }
    fprintf(stderr, "Write Failed: No Error \n");
    return 255;
}

// Weird bugs with this, TODO: Ask james
int map_shared(int fd) {
    fprintf(stderr, "Executing Test #2 (write to MAP_SHARED): \n");
    char* c = mmap(0, 5 * sizeof(char), PROT_READ | PROT_WRITE,
                   MAP_FILE | MAP_SHARED, fd, 0);

    fprintf(stderr, "Writing 'l' to MAP_SHARED: \n");
    c[0] = 'l';
    char buf[5];
    int numread;
    fprintf(stderr, "Reading from MAP_SHARED: \n");
    read(fd, buf, numread);
    if (buf[0] == 'l') {
        fprintf(stderr, "Read the byte correctly. \n");
        return 0;
    }
    fprintf(stderr, "Read Failed. \n");
    return 1;
}

int map_private(int fd) {
    fprintf(stderr, "Executing Test #3 (write to MAP_PRIVATE): \n");
    char* c = mmap(0, sizeof(char), PROT_READ | PROT_WRITE,
                   MAP_FILE | MAP_PRIVATE, fd, 0);
    fprintf(stderr, "Writing 'l' to MAP_PRIVATE: \n");
    c[0] = 'l';
    char buf[5];
    int numread;
    fprintf(stderr, "Reading from MAP_PRIVATE: \n");
    read(fd, buf, numread);
    if (buf[0] == 'l') {
        fprintf(stderr, "Read the byte correctly. \n");
        return 0;
    }
    fprintf(stderr, "Read Failed. \n");
    return 1;
}

int holey_moley(int fd, int offset, int size) {
    fprintf(stderr, "Executing Test #4 (write to hole): \n");
    char* c = mmap(0, size, PROT_READ | PROT_WRITE,
                   MAP_FILE | MAP_SHARED, fd, offset);
    char buf[4101];
    int nread = 4101;
    fprintf(stderr, "Reading file to verify \n");
    int l = read(fd,buf,nread);
    if(l < nread) {
        return 0;
    }
    for(int i = 0; i < 4101; i++){
        if(buf[i] != 0) {
            return 0;
        }
    }
    c = 'X';
    lseek(fd,16, SEEK_CUR);
    *(c + 16) = "A";
    int new_read = read(fd,buf,16);
    if(buf[0] != "X") {
        fprintf(stderr, "Could not read byte X in test 4\n");
        return 0;

    }
    
    return 0;
}

int main(int argc, char** argv) {
    printf("Number of args: %d \n", argc);
    int fd;

    if (argc == 1) {
        fprintf(stderr, "No Argument Specified: \n");
        return 1;
    }
    switch (argv[1][0]) {
        case '1':
            return sigsegv_generator();
            break;
        case '2':
            // Assume that a file descriptor is passed
            fprintf(stderr, "Missing File to open: creating a testfile. \n");
            fd = open("testfile", O_RDWR | O_CREAT | O_TRUNC, 0666);
            ftruncate(fd, 10);
            return map_shared(fd);
            break;

        case '3':
            // Assume that a file descriptor is passed
            fprintf(stderr, "Missing File to open: creating a testfile. \n");
            fd = open("testfile", O_RDWR | O_CREAT | O_TRUNC, 0666);
            ftruncate(fd, 10);
            return map_private(fd);
            break;
        case '4':
            fd = open("testfile", O_RDWR | O_CREAT | O_TRUNC, 0666);
            // ftruncate(fd, 4101);
            holey_moley(fd,4101,8192);
        default:
            fprintf(stderr, "Invalid Argument \n");
            return 0;
            break;
    }
    return 0;
}