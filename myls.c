// Pierre Drego 301301848

// Headers
#include "getFiles.h"

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

// Tracks information about our inputs and current directory
int gArgTracker = 1; // Tracks # of arg/Userinputted directories/options we're at
int gTotalDirectories = 0; // Total number of directories
int gTitleCount = 0; // Whether we display a header
bool gAbsoluteDirectory = 0; // Whether we're dealing with an absolute directory
bool gSkipTitleDueToRCount = 1; // Skip title due to recursion
int first = 0; // Don't make an extra space on the first print

char* gOurDirectory; // Current directory we're working with
char* gRDirectory; // Stores directory names for recursion

// Global information storage
// gStatArray,gNameArray and gDirectoryArray always refer to the same
// item given a location i, ex: gStatArray[i] is related to gNameArray[i]

char gNameArray[MAX_SIZE][MAX_ELE_SIZE]; // Stores name
struct stat gStatArray[MAX_SIZE][MAX_ELE_SIZE]; // Stores stat structs
int gStatTracker = 0; // Tracks length of array

char gDirectoryArray [MAX_SIZE][MAX_ELE_SIZE]; // Stores ordered names lexiographically
int gDirectoryTracker = 0;

char gEmptyArray [MAX_SIZE][MAX_ELE_SIZE]; // Stores empty folders we have to print later
int gEmptyArrayTracker = 0;

char argArrayDir [MAX_SIZE][MAX_ELE_SIZE]; // Stores args
int maxArgs = 0;

// Tracks given options
int gOLength = 0; // Length of options, ex: -il = 2
bool gICount = 0, gRCount = 0, gLCount = 0; //

// Resets all our globals regarding directories
void resetDirGlobals()
{
    gAbsoluteDirectory = 0;
}

// Resets all our global array values
void resetArrGlobals()
{
    // Tracks information about our inputs and current directory
    gStatTracker = 0;
    gDirectoryTracker = 0;

    // Clears arrays
    char blank[1] = "";
    memcpy(gStatArray, blank, sizeof(struct stat));
    memcpy(gDirectoryArray, blank, sizeof(char));
    memcpy(gNameArray, blank, sizeof(char));
}

// Gets the options from the user input
void getArgOptions(char** myArg)
{   
    gArgTracker = 1; // Skip ./myls and check the args after that
    char myTemp[MAX_SIZE] = "";

    while(myArg[gArgTracker] != NULL)
    {
        strcpy(myTemp,myArg[gArgTracker]);

        if(myTemp[0] != '-'){
            break;
        }
        else{
            char* tempPtr = myTemp;
            // Iterate for each character in the given string
            for(int i = 1; i < strlen(tempPtr); i++)
            {
                if((myTemp[i] == 'i') && (gICount == 0)){
                    gICount = 1;
                    gOLength++;
                }
                else if((myTemp[i] == 'l') && (gLCount == 0)){
                    gLCount = 1;
                    gOLength++;
                }
                else if((myTemp[i] == 'R') && (gRCount == 0)){
                    gRCount = 1;
                    gOLength++;
                }
                else{
                    printf("Incorrect input! %c\n", myTemp[i]);
                    //exit(0);
                }    
            }
        }
        gArgTracker++; // Check next arg
    }
}

// Gets the directories from the user input
int getArgDirectories()
{
    char cleanDirectory[MAX_ELE_SIZE];
    gAbsoluteDirectory = 0;

    if(argArrayDir[gArgTracker] == NULL){
        printf("GetArg Invalid directory: NULL\n\n");
    }
    else{
        strcpy(cleanDirectory, argArrayDir[gArgTracker]);

        // Makes sure given directory is valid
        if((argArrayDir[gArgTracker][0] == '.') && (argArrayDir[gArgTracker][1] == ('.' || '/'))){
            printf("'.' Not allowed!\n\n");
            gOurDirectory = NULL;
            exit(0);
            return 0;
        }
        else
        {
            if(argArrayDir[gArgTracker][0] == '/'){ // ex: /home
                gAbsoluteDirectory = 1;
            }
            gOurDirectory = argArrayDir[gArgTracker];
        }
    }
    
    // Ensures a directory is actually a directory
    DIR *dp = NULL;
    if((gOurDirectory != NULL) && ((dp = opendir(gOurDirectory)) == NULL)){
        //printf("Invalid directory: '%s'\n", gOurDirectory); // Wildcard makes this message act funny
        //exit(0);
    }
    else{
        closedir(dp);
        return 1;
    }
    return 0;
}

