/*
 CITS2002 Project 2 2016
 Name(s):		Bradley Morgan , Robert Gross
 Student number(s):	21730745 , 20495129
 */

#include <getopt.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

#include "/cslinux/adhoc/include/statexpr.h"
#include "cfind.h"
#include "sort.h"

#define OPTLIST "acd:lrstu"

// entries contains all the filenames of the found files and folders
char **entries;
char **output;
// size is the number of elements in entries
int size = 0;
int outputSize = 0;

// totalSize is the size of the entries array, used for memory allocation
unsigned long totalSize = 1;

// r reverses sorting options, global variable is defined here
int r = 0;

//unlink
// unlink will be called if the -u option is specified
void unlinkEntries (void) {
    bool failed = false;
    if (outputSize > 1) {
        for (int i = outputSize - 2; i > -1; i--) {
            struct stat s;
            // check if directory or file
            if (stat(output[i], &s) == 0){
                if (S_ISDIR(s.st_mode)) {
                    // entry is a directory
                    if (rmdir(output[i]) != 0) {
                        // removal unsuccessful
                        fprintf(stderr, "Failed to remove %s\n", output[i]);
                        failed = true;
                    }
                    
                }
                else if (S_ISREG(s.st_mode)) {
                    // entry is a file
                    if (unlink(output[i]) != 0) {
                        // unlink unsuccessful
                        fprintf(stderr, "Failed to unlink %s.\n", output[i]);
                        failed = true;
                    }
                }
            }
            
        }
        if (failed){
            exit(EXIT_FAILURE);
        }
        else {
            exit(EXIT_SUCCESS);
        }
    }
    
    // unlink root
    // check if directory or file
    struct stat s;
    if (stat(output[outputSize - 1], &s) == 0){
        
        if (S_ISDIR(s.st_mode)) {
            // entry is a directory
            if (rmdir(output[outputSize - 1]) != 0) {
                // removal unsuccessful
                fprintf(stderr, "Failed to remove %s\n", output[outputSize - 1]);
            }
        }
        else if (S_ISREG(s.st_mode)) {
            // entry is a file
            if (unlink(output[outputSize - 1]) != 0) {
                // unlink unsuccessful
                fprintf(stderr, "Failed to unlink %s.\n", output[outputSize - 1]);
            }
        }
    }
}

// removeElement removes element at index from entries
void removeElement (int index) {
	// move every element after index up one
    for (int i = index; i < size - 1; i++) {
        entries[i] = realloc(entries[i], sizeof(char)*strlen(entries[i+1])+1);
        if (entries[i] == NULL) {
            fprintf(stderr, "Memory allocation failed. \n");
            exit(EXIT_FAILURE);
        }
        strcpy(entries[i], entries[i+1]);
    }
    size--;
	// exit if list is depleted
	if (size == 0) {
		fprintf(stderr,"No matching files have been found!.\n");
		exit(EXIT_FAILURE);
	}
}

// removeDotSlash removes "./" from the beginning of paths
void removeDotSlash(){
    for (int i = 0; i < outputSize; i++){
        if (output[i][0] == '.' && output[i][1] == '/') {
            output[i] += 2;
        }
    }
}

// filterDepth filters the list of entried by a specified depth
void filterDepth (int depth) {
	for (int i = 0; i < size; i++) {
		// iterate through whole list of entries
		char *thisEntry = entries[i];
		int thisDepth = -1;

		while (*thisEntry != '\0') {
			// determine which depth this entry lies at
			if (*thisEntry == '/') {
				thisDepth++;
			}
			thisEntry++;
		}
		
		// if depth of this entry exceeds specified depth, remove it
		if (thisDepth > depth) {
			removeElement(i);
			i--;
		}
	}
}

// removeDotFiles removes files that begin with '.' from output
void removeDotFiles(){
    for (int i = 0; i < size; i++){
        if (basename(entries[i])[0] == '.'){
            removeElement(i);
            i--;
        }
    }
    return;
}

// usage outputs how to use cfind if too few arguments are provided
void usage(){
    printf("usage: cfind [options] pathname [stat-expression]\n");
    printf("\t-a: consider all entries, including system files beginning with '.' (but excluding '.' and '..' directories\n");
    printf("\t-c: print the number of files\n");
    printf("\t-d depth: limit depth of search to depth\n");
    printf("\t-l: print information about all found files\n");
    printf("\t-r: reverse order of any sorting options\n");
    printf("\t-s: sort file-entries by size\n");
    printf("\t-t: sort file-entries by modification time\n");
    printf("\t-u: unlink file-entires\n");
    return;
}

