#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>


typedef struct out_data {
        //Initializing as long ints: A filesystem with 2^8*4 bytes (4 gigabytes) is full well possible
    long unsigned int total_file_size;
    long unsigned int total_disk_blocks;
    int num_hardlinks;
    int num_failed_symlinks;
    //Assuming problematic if it's not alphanumeric or 
    int n_problematic_names;
    //Stores all the inodes consecutively; 
    int inodes[16];
    //Making this super large so I don't have to rebuild it ever
    int seen_hardlinks[5000];
    int size_cap;
} data;

int IsAscii(char* str){
    for(int i = 0; i < strlen(str); i++){
        //Used an ascii table, assumed that values within these bounds would be reasonable
        if( '!' <= str[i] || str[i] >= '~' || str[i] == '/' ){
             return -1;
        }
    }
    return 1;
}

int in_array(int key, int* array, int array_size){
    for(int i = 0; i < array_size; i++){
        if(array[i] == key){
            return 1;
        }
    }
    return -1;
}

int descend_dir(char* path, struct out_data* data){
    DIR *dirp;
    struct dirent *de;
    extern int errno;
    errno = 0;

    if (!(dirp=opendir(path))){
    fprintf(stderr,"Can not open directory %s:%s \n",path,strerror(errno));
    errno = 0;
    return -1;
    }


    while (de = readdir(dirp)) {
        char* name = de -> d_name;
        //Stop the call for prior directory. Only check current & others.
        if(strcmp(name, "..") == 0 || strcmp(name, ".") == 0) {
            continue;
        }
        //2 for null character & slash
        char pathname[strlen(path) + 2 +strlen(name)];
        strcpy(pathname,path);
        strcat(pathname,"/");
        strcat(pathname,name);

        // //Testing code, comment out before submission
        // printf("Reached File %s\n", pathname);

        int ascii = IsAscii(name);
        if(ascii < 0) {
            data->n_problematic_names++;
        }
        
        struct stat st = {0};
        int fd;
        lstat(pathname,&st);
        int mode = (st.st_mode&S_IFMT);
        //Converts mode to a 4 bit number, for the inode table
        int indexed_mode = mode >> 12;
        //Add inode type
        data -> inodes[indexed_mode]++;
        switch (mode) {
            case S_IFDIR:
                int i = descend_dir(pathname, data);
                break;
            case S_IFREG:
                if(st.st_nlink > 1){
                    // printf("Found Hardlinked File %s \n", name);
                    if(in_array(st.st_ino,data -> seen_hardlinks, data -> num_hardlinks) < 0 ) {
                        //If the element is not in the array, 
                        if(data -> num_hardlinks < data ->size_cap) {
                            data -> seen_hardlinks[data -> num_hardlinks] = st.st_ino;
                            data -> num_hardlinks ++;
                            data -> total_disk_blocks += st.st_blocks;
                            data -> total_file_size += st.st_size;
                        } else {
                            printf("Oops, too many hardlinks");
                        }
                    } else {
                        printf("File %s referenced prior \n", name);
                    }
                } else {
                        data -> total_disk_blocks += st.st_blocks;
                        data -> total_file_size += st.st_size;
                }
                break;
            case S_IFLNK:
                struct stat st_tmp = {0};
                if(stat(pathname,&st_tmp) < 0) {
                    errno = 0;
                    data -> num_failed_symlinks ++;
                }
                break;
        }
        if (errno) {
            fprintf(stderr,"Error reading object %s: %s \n",name,strerror(errno));
            errno = 0;
        }
    } 
    if (errno) {
        fprintf(stderr,"Error while reading directory %s: %s \n",path,strerror(errno));
        errno = 0;
        return -1;
    }
    closedir(dirp);
    return 0;
}

int main(int argc, char **argv) {
    extern int errno;
    errno = 0;

    // Simple error handling if no arguments are specified
    if (argc == 1) {
        fprintf(stderr, "Error: No Arguments Specified\n");
        return -1;
    }

    char* starting_path =argv[1];
    printf("Got arg: %s \n", starting_path);
    data rd = {0};
    rd.size_cap = 5000;


    descend_dir(argv[1],&rd);
    //Leaving the outdated inodes out: This can easily be modified to display them
    char inode_labels[16][25] = {"","Named Pipes", "Character Devices", "", "Directories", "", "Block Devices", "", "Regular Files", 
                                "", "Symlinks", "", "Network Sockets", "", "", ""};
    for(int i = 0; i < 16; i++){
        if(strlen(inode_labels[i]) > 0){
            printf("# of %s: %d\n", inode_labels[i],rd.inodes[i]);
        }
    }
    printf("Regular File Sum: %lu \n", rd.total_file_size);
    printf("Regular File Block Sum: %lu \n", rd.total_disk_blocks);
    printf("# of Hardlinked Files: %d \n", rd.num_hardlinks);
    printf("# of Failed Symlinks: %d \n", rd.num_failed_symlinks);
    printf("# of \"problematic\" pathnames: %d \n", rd.n_problematic_names);


    return 0;

}


