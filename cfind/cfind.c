

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

#include "statexpr.h"
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
                    }
                    else {
                        // removal successful
                        printf("Successfully removed %s\n", output[i]);
                    }
                    
                }
                else if (S_ISREG(s.st_mode)) {
                    // entry is a file
                    if (unlink(output[i]) != 0) {
                        // unlink unsuccessful
                        fprintf(stderr, "Failed to unlink %s.\n", output[i]);
                    }
                    else {
                        printf("Successfully unlinked %s\n", output[i]);
                    }
                }
            }
            
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
            else {
                // removal successful
                printf("Successfully removed %s\n", output[outputSize - 1]);
            }
            
        }
        else if (S_ISREG(s.st_mode)) {
            // entry is a file
            if (unlink(output[outputSize - 1]) != 0) {
                // unlink unsuccessful
                fprintf(stderr, "Failed to unlink %s.\n", output[outputSize - 1]);
            }
            else {
                printf("Successfully unlinked %s\n", output[outputSize - 1]);
            }
        }
    }
}

// removeElement removes element at index from entries
void removeElement (int index) {
	// move every element after index up one
    for (int i = index; i < size - 1; i++) {
        entries[i] = realloc(entries[i], sizeof(char)*strlen(entries[i+1])+1);
        strcpy(entries[i], entries[i+1]);
    }
    size--;
	// exit if list is depleted
	if (size == 0) {
		fprintf(stderr,"No matching files have been found!.\n");
		exit(EXIT_FAILURE);
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

void removeDotFiles(){
    for (int i = 0; i < size; i++){
        if (basename(entries[i])[0] == '.'){
            removeElement(i);
            i--;
        }
    }
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
	
	if (argc <= 0){
		//usage(1);
		opterr++;
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		fprintf(stderr,"A pathname is required.\n");
		exit(EXIT_FAILURE);
	}
	char *pathname = argv[0];

	// determine stat expression
	STAT_EXPRESSION statexp;
	if (argc == 2){
		statexp = compile_stat_expression(argv[1]);
	}
	else {
		statexp = compile_stat_expression("1");
	}


	
	
	// run a depth-first search starting at pathname
	dfs(pathname);
	// add on directory/file searched to end of list
	totalSize += (sizeof(char) * (strlen(pathname)+1));
	if (size == 0) {
		entries = malloc(totalSize);
	}
	else {
		entries = realloc(entries, totalSize);
	}
	entries[size] = malloc(sizeof(char)*(strlen(pathname) + 1));
	strcpy(entries[size], pathname);
	size++;

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

	// convert the full locations into just file and folder names
	for (int i = 0; i < size; i++) {
		// check stat expression
		struct stat thisEntry;
		if(stat(entries[i], &thisEntry) == 0) {
			if (evaluate_stat_expression(statexp, basename(entries[i]), &thisEntry)) {
				// entry matches stat expression, added to output
				output[outputSize] = malloc(sizeof(char)*strlen(entries[i])+1);
                strcpy(output[outputSize], entries[i]);
				outputSize++;
			}
		}
		else {
            // ADD ERROR MESSSAGE HERE!!!!!!!!!!!!!!!!!!!!!!!!!!
			printf("error at %i\n", i);
		}
		
        
	}
	free_stat_expression(statexp);
	
    
	// check if anything left in output after processing:
	if (outputSize == 0) {
		printf("Nothing found!\n");
		return 0;
	}
    
    // if printing count
    if (c == 1) {
        // print the count and nothing else
        printf("%i\n", outputSize);
        return 0;
    }
    
    // if unlinking
    if (u == 1){
        unlinkEntries();
        return 0;
    }
    
    // if printing list data
    if (l == 1){
        listInfo(output,outputSize);
        return 0;
    }
    
    // if sorting by size
    if (s == 1){
        qsort(output, outputSize, sizeof(char*), bySize);
    }
    
    // if sorting by time
    if (t == 1){
        qsort(output, outputSize, sizeof(char*), byTime);
    }
    
    // convert output to basenames
    for (int i = 0; i < outputSize; i++){
        long unsigned newSize = sizeof(char)* strlen(basename(output[i])+1);
        char *newOutput = basename(output[i]);
        output[i] = realloc(output[i], newSize);
        strcpy(output[i],newOutput);
    }
    
    if (s+t == 0){
        qsort(output, outputSize, sizeof(char*), alphabetical);
    }
    
    for (int i = 0; i < outputSize; i++) {
		printf("%s\n", basename(output[i]));
		
	}

	return 0;
}