int main(int argc, char *argv[]) {
	int opt;
	int a = 0;
	int c = 0;
	int d = -1;
	int l = 0;
    int s = 0;
	int t = 0;
	int u = 0;
    int opterr = 0;

	while((opt = getopt(argc,argv, OPTLIST)) != -1){
		if (opt == 'a'){
			a = 1;
		}
		else if(opt == 'c'){
			c = 1;
		}		
		else if(opt == 'd'){
			d = atoi(optarg);
		}
		else if(opt == 'l'){
			l = 1;
		}
		else if(opt == 'r'){
			r = 1;
		}
		else if(opt == 's'){
			s = 1;
		}
		else if(opt == 't'){
			t = 1;
		}
		else if(opt == 'u'){
			u = 1;		
		}
		else{
			argc = -1;
		}
	}
    
 
	argc -= optind;
	argv += optind;

    /*
     if this is true, no path name was given. If no pathname was given but a stat expression was
     (for some strange reason), then the program will assume that the stat expression was the pathname
     */
    if (argc < 1){
        usage();
        opterr++;
        fprintf(stderr,"A pathname is required.\n");
        exit(EXIT_FAILURE);
    }
    
    //get pathname from arguments
	char *pathname = argv[0];

	// determine stat expression
	STAT_EXPRESSION statexp;
	if (argc == 2){
		statexp = compile_stat_expression(argv[1]);
	}
	else {
		statexp = compile_stat_expression("1");
	}


	
    struct stat pathStat;
    if(stat(pathname,&pathStat) == 0){
        //if file, add to entries
        if(S_ISREG(pathStat.st_mode)){
            totalSize += (sizeof(char) * (strlen(pathname)+1));
            if (size == 0) {
                entries = malloc(totalSize);
            }
            else {
                entries = realloc(entries, totalSize);
            }
            if (entries == NULL) {
                fprintf(stderr, "Memory allocation failed. \n");
                exit(EXIT_FAILURE);
            }
            entries[size] = malloc(sizeof(char)*(strlen(pathname) + 1));
            if (entries[size] == NULL) {
                fprintf(stderr, "Memory allocation failed. \n");
                exit(EXIT_FAILURE);
            }
            strcpy(entries[size], pathname);
            size++;
        }
        //else (i.e. it's a directory) run a depth-first search starting from pathname
        else{
            dfs(pathname);
        }
    }
    // remove files that begin with '.' if a flag is not set
    if (a != 1){
        removeDotFiles();
    }

	// apply depth constraints
	if (d != -1) {
		// a depth constraint has been specified
		filterDepth(d);
	}
    
	// output is an array containing all output information relative to requested parameters
	output = malloc(totalSize);
    if (output == NULL) {
        fprintf(stderr, "Memory allocation failed. \n");
        exit(EXIT_FAILURE);
    }

	// convert the full locations into just file and folder names
	for (int i = 0; i < size; i++) {
		// check stat expression
		struct stat thisEntry;
		if(stat(entries[i], &thisEntry) == 0) {
			if (evaluate_stat_expression(statexp, basename(entries[i]), &thisEntry)) {
				// entry matches stat expression, added to output
				output[outputSize] = malloc(sizeof(char)*strlen(entries[i])+1);
                if (output[outputSize] == NULL) {
                    fprintf(stderr, "Memory allocation failed. \n");
                    exit(EXIT_FAILURE);
                }
                strcpy(output[outputSize], entries[i]);
				outputSize++;
			}
		}
		else {
			printf("error at %i\n", i);
		}
		
        
	}
	free_stat_expression(statexp);
	
    
	// check if anything left in output after processing:
	if (outputSize == 0) {
		printf("Nothing found!\n");
		exit(EXIT_SUCCESS);
	}
    
    // check if pathname = '.' and, if so, remove "./" from the front of
    if (strcmp(pathname,".") == 0){
        removeDotSlash();
    }
    // if printing count
    if (c == 1) {
        // print the count and nothing else
        printf("%i\n", outputSize);
        exit(EXIT_SUCCESS);
    }
    
    // if unlinking
    if (u == 1){
        unlinkEntries();
        exit(EXIT_SUCCESS);
    }
    
    // if printing list data
    if (l == 1){
        listInfo(output,outputSize);
        exit(EXIT_SUCCESS);
    }
    
    // if sorting by size
    if (s == 1){
        mergesort(output, outputSize, sizeof(char*), bySize);
    }
    
    // if sorting by time
    if (t == 1){
        qsort(output, outputSize, sizeof(char*), byTime);
    }
    
    // if needing to sort alphabetically
    if (s+t == 0){
        mergesort(output, outputSize, sizeof(char*), alphabetical);
    }
    
    //print out final results
    for (int i = 0; i < outputSize; i++) {
		printf("%s\n", output[i]);
		
	}

    exit(EXIT_SUCCESS);
}
