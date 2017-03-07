#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>

#include "cfind.h"

void listInfo(char **paths, int pathsSize){
    struct stat s;
    struct group *grp;
    struct passwd *pwd;
    
    for (int i = 0; i < pathsSize; i++){
        if(stat(paths[i], &s)==0){
            //File inode
            printf("%ld ",(long)s.st_ino);
            
            //File Permissions
            printf( (S_ISDIR(s.st_mode)) ? "d" : "-");
            printf( (s.st_mode & S_IRUSR) ? "r" : "-");
            printf( (s.st_mode & S_IWUSR) ? "w" : "-");
            printf( (s.st_mode & S_IXUSR) ? "x" : "-");
            printf( (s.st_mode & S_IRGRP) ? "r" : "-");
            printf( (s.st_mode & S_IWGRP) ? "w" : "-");
            printf( (s.st_mode & S_IXGRP) ? "x" : "-");
            printf( (s.st_mode & S_IROTH) ? "r" : "-");
            printf( (s.st_mode & S_IWOTH) ? "w" : "-");
            printf( (s.st_mode & S_IXOTH) ? "x" : "-");
            printf(" ");
            
            //Number of Links
            printf("%d ",s.st_nlink);
            
            //Owners Name
            grp = getgrgid(s.st_gid);
            printf("%s ", grp->gr_name);
            
            //Group-Owners Name
            pwd = getpwuid(s.st_uid);
            printf("%s ", pwd->pw_name);
            
            //Size
            printf("%llu ", s.st_size);
            
            //Modification Time
            printf("%s ", ctime(&s.st_mtime));
            
            //Path Name
            char actualpath [PATH_MAX+1];
            char *ptr;
            ptr = realpath(paths[i],actualpath);
            printf("%s ", actualpath);
            
            //New Line
            printf("\n");            
        }
    }
    exit(EXIT_SUCCESS);
    return;
}
