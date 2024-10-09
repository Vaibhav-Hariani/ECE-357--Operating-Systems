#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

struct out_data {
        //Initializing as long ints: A filesystem with 2^8*4 bytes (4 gigabytes) is full well possible
    long int file_size;
    long int disk_blocks;
    int hardlinked;
    int failed_symlinks;
    //Assuming problematic if it's not alphanumeric or 
    int problematic_names;
    //Stores all the inodes consecutively; 
    int inodes[16];
};

int IsAscii(char* str){
    for(int i = 0; i < strlen(str); i++){
        //Used an ascii table, assumed that values within these bounds would be reasonable
        if( "!" <= str[i] < "~"){
             return -1;
        }
    }
    return 1;
}

int descend_dir(char* path, struct out_data* data){
    DIR *dirp;
    struct dirent *de;

    if (!(dirp=opendir(path))){
    fprintf(stderr,"Can not open directory %s:%s\\n",path,strerror(errno));
    return -1;
    }
    while (de = readdir(dirp)) {
        char* name = de -> d_name;
        int ascii = isAscii(name);
        if(ascii < 0) {
            data->problematic_names++;
        }
        //Stop the call for prior directory. Only check current & others.
        if(!strcmp(name, "..")) {
            continue;
        }
        struct stat st;
        int fd;
        stat(name,&st);
        if ( (st.st_mode & S_IFMT) == S_IFDIR){
            data -> disk_blocks += st.st_nlink - 2;
            if(strcmp(name, ".") != ".") {
                descend_dir(name, &data);
            }
        }
        printf("%s\n", name);
    } 

    if (errno) {
        fprintf(stderr,"Error reading directory %s:%s\\n",path,strerror(errno));
    }
    closedir(dirp);
    return 0;
}

int main(int argc, char **argv) {
    extern int errno;
    errno = 0;

    // Simple error handling if no arguments are specified
    if (argc == 1 || argv[1] ) {
        fprintf(stderr, "Error: No Arguments Specified\n");
        return -1;
    }
    struct out_data* rd = {0};
    char* starting_path =argv[1];


    descend_dir(argv[1],&rd);

}