// Checks the number of valid directories
void checkNumValidArgDirectories(char** myArg){
    int startPoint = gArgTracker;

    // Ensures a directory is actually a directory
    while(myArg[startPoint] != NULL)
    { 
        DIR *dp = NULL;
        char theDir[MAX_ELE_SIZE*2], theCWD[MAX_ELE_SIZE];

        if(argArrayDir[startPoint][0] != '/'){
            getcwd(theCWD, sizeof(theCWD));
            sprintf(theDir, "%s/%s",theCWD,argArrayDir[startPoint]);
        }
        else{
            strcpy(theDir,argArrayDir[startPoint]);
        }

        if((dp = opendir(theDir)) == NULL){
            //printf("~~~checkNum Invalid directory %s\n", theDir);
            //exit(0);
        }
        else{
            closedir(dp);
            gTotalDirectories++;
        }
        startPoint++;
    }
    // To match ls format
    if ((maxArgs - gArgTracker) >= 2){
        gTitleCount++;
    }

}

// Translates all our given values into the proper format based on the
// options we were previously given
void translateDirectories()
{
    struct stat inoBuffer;

    int lenLink = 0, lenUName = 0, lenIno = 0,
             lenGName = 0,lenFSize = 0;

    // Get maxlength amongst buffers
    for(int i = 0; i < gStatTracker; i++)
    {
        char temp[MAX_ELE_SIZE] = "";
        char* tempPtr = NULL;

        memcpy(&inoBuffer, gStatArray[i], sizeof(struct stat));

        sprintf(temp, "%ld", inoBuffer.st_nlink);
        if(lenLink < strlen(temp)){
            lenLink = strlen(temp);
        }
        sprintf(temp, "%ld", inoBuffer.st_size);
        if(lenFSize < strlen(temp)){
            lenFSize = strlen(temp);
        }
        sprintf(temp, "%ld", inoBuffer.st_ino);
        if(lenIno < strlen(temp)){
            lenIno = strlen(temp);
        }

        tempPtr = getAndPrintUserName(inoBuffer.st_uid);
        if(lenUName < strlen(tempPtr)){
            lenUName = strlen(tempPtr);
        }
        tempPtr = getAndPrintGroup(inoBuffer.st_gid);
        if(lenGName < strlen(tempPtr)){
            lenGName = strlen(tempPtr);
        }       
    }

    // Translate directories into their proper form
    int rTitle = 0; // Recursive title, do we need to place one yet?
    for(int i = 0; i < gStatTracker; i++)
    {
        memcpy(&inoBuffer, gStatArray[i], sizeof(struct stat)); // gStatArray has the stat struct of every valid file we've gone through

        char* userName = getAndPrintUserName(inoBuffer.st_uid);
        char* groupName = getAndPrintGroup(inoBuffer.st_gid);

        long int numberLinks = inoBuffer.st_nlink;
        long int fileSize = inoBuffer.st_size;
        long int inoNumber = inoBuffer.st_ino;
        
        char myPermissions[MAX_ELE_SIZE] = "";
        evaluatePermissions(inoBuffer, &myPermissions[0]);

        // Adds apostrophes as needed
        char fileName[MAX_ELE_SIZE] = "";
        strcpy(fileName,gNameArray[i]);
        checkNameHasSpace(&fileName[0]);

        // Ensures valid localtime else it will crash, happens at ~385 iterations in /bin
        char theTime[MAX_ELE_SIZE];
        if(localtime(&inoBuffer.st_mtime)){
            strftime(theTime, sizeof(theTime), "%h %-2d %-4Y %02H:%02M", localtime(&inoBuffer.st_mtime));
        }
        else{
            //exit(0);
        }
        
        // gOLength is the length of the permissions, this is fairly straightforward
        // gDirectoryArray as previously stated tracks the ordered form of all the files
        // it will be bubbleSorted directly after this or already has been
        if(gOLength == 0)
        {
            sprintf(gDirectoryArray[gDirectoryTracker++] ,"%s\n",fileName);
        }
        else if(gOLength == 1)
        {
            if(gICount == 1){
                sprintf(gDirectoryArray[gDirectoryTracker++] ,"%*ld %s\n", lenIno, inoNumber,  fileName);
            }
            if(gLCount == 1){
                sprintf(gDirectoryArray[gDirectoryTracker++] ,"%s %*ld %-*s %-*s %*ld %s %s\n", myPermissions, lenLink, numberLinks,
                        lenUName, userName, lenGName, groupName, lenFSize, fileSize, theTime, fileName);
            }
            if(gRCount == 1){
                if((rTitle == 0) && (gSkipTitleDueToRCount == 0)){ // Do we add a header to the output?
                    checkNameHasSpace(gRDirectory);
                    printf("\n%s:\n", gRDirectory);
                    removeApostrophes(gRDirectory);
                    rTitle++;
                }
                sprintf(gDirectoryArray[gDirectoryTracker++] ,"%s\n",fileName);
            }
        }
        else if(gOLength == 2)
        {
            if((gICount == 1) && (gRCount == 1)){
                if((rTitle == 0) && (gSkipTitleDueToRCount == 0)){
                    checkNameHasSpace(gRDirectory);
                    printf("\n%s:\n", gRDirectory);
                    removeApostrophes(gRDirectory);
                    rTitle++;
                }
                sprintf(gDirectoryArray[gDirectoryTracker++] ,"%*ld %s\n", lenIno, inoNumber,  fileName);
            }
            if((gICount == 1) && (gLCount == 1)){
                sprintf(gDirectoryArray[gDirectoryTracker++] ,"%*ld %s %*ld %-*s %-*s %*ld %s %s\n", lenIno, inoNumber, myPermissions, 
                              lenLink, numberLinks, lenUName, userName, lenGName, groupName, lenFSize, fileSize, theTime, fileName);
            }
            if((gRCount == 1) && (gLCount == 1)){
                if((rTitle == 0) && (gSkipTitleDueToRCount == 0)){
                    checkNameHasSpace(gRDirectory);
                    printf("\n%s:\n", gRDirectory);
                    removeApostrophes(gRDirectory);
                    rTitle++;
                }
                sprintf(gDirectoryArray[gDirectoryTracker++] ,"%s %*ld %-*s %-*s %*ld %s %s\n", myPermissions, lenLink, numberLinks, 
                        lenUName, userName, lenGName, groupName, lenFSize, fileSize, theTime, fileName);
            }
        }
        else if(gOLength == 3)
        {
            if((gICount == 1) && (gRCount == 1) && (gLCount == 1)){
                if((rTitle == 0) && (gSkipTitleDueToRCount == 0)){
                    checkNameHasSpace(gRDirectory);
                    printf("\n%s:\n", gRDirectory);
                    removeApostrophes(gRDirectory);
                    rTitle++;
                }
                sprintf(gDirectoryArray[gDirectoryTracker++] ,"%*ld %s %*ld %-*s %-*s %*ld %s %s\n", lenIno, inoNumber, myPermissions, 
                              lenLink, numberLinks, lenUName, userName, lenGName, groupName, lenFSize, fileSize, theTime, fileName);
            }
        }
        
    }
    gSkipTitleDueToRCount = 0;
    
}

