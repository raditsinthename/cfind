/*
 CITS2002 Project 2 2016
 Name(s):		Bradley Morgan , Robert Gross
 Student number(s):	21730745 , 20495129
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cfind.h"

/* dfs will run a depth-first search of directories
 starting at dirName */
void dfs(char *dirName){
    DIR *dir;
    struct dirent *ent;
    
    dir = opendir(dirName);
    // check directory is legal
    if (dir != NULL) {
        ent = readdir(dir);
        // while there are further directories to traverse
        while (ent != NULL) {
            char *dirNameOld = malloc( strlen(dirName) + 1);
            if (dirNameOld == NULL) {
                fprintf(stderr, "Memory allocation failed for directory name");
            }
            strcpy(dirNameOld, dirName);
            // create name of further directories
            char *thisDir = ent->d_name;
            // another directory has been found, traversing:
            // if not traversing up through directories
            if (strcmp(thisDir, "..") != 0 && strcmp(thisDir, ".") != 0) {
                struct stat s;
                strcat(dirName, "/");
                strcat(dirName, thisDir);
                // check if directory or file
                if (stat(dirName, &s) == 0){
                    totalSize += (sizeof(char) * (strlen(dirName)+1));
                    // allocating memory to entries
                    // memory allocation to first entry
                    if (size == 0) {
                        entries = malloc(totalSize);
                    }
                    // memory allocation to each new entry
                    else {
                        entries = realloc(entries, totalSize);
                    }
                    if(entries == NULL) {
                        // if failed to allocate memory
                        fprintf(stderr,"Memory allocation failed while reading %s.\n", dirName);
                    }
                    // memory allocation to specific index inside entry list
                    entries[size] = malloc(sizeof(char)*(strlen(dirName) + 1));
                    if(entries[size] == NULL) {
                        // if failed to allocate memory
                        fprintf(stderr,"Memory allocation failed while allocating at index %i.\n", size);
                    }
                    // copy the current entry into entries
                    strcpy(entries[size], dirName);
                    size++;
                }
                // traverse recursively
                dfs(dirName);
            }
            
            strcpy(dirName, dirNameOld);
            //free memory for dirNameOld as it's no longer needed
            free(dirNameOld);
            //reread the directory
            ent = readdir(dir);
        }
        closedir(dir);
    }
    else{
        fprintf(stderr,"%s: Permission denied\n", dirName);
    }
}
