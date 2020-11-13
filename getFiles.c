// Name studentNumber //** GIT privacy EDIT

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
extern int first;

extern bool gICount, gRCount, gLCount; 
extern char* gOurDirectory;

extern char gEmptyArray [MAX_SIZE][MAX_ELE_SIZE];
extern char gNameArray[MAX_SIZE][MAX_ELE_SIZE]; // Stores name
extern char gDirectoryArray [MAX_SIZE][MAX_ELE_SIZE]; // Stores ordered names lexiographically
extern struct stat gStatArray[MAX_SIZE][MAX_ELE_SIZE]; // Stores stat structs


// Checks if a file exists, returns 1 if file exists, 2 if directory does
int fileExists(char* thePath)
{
    // Does file exist
    FILE* myfile;
    if ((myfile = fopen(thePath, "r"))){
        fclose(myfile);
        return 1;
    }

    // Does directory exist and can we open it
    DIR *dp = NULL;
    if ((dp = opendir(thePath)) == NULL){
        closedir(dp);
        return 2;
    }
        
    return 0;
}

// Determines the permissions for a given stat datatype, returns myPerms
void evaluatePermissions(struct stat myStat, char* myPerms)
{
    mode_t thePerm = myStat.st_mode;

    // Set all bits to -
    for(int i = 0; i < 10; i++){
        myPerms[i] = '-';
    }

    // Set the permission bits
    if(S_ISDIR(thePerm)){
        myPerms[0] = 'd';
    }
    if(S_ISLNK(thePerm)){
        myPerms[0] = 'l';
    }
    if(thePerm & S_IRUSR){
        myPerms[1] = 'r';
    }  
    if(thePerm & S_IWUSR){
        myPerms[2] = 'w';
    }    
    if(thePerm & S_IXUSR){
        myPerms[3] = 'x';
    }    
    if(thePerm & S_IRGRP){
        myPerms[4] = 'r';
    }    
    if(thePerm & S_IWGRP){
        myPerms[5] = 'w';
    }    
    if(thePerm & S_IXGRP){
        myPerms[6] = 'x';
    }    
    if(thePerm & S_IROTH){
        myPerms[7] = 'r';
    }    
    if(thePerm & S_IWOTH){
        myPerms[8] = 'w';
    }  
    if(thePerm & S_IXOTH){
        myPerms[9] = 'x';
    }
}

// Gets Username, from Brian Fraser
char* getAndPrintUserName(uid_t uid)
{
    struct passwd *pw = getpwuid(uid);

    if (pw) {
        //printf("The user ID %u -> %s\n", uid, pw->pw_name);
        return pw->pw_name;
    } else {
        //perror("Hmm not found???");
        //printf("No name found for %u\n", uid);
        return "";
    }
}
// Gets group name, from Brian Fraser
char* getAndPrintGroup(gid_t grpNum)
{
    struct group *grp = getgrgid(grpNum);

    if (grp) {
        //printf("The group ID %u -> %s\n", grpNum, grp->gr_name);
        return grp->gr_name;
    } else {
        //printf("No group name for %u found\n", grpNum);
        return "";
    }
}


// bubbleSort helper; Swaps variables of type stat
void statBubbleSwap(struct stat a[MAX_ELE_SIZE], struct stat b[MAX_ELE_SIZE])
{
    struct stat temp[MAX_ELE_SIZE];
    memcpy(temp,a,sizeof(struct stat));
    memcpy(a,b,sizeof(struct stat));
    memcpy(b,temp,sizeof(struct stat));
}
// bubbleSort helper; swaps variables of type char
void bubbleSwap(char a[MAX_ELE_SIZE], char b[MAX_ELE_SIZE])
{
    char temp[MAX_ELE_SIZE];
    strcpy(temp,a);
    strcpy(a,b);
    strcpy(b,temp);
}