// Checks for symbolic links, and links them
void storeDirectoriesHelper(struct stat inoBuffer,struct dirent* pointerDP)
{
    if(S_ISLNK(inoBuffer.st_mode)){
        char tempBuffer[MAX_ELE_SIZE];
        char thePath[MAX_ELE_SIZE*2];

        // Get CWD and full path
        if(gAbsoluteDirectory == 0){
            getcwd(tempBuffer, sizeof(tempBuffer));
            sprintf(thePath, "%s/%s/%s",tempBuffer,gOurDirectory,pointerDP->d_name);
        }
        else{
            sprintf(thePath, "%s/%s",gOurDirectory,pointerDP->d_name);
        }

        // Get link from the path
        int pathLen = 0;
        pathLen = readlink(thePath, tempBuffer, sizeof(tempBuffer)-1);

        // If valid null terminate string, reformat and copy it to our pointerDP
        if(pathLen != -1){
            tempBuffer[pathLen] = '\0';
            sprintf(thePath, "%s -> %s", pointerDP->d_name, tempBuffer);
            strcpy(pointerDP->d_name, thePath);
        }        
    }
}

// Stores directory information into their respective array
void storeDirectories(struct stat inoBuffer,struct dirent* pointerDP)
{
    if(gLCount == 1){
        storeDirectoriesHelper(inoBuffer, pointerDP);
    }
    
    memcpy(gNameArray[gStatTracker], pointerDP->d_name, sizeof(struct dirent));
    memcpy(gStatArray[gStatTracker++], &inoBuffer, sizeof(struct stat));
}

