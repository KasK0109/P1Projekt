#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

typedef struct {                                        // Struct der indeholder en fil og dens index nummer
    int number;
    char filename[256];
} FileIndex;

typedef struct {
    double GRID;
    double SUSTAIN;
    double USAGE;
} PowerStruct;

int printFileData(char *fileName, PowerStruct powerStructs[]);
int getDirectoryData(FileIndex *files);
void printStruct(PowerStruct power);

int main(void)
{
    FileIndex files[1024];
    PowerStruct powerStructs[2048];

    int numberOfFiles = getDirectoryData(files);        // Antallet af filer i en mappe

    for (int i = 0; i < numberOfFiles; i++)
    {
        printf("Number: %d - Filename: %s\n", files[i].number, files[i].filename);
    }

    int userInput = 0;
    printf("Type the number of the file you want to see: \n");
    scanf("%d", &userInput);

    // get data of file that the user wrote out, if it exists
    int powerLength = 0;
    for (int i = 0; i < numberOfFiles; i++)
    {
        if (userInput == files[i].number)
        {
            powerLength = printFileData(files[i].filename, powerStructs);
        }
    }
    
    for (int i = 0; i < powerLength; i++)
    {
        printStruct(powerStructs[i]);
    }
    

    return 0;
}

/// @brief Print all the data in a file within the `./data/` directory.
/// @param fileName
/// @return a status code?
int printFileData(char *fileName, PowerStruct powerStructs[])
{
    char fileToOpen[256];

    strcpy(fileToOpen, "./data/"); // `./data/`
    strcat(fileToOpen, fileName);  // `./data/<fileName>`

    FILE *fptr = fopen(fileToOpen, "r");
    if (fptr == NULL) // happens if file does not exist
    {
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int index = 0;

    while (fgets(line, sizeof(line), fptr))
    {
        PowerStruct powerStruct;
        int result = fscanf(fptr, " %lf %lf %lf", &powerStruct.GRID, &powerStruct.SUSTAIN, &powerStruct.USAGE);
        if(result != 3) {
            break;
        } 
        powerStructs[index] = powerStruct;
        index++;
    }

    fclose(fptr);

    return index;
}

/// @brief Fetch directory data, relative to running directory. Also prints information as it goes.
/// @param files
/// @return The number of files
int getDirectoryData(FileIndex *files)
{
    DIR *directory;
    struct dirent *entry;
    int fileNumber = 0;

    directory = opendir("./data");

    if (directory == NULL)
    {
        printf("Error opening folder");
        return 1;
    }

    while ((entry = readdir(directory)) != NULL)
    {
        // Check if entry is a regular file and ends with ".csv"
        if (entry->d_type == DT_REG)
        {
            // Find the length of the filename
            int len = strlen(entry->d_name);

            // Check if the file has a ".csv" extension
            if (len > 4 && strcmp(entry->d_name + len - 4, ".csv") == 0)
            {
                files[fileNumber].number = fileNumber + 1;

                // Corrected strncpy:
                strncpy(files[fileNumber].filename, entry->d_name, sizeof(files[fileNumber].filename) - 1);
                files[fileNumber].filename[sizeof(files[fileNumber].filename) - 1] = '\0';

                printf("Added: %s\n", files[fileNumber].filename);
                fileNumber++;
            }
        }
    }

    if (closedir(directory) == -1)
    {
        printf("Error closing directory.\n");
        return 1;
    }

    return fileNumber;
}

void printStruct(PowerStruct power) {
    // Get the values
    double grid = power.GRID;
    double sustain = power.SUSTAIN;
    double usage = power.USAGE;

    if(grid <= 1.0) {
        printf("\n GRID: OUTAGE");
    } else {
        printf("\n GRID: %6.2lf", grid);
    }
    printf("| SUSTAINABLE: %6.2lf", sustain);
    printf("| USAGE: %6.2lf", usage);    
}