// Pierre Drego 301301848

#ifndef _GETFILES_H_
#define _GETFILES_H_
// Basic
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <errno.h>

// Tolower
#include <ctype.h>

// Getgrgid, stat
#include <grp.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// getpwuid
#include <pwd.h>

// Directories
#include <dirent.h>

// Time
#include <time.h>

#define MAX_SIZE 3000
#define MAX_ELE_SIZE 500

extern int gEmptyArrayTracker;
extern int gStatTracker;
extern int gTitleCount; // Whether we display a header

extern bool gICount, gRCount, gLCount; 
extern char* gOurDirectory;

extern char gEmptyArray [MAX_SIZE][MAX_ELE_SIZE];
extern char gNameArray[MAX_SIZE][MAX_ELE_SIZE]; // Stores name
extern char gDirectoryArray [MAX_SIZE][MAX_ELE_SIZE]; // Stores ordered names lexiographically
extern struct stat gStatArray[MAX_SIZE][MAX_ELE_SIZE]; // Stores stat structs


// Checks if a file exists, returns 1 if it does
int fileExists(char* thePath);

// Determines the permissions for a given stat datatype, returns myPerms
void evaluatePermissions(struct stat myStat, char* myPerms);

// Gets Username, from Brian Fraser
char* getAndPrintUserName(uid_t uid);
// Gets group name, from Brian Fraser
char* getAndPrintGroup(gid_t grpNum);


// Sorts arrays O(n^2), only for non recursive directories
void bubbleSort();
// bubbleSort helper; swaps variables of type char
void bubbleSwap(char a[MAX_ELE_SIZE], char b[MAX_ELE_SIZE]);
// bubbleSort helper; Swaps variables of type stat
void statBubbleSwap(struct stat a[MAX_ELE_SIZE], struct stat b[MAX_ELE_SIZE]);

// If name has a space in it put apostrophes around it
void checkNameHasSpace(char* fileName);
// Remove apostrophes from filename
void removeApostrophes(char* fileName);

// Prints empty folders
void printEmpty();
// Prints 'title'
void printTitle(char* theDir);





#endif 