// Sorts directories, translates them based on their options into a
// proper readable format, then outputs them
void displayAppropriate()
{
    translateDirectories(); // Translates directories
    bubbleSort(); // Sorts arrays

    for(int i = 0; i < gDirectoryTracker; i++){
        printf("%s", gDirectoryArray[i]);
    }
}

// Checks for symbolic links, and links them
void rStoreDirectoriesHelper(struct stat* inoBuffer,char* theName)
{
    if(S_ISLNK(inoBuffer->st_mode)){
        char tempBuffer[MAX_ELE_SIZE];
        char thePath[MAX_ELE_SIZE*2];

        // Get CWD and full path
        if(gAbsoluteDirectory == 0){
            getcwd(tempBuffer, sizeof(tempBuffer));
            sprintf(thePath, "%s/%s/%s",tempBuffer,gRDirectory,theName);
        }
        else{
            sprintf(thePath, "%s/%s",gRDirectory,theName);
        }
        // Get link from the path
        int pathLen = 0;
        pathLen = readlink(thePath, tempBuffer, sizeof(tempBuffer)-1);

        // If valid null terminate string, reformat and copy it to our pointerDP
        if(pathLen != -1){
            tempBuffer[pathLen] = '\0';
            sprintf(thePath, "%s -> %s", theName, tempBuffer);
            strcpy(theName, thePath);
        }  
    }
}

// Stores directories for recursion
void storeDirectoriesR(struct stat inoBuffer[MAX_ELE_SIZE], char theName[MAX_ELE_SIZE])
{
    if(gLCount == 1){
        rStoreDirectoriesHelper(inoBuffer, theName);
    }
    
    // Together
    strcpy(gNameArray[gStatTracker], theName);
    memcpy(gStatArray[gStatTracker++], inoBuffer, sizeof(struct stat));
}

// Returns an ordered set of directories so that we can iterate through them in
// the correct order
char* getBestDirectory(char myDir[], int printTheTitle)
{
    
    // Tracks directories
    char unsortedDirectories[MAX_SIZE][MAX_ELE_SIZE];
    struct stat static unsortedDirectoriesStat[MAX_SIZE][MAX_ELE_SIZE];
    int numDirectories = 0;

    // Combines both directories and files then sorts them
    char unsortedEverything[MAX_SIZE][MAX_ELE_SIZE];
    struct stat static unsortedEverythingStat[MAX_SIZE][MAX_ELE_SIZE];
    int numEverything = 0;

    // Pointers
    DIR *dp = NULL;
    struct dirent *pDp = NULL;

    // Iterates through directories as needed
    if((dp = opendir(myDir)) == NULL){
        printf("---Best Cannot open Input directory '%s'\n",myDir);
        return "fail";
        //exit(1);
    }
    else{
        // While directory isn't invalid, continue reading
        while((pDp = readdir(dp)) != 0)
        {   
            struct stat inoBuf;
            char homeDirectory[MAX_ELE_SIZE];

            if(pDp->d_name[0] != '.')
            {
                sprintf(homeDirectory, "%s/%s", myDir, pDp->d_name);
                
                if(lstat(homeDirectory, &inoBuf) == -1){ // Supposed to be -1
                    printf("----Failed at Recursive Dir \"%s\"\n", homeDirectory);
                    strcpy(gEmptyArray[gEmptyArrayTracker++], myDir);
                }
                else{
                    if(S_ISDIR(inoBuf.st_mode) && (!(S_ISLNK(inoBuf.st_mode)))){
                        memcpy(unsortedDirectoriesStat[numDirectories], &inoBuf,sizeof(struct stat));
                        memcpy(unsortedDirectories[numDirectories++],pDp->d_name,sizeof(pDp->d_name));
                    }
                    else{
                        memcpy(unsortedEverythingStat[numEverything],&inoBuf,sizeof(struct stat));
                        memcpy(unsortedEverything[numEverything++],pDp->d_name,sizeof(pDp->d_name));
                    }
                } 
            }               
        }
        closedir(dp);
    }

    // Store title name
    gRDirectory = myDir;

    // Stores empty arrays
    if((numEverything == 0) && (printTheTitle == 1)){   
        strcpy(gEmptyArray[gEmptyArrayTracker++], myDir);
    }

    // Copy values so we can sort everything together
    for(int i = 0; i < numDirectories; i++){
        strcpy(unsortedEverything[numEverything], unsortedDirectories[i]);
        memcpy(unsortedEverythingStat[numEverything++], unsortedDirectoriesStat[i], sizeof(struct stat));
    }

    // Stores directories they'll actually be sorted later
    for(int i = 0; i < numEverything; i++){
        storeDirectoriesR(unsortedEverythingStat[i], unsortedEverything[i]);
    }

    // Displays directories
    displayAppropriate();
    resetArrGlobals();

    char temp[MAX_ELE_SIZE*2];
    sprintf(temp,"%s",myDir);

    char* bestSort = temp;
    return bestSort;
}

