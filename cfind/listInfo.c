/*
 CITS2002 Project 2 2016
 Name(s):		Bradley Morgan , Robert Gross
 Student number(s):	21730745 , 20495129
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
            printf("  ");
            
            //Number of Links
            printf("%d ",s.st_nlink);
            
            //Group-Owners Name
            pwd = getpwuid(s.st_uid);
            printf("%s ", pwd->pw_name);
            
            //Owners Name
            grp = getgrgid(s.st_gid);
            printf("%s ", grp->gr_name);
            
            //Size
            printf("%*llu", 8,s.st_size);
            printf(" ");
            
            //Modification Time
            printf("%s ", strtok(ctime(&s.st_mtime),"\n"));
            
            //Path Name
            printf("%s ", paths[i]);
            
            //New Line
            printf("\n");            
        }
    }
    exit(EXIT_SUCCESS);
    return;
}
