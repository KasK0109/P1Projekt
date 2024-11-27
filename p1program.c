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

int getLinesFromFile(char *fileName, PowerStruct powerStructs[]);
int getDirectoryData(FileIndex *files);
void printStruct(PowerStruct power[], int powerArrayLength);
double calcArrayAverage(double array[], int length);
void printDir(FileIndex files[], int fileArrayLength);
int getFileLength(FileIndex files[], int fileArrayLength, int input, PowerStruct powerStructs[]);
void runStandartProgram(FileIndex files[],PowerStruct powerStructs[]);

int main(void)
{
    FileIndex files[1024];
    PowerStruct powerStructs[2048];
    int UserMenuInput;

    printf("Hello welcome to program menu please select:\n");
    printf("1 - view csv data files\n");
    printf("2 - edit data\n");
    scanf(" %d", &UserMenuInput);

    switch (UserMenuInput)
    {
    case 1:
        runStandartProgram(files,powerStructs);
        break;
    case 2:
        eidtData(files,powerStructs);
        break;
    
    default:
        break;
    }


    return 0;
}

/// @brief Print all the data in a file within the `./data/` directory.
/// @param fileName
/// @return a status code?
int getLinesFromFile(char *fileName, PowerStruct powerStructs[])
{
    char fileToOpen[256];

    strcpy(fileToOpen, "./data/"); // `./data/`
    strcat(fileToOpen, fileName);  // `./data/<fileName>`

    FILE *fptr = fopen(fileToOpen, "r");
    if (fptr == NULL) { // happens if file does not exist
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int index = 0;

    while (fgets(line, sizeof(line), fptr)) {
        PowerStruct powerStruct;
        int result = fscanf(fptr, " %lf %lf %lf", &powerStruct.GRID, &powerStruct.SUSTAIN, &powerStruct.USAGE);
        if(result != 3) {
            break;
        } 

        powerStructs[index] = powerStruct;
        index++;
    }

    fclose(fptr); // Close the file

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
        if (entry->d_type == DT_REG) {
            // Find the length of the filename
            int len = strlen(entry->d_name);

            // Check if the file has a ".csv" extension
            if (len > 4 && strcmp(entry->d_name + len - 4, ".csv") == 0) {
                files[fileNumber].number = fileNumber + 1;

                // Corrected strncpy:
                strncpy(files[fileNumber].filename, entry->d_name, sizeof(files[fileNumber].filename) - 1);
                files[fileNumber].filename[sizeof(files[fileNumber].filename) - 1] = '\0';

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

void printStruct(PowerStruct power[], int powerArrayLength) {
    // Get the values

    double grid;
    double sustain;
    double usage;

    double gridArray[powerArrayLength];
    double sustainArray[powerArrayLength];
    double usageArray[powerArrayLength];

    printf("-------------------------------------------------\n");
    printf("|                      DATA                     |\n");
    printf("-------------------------------------------------\n");

    for (int i = 0; i < powerArrayLength; i++) {
        grid = power[i].GRID;
        sustain = power[i].SUSTAIN;
        usage = power[i].USAGE;

        gridArray[i] = grid;
        sustainArray[i] = sustain;
        usageArray[i] = usage;

        if (grid <= 1.0) {
            printf("\n\033[31m GRID: OUTAGE\033[0m"); // Rød tekst
        } else {
            printf("\n\033[32m GRID: %6.2lf\033[0m", grid); // Grøn tekst
        }

        printf("| \033[34mSUSTAINABLE: %6.2lf\033[0m", sustain); // Blå tekst
        printf("| \033[36mUSAGE: %6.2lf\033[0m", usage); // Cyan tekst
    }   

    printf("\n");
    double gridAverage = calcArrayAverage(gridArray, powerArrayLength);
    double sustainAverage = calcArrayAverage(sustainArray, powerArrayLength);
    double usageAverage = calcArrayAverage(usageArray, powerArrayLength);

    printf("-------------------------------------------------\n");
    printf("|                    AVERAGES                   |\n");
    printf("-------------------------------------------------\n");

    if (gridAverage <= 1.0) {
        printf("\033[31m GRID: OUTAGE\033[0m"); // Rød tekst
    } else {
        printf("\033[32m GRID: %6.2lf\033[0m", gridAverage); // Grøn tekst
    }

    printf("| \033[34mSUSTAINABLE: %6.2lf\033[0m", sustainAverage); // Blå tekst
    printf("| \033[36mUSAGE: %6.2lf\033[0m", usageAverage); // Cyan tekst
    printf("\n");
}

double calcArrayAverage(double array[], int length) {

    double sum = 0;
    double average = 0;

    for (int i = 0; i < length; i++)
    {
        sum += array[i];
    }

    return average = sum / length;    
}

void printDir(FileIndex files[], int fileArrayLength) {
    for (int i = 0; i < fileArrayLength; i++){
        printf("Number: %d - Filename: %s\n", files[i].number, files[i].filename);
    }
}

int getFileLength(FileIndex files[], int fileArrayLength, int input, PowerStruct powerStructs[]) {

    for (int i = 0; i < fileArrayLength; i++){
        if (input == files[i].number){
                return getLinesFromFile(files[i].filename, powerStructs);
        }
    }
}

void runStandartProgram(FileIndex files[],PowerStruct powerStructs[]){
    
            int numberOfFiles = getDirectoryData(files);        // Antallet af filer i en mappe
            printDir(files, numberOfFiles);                     // Print filnavne og numre

            int userInput = 0;
            printf("Type the number of the file you want to see: \n");
            scanf("%d", &userInput);

            // get data of file that the user wrote out, if it exists
            int powerLength = getFileLength(files, numberOfFiles, userInput, powerStructs);

            printStruct(powerStructs, powerLength);
            printf("\n");
}


void eidtData(FileIndex files[],PowerStruct powerStructs[]){
    int numberOfFiles = getDirectoryData(files);        // Antallet af filer i en mappe
    printDir(files, numberOfFiles);                     // Print filnavne og numre

    int userInput = 0;
    printf("Type the number of the file you want to see: \n");
    scanf("%d", &userInput);



}