void getRecursiveDirectories(char myDir[])
{
    char tempBuffer[MAX_ELE_SIZE], cwd[MAX_ELE_SIZE*2];

    if (getcwd(tempBuffer, sizeof(tempBuffer)) == NULL){
        printf("WEird error \n");
    }

    if(!gAbsoluteDirectory)
    {
        sprintf(cwd, "%s/%s", tempBuffer, myDir);

        if(fileExists(cwd) != 1){
            //printf("AFile does not exist, exiting %s\n\n", cwd);
            //exit(0);
        }
    }
    else
    {
        // sprintf(cwd, "%s%s", tempBuffer, myDir);
        if(fileExists(myDir) != 1){
            printf("Dir does not exist, exiting %s\n\n", myDir);
            //exit(0);
        }
    }

    // Pointers
    DIR *dp = NULL;
    struct dirent *pDp = NULL;

    // Iterates through directories as needed
    if((dp = opendir(myDir)) == NULL){
        printf("------Recursion Cannot open Input directory \"%s\"\n",myDir);
        //exit(0);
    }
    else{
        // While valid directory
        while((pDp = readdir(dp)) != 0)
        {   
            struct stat inoBuf;
            char homeDirectory[MAX_ELE_SIZE*2];

            if(pDp->d_name[0] != '.')
            {
                sprintf(homeDirectory, "%s/%s", myDir, pDp->d_name);
                
                if(lstat(homeDirectory, &inoBuf) == -1){ // Supposed to be -1
                    printf("Error accessing '%s'\n", homeDirectory);
                }
                else{
                    if(S_ISDIR(inoBuf.st_mode) && (!(S_ISLNK(inoBuf.st_mode))))
                    {   
                        char temp[MAX_ELE_SIZE] = "";
                        // Removes extra / if needed from end of string
                        if(myDir[strlen(myDir) -1] == '/'){
                            sprintf(temp,"%s%s",myDir, pDp->d_name);
                        }
                        else{
                            sprintf(temp,"%s/%s",myDir, pDp->d_name);
                        }
                        strcpy(temp,getBestDirectory(temp, 1));
                        getRecursiveDirectories(temp);
                    }
                } 
            }
            
        }
        closedir(dp);
        
    }
}

// Gets directories
int getDirectories(char myDir[])
{   
    int numDirectories = 0;
    gRDirectory = myDir;
    

    char tempBuffer[MAX_ELE_SIZE], cwd[MAX_ELE_SIZE*2];
    if(!gAbsoluteDirectory)
    {
        if (getcwd(tempBuffer, sizeof(tempBuffer)) == NULL){
            printf("getcwd error \n");
        }

        sprintf(cwd, "%s/%s", tempBuffer, myDir);

        if(fileExists(cwd) == 0){
            printf("File does not exist, exiting %s\n\n", cwd);
            //exit(0);
        }
    }
    else
    {
        if(fileExists(myDir) == 0){
            printf("Directory does not exist %s\n\n", cwd);
            //exit(0);
        }
        strcpy(cwd,myDir);
    }

    // Pointers
    DIR *dp = NULL;
    struct dirent *pDp = NULL;
    if((dp = opendir(myDir)) == NULL){
        if(fileExists(cwd) == 1){
            struct stat inoBuf2;
            char tempChar[MAX_ELE_SIZE];
            strcpy(tempChar, argArrayDir[gArgTracker]);

            if(lstat(cwd, &inoBuf2) == -1){
                printf("--Invalid directory for given file \"%s\"\n",myDir);
            }
            else
            {
                storeDirectoriesR(&inoBuf2, argArrayDir[gArgTracker]);
                gTitleCount = 0;
                gSkipTitleDueToRCount = 1;
            }
        }
        else{
            printf("--getDir Cannot open Input directory \"%s\"\n",myDir);
        }
        //exit(1);
    }
    else{
        // While directory isn't invalid, continue reading
        while((pDp = readdir(dp)) != 0)
        {   
            struct stat inoBuf;
            char homeDirectory[MAX_ELE_SIZE*2];

            if(pDp->d_name[0] != '.')
            {   
                sprintf(homeDirectory, "%s/%s", myDir, pDp->d_name);

                if(lstat(homeDirectory, &inoBuf) == -1){ // Supposed to be -1
                     printf("\nMinor error at getDir '%s'\n",homeDirectory);
                    //return -1;
                }
                else{
                    storeDirectories(inoBuf, pDp);
                    numDirectories++;
                }
            }
        }
        closedir(dp);
        
        checkNameHasSpace(&gRDirectory[0]);
        printTitle(myDir);
    }
    return numDirectories;
}

