#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

typedef struct {                                        // Struct der indeholder en fil og dens index nummer
    int number;
    char filename[256];
} FileIndex;

int getFileData(char *fileName);            
int getDirectoryData(FileIndex *files);

int main(void) {

    FileIndex files[1024];                              // Et array af structs med filnavn og nummer

    int numberOfFiles = getDirectoryData(files);        // Antallet af filer i en mappe

    for (int i = 0; i < numberOfFiles; i++)
    {
        printf("Number: %d - Filename: %s\n", files[i].number, files[i].filename);
    }
    
    int userInput = 0;
    printf("Type the number of the file you want to see: \n");
    scanf(" %d", &userInput);

    for (int i = 0; i < numberOfFiles; i++)
    {
        if(userInput == files[i].number) {
            getFileData(files[i].filename);
            printf("Filename 1: %s", files[i].filename);
        }
    }

    return 0;
}

int getFileData(char *fileName) {
    
    FILE *fptr;
    char fileToOpen[256];

    strcpy(fileToOpen, "./data/");
    strcat(fileToOpen, fileName);
    fptr = fopen(fileToOpen, "r");

    if(fptr == NULL) {
        printf("Error opening file");
    }

    char line[1024];

    while(fgets(line, sizeof(line), fptr)) {
        printf("%s", line);
        }
    
    fclose(fptr);

    return 0;
}

int getDirectoryData(FileIndex *files) {

    DIR *directory;
    struct dirent *entry;
    int fileNumber = 0;

    directory = opendir("./data");

    if(directory == NULL) {
        printf("Error opening folder");
        return 1;
    }

    while ((entry = readdir(directory)) != NULL) {
        // Check if entry is a regular file and ends with ".csv"
        if (entry->d_type == DT_REG) {
            // Find the length of the filename
            int len = strlen(entry->d_name);

            // Check if the file has a ".csv" extension
            if (len > 4 && strcmp(entry->d_name + len - 4, ".csv") == 0) {
                files[fileNumber].number = fileNumber + 1;

                // Corrected strncpy: 
                strncpy(files[fileNumber].filename, entry->d_name, sizeof(files[fileNumber].filename) - 1);
                files[fileNumber].filename[sizeof(files[fileNumber].filename) - 1] = '\0';

                printf("Added: %s\n", files[fileNumber].filename);
                fileNumber++;
            }
        }
    }


    if(closedir(directory) == -1) {
        printf("Error closing directory.\n");
        return 1;
    }

    return fileNumber;
}