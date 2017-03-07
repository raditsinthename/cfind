#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cfind.h"
#include "sort.h"

// alphabetical is implemented by qsort to alphabetically sort elements
int alphabetical (const void *a, const void *b) {
    const char **ca = (const char **)a;
    const char **cb = (const char **)b;
    
    if (r ==0){
        return strcmp(*ca, *cb);
    }
    else{
        return strcmp(*cb, *ca);
    }
}

// bySize is implemented by qsort to sort elements by size
int bySize (const void *a, const void *b){
    const char **ca = (const char **)a;
    const char **cb = (const char **)b;

    struct stat sa;
    struct stat sb;
    if(stat(*ca, &sa) == 0){
        if (stat(*cb, &sb) == 0){
            if (r == 0){
                return (int)sa.st_size - (int)sb.st_size;
            }
            else {
                return (int)sb.st_size - (int)sa.st_size;
            }
        }
        else{
            fprintf(stderr,"Error sorting by size.\n");
            exit(EXIT_FAILURE);
            return 0;
        }
    }
    else {
        fprintf(stderr,"Error sorting by size.\n");
        exit(EXIT_FAILURE);
        return 0;
    }
}

int byTime (const void *a, const void *b){
    const char **ca = (const char **)a;
    const char **cb = (const char **)b;
    
    struct stat sa;
    struct stat sb;
    if(stat(*ca, &sa) == 0){
        if (stat(*cb, &sb) == 0){
            if (r == 0){
                return (int)sa.st_mtime - (int)sb.st_mtime;
            }
            else {
                return (int)sb.st_mtime - (int)sa.st_mtime;
            }
        }
        else{
            fprintf(stderr,"Error sorting by time.\n");
            exit(EXIT_FAILURE);
            return 0;
        }
    }
    else {
        fprintf(stderr,"Error sorting by time.\n");
        exit(EXIT_FAILURE);
        return 0;
    }
}