// Determines how to deal with input
void sortInput(int inputType, char tempBuffer[MAX_ELE_SIZE])
{
    if(inputType == 1)
    {
        getDirectories(gOurDirectory);
        displayAppropriate();
        resetArrGlobals();
        
        if(gRCount == 1){
            gTitleCount--;
            getRecursiveDirectories(gOurDirectory);
        }
    }
    else if(inputType == 2)
    {
        getBestDirectory(tempBuffer, 0);
    }
    else if(inputType == 3)
    {
        getBestDirectory(tempBuffer, 0);
        getRecursiveDirectories(".");
    }
    printEmpty();
}

// Sorts arguments directories
void sortArgs(char** myArg)
{
    maxArgs = gArgTracker;
    static char newArgs[MAX_SIZE][MAX_ELE_SIZE];

    // Iterate through all the args
    while(myArg[maxArgs] != NULL)
    {   
        strcpy(newArgs[maxArgs], myArg[maxArgs]);
        maxArgs++;
    }

    // Sort them O(n^2) bubble sort
    for(int i = gArgTracker; i < maxArgs-1; i++)
    {
        for(int j = gArgTracker; j < maxArgs-1; j++)
        {   
            if(strcmp(newArgs[j], newArgs[j+1]) > 0)
            {
                bubbleSwap(newArgs[j], newArgs[j+1]);
            }
        }
    }

    // Copy it over
    for(int i = gArgTracker; i < maxArgs; i++){
        strcpy(argArrayDir[i],newArgs[i]);
    }

    resetArrGlobals();
}

int main(int numArgs, char* args[])
{
    //system("@cls||clear"); // Clear terminal screen

    resetArrGlobals();

    getArgOptions(args); // Get options -R -i
    sortArgs(args);

    int initialize = 1;
    char tempBuffer[MAX_ELE_SIZE];
    
    getcwd(tempBuffer, sizeof(tempBuffer));
    
    getArgDirectories(); // Get directories
    checkNumValidArgDirectories(args); // Count number of directories

    while (gArgTracker < maxArgs)
    {
// THIS IS ONLY NECESSARY FOR WILDCARD OPERATOR ***************
        // if(initialize == 1) // Get initial directory
        // { 
            // char tempBuffer2[MAX_ELE_SIZE*2];
            // sprintf(tempBuffer2, "%s/%s/..", tempBuffer,gOurDirectory);
            // if(strcmp(getBestDirectory(tempBuffer2, 0), "fail\n") == 0)
            // {

            // }
            // else
            // {
            //     if (gRCount == 1){
            //         gTitleCount--;
            //     }
            //     initialize++;
            // }
            
        // }
        // else
        // {
//-- END THIS IS ONLY NECESSARY FOR WILDCARD OPERATOR ***************
            getArgDirectories();
            sortInput(1, tempBuffer); // Deals with our input
            gArgTracker++;
            initialize++;
        // }
    }

    // For empty statements, ls <blank>
    if((initialize == 1) && (gRCount == 0)){ // -R flag not set
        sortInput(2, tempBuffer); // Deals with our input
    }   
    else if((initialize == 1) && (gRCount == 1)){
        sortInput(3, tempBuffer); // Deals with our input
    }

    return 0;
}