// Sorts arrays O(n^2), only for non recursive directories
void bubbleSort()
{
    for(int i = 0; i < gStatTracker-1; i++)
    {
        for(int j = 0; j < gStatTracker-1; j++)
        {   
            // Swaps EVERY array since they all have related values
            // and changing one would influence all of them
            if(strcmp(gNameArray[j], gNameArray[j+1]) > 0)
            {
                bubbleSwap(gNameArray[j], gNameArray[j+1]);
                bubbleSwap(gDirectoryArray[j], gDirectoryArray[j+1]);
                statBubbleSwap(gStatArray[j], gStatArray[j+1]);
            }
        }
    }
}



// If name has a space or another special char in it put apostrophes around it
void checkNameHasSpace(char* fileName){
    char tempBuf[MAX_ELE_SIZE+2] = "";
    int fileLength = strlen(fileName);
    bool apostrophe = 1;

    for(int i = 0; i < fileLength; i++)
    {
        //  spaces, !, $, ',' , ^, &, (, )  ---> and '?'
        if(fileName[i] == ' ' || fileName[i] == '!' || fileName[i] == '$' || fileName[i] == '\'' || fileName[i] == '?' ||
        fileName[i] == ',' || fileName[i] == '^' || fileName[i] == '&' || fileName[i] == '(' || fileName[i] == ')')
        {
            if(fileName[0] == '\''){ // We don't want double apostrophes
                break;
            }
            for(int j = 0; j < fileLength+2; j++){
                if((apostrophe) && (tempBuf[j-4] == ' ') && (tempBuf[j-3] == '-') && 
                                  (tempBuf[j-2] == '>') && (tempBuf[j-1] == ' '))
                {
                    tempBuf[j-4] = '\'';
                    tempBuf[j-3] = ' ';
                    tempBuf[j-2] = '-';
                    tempBuf[j-1] = '>';
                    tempBuf[j] = ' ';
                    fileLength += 3;
                    apostrophe = 0;       
                }

                if(j == 0){
                    tempBuf[0] = '\'';
                }
                else if((j == (fileLength+1)) && (apostrophe)){
                    tempBuf[fileLength+1] = '\'';
                }
                else if(!apostrophe){
                    tempBuf[j+1] = fileName[j-2];
                }
                else{
                    tempBuf[j] = fileName[j-1];
                }
            }
            strcpy(fileName, tempBuf);
            break;
        }
        
    }
}

// Remove apostrophes from filename
void removeApostrophes(char* fileName){
    char tempBuf[MAX_ELE_SIZE+2] = "";
    int fileLength = strlen(fileName);

    if((fileName[0] == '\'') && (fileName[fileLength-1] == '\''))
    {
        for(int i = 0; i < fileLength-2; i++){
            tempBuf[i] = fileName[i+1];
        }
        strcpy(fileName,tempBuf);
    }
}


// Prints 'title'
void printTitle(char* theDir)
{
    // Checks if we need to print the title yet then prints it if possible
    if(gTitleCount <= 0){
        gTitleCount++;
        if(gRCount != 1){
            printf("\n");
        }
    }
    else{
        if((gOurDirectory[strlen(gOurDirectory)] == '/')){ // ex: home/
            char cleanDirectory[MAX_ELE_SIZE];
            strcpy(cleanDirectory,gOurDirectory);

            cleanDirectory[(strlen(cleanDirectory)) - 1] = 0; // Remove '/'
        }
        else{
            if(first == 0){
                first++;
                printf("%s:\n",gOurDirectory);
            }
            else
            {
                printf("\n%s:\n",gOurDirectory);
            }
        }
    } 
}

// Prints empty folders
void printEmpty()
{
    int originalLength = gEmptyArrayTracker;
    gEmptyArrayTracker = 0;
    // Bubble sorts values O(n^2)
    for(int i = 0; i < originalLength-1; i++)
    {
        for(int j = 0; j < originalLength-1; j++)
        {      
            if(strcmp(gEmptyArray[j], gEmptyArray[j+1]) > 0)
            {
                bubbleSwap(gEmptyArray[j], gEmptyArray[j+1]);
            }
        }
    }

    for(int i = 0; i < originalLength; i++){
        checkNameHasSpace(&gEmptyArray[0][0]);
        printf("\n%s:\n", gEmptyArray[i]);
    }

    for(int i = 0; i < originalLength; i++){
        strcpy(gEmptyArray[i],"");
    }
}

