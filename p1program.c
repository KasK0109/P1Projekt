#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

typedef struct
{ // Struct der indeholder en fil og dens index nummer
    int number;
    char filename[256];
} FileIndex;

typedef struct
{
    double GRID;
    double SUSTAIN;
    double USAGE;
} PowerStruct;

int getLinesFromFile(char *fileName, PowerStruct powerStructs[]);
int getDirectoryData(FileIndex *files);
void printStruct(PowerStruct power[], int powerArrayLength);
double calcArrayAverage(double array[], int length);
void printDir(FileIndex files[], int fileArrayLength);
char *getFileName(FileIndex files[], int fileArrayLength, int input);
int getFile(char *fileName, PowerStruct powerStructs[]);
int runStandardProgram(FileIndex files[], PowerStruct powerStructs[]);
int editData(FileIndex files[], PowerStruct powerStructs[]);

int main(int argc, char *argv[])
{
    int UserMenuInput;
    // test
    if (argv[1] != NULL)
    {
        int number = atoi(argv[1]);
        printf("User input received: %d\n", number);
        UserMenuInput = number;
    }
    else
    {
        printf("Hello welcome to program menu please select:\n");
        printf("1 - view csv data files\n");
        printf("2 - edit data\n");
        printf("> ");
        scanf(" %d", &UserMenuInput);
    }

    FileIndex files[1024];
    PowerStruct powerStructs[2048];

    switch (UserMenuInput)
    {
    case 1:
        return runStandardProgram(files, powerStructs);
    case 2:
        return editData(files, powerStructs);
    default:
        return -1;
    }
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
    if (fptr == NULL)
    { // happens if file does not exist
        exit(EXIT_FAILURE);
    }

    char line[1024];
    int index = 0;

    // note: sscanf scans the line starting from the first line.
    // if any line is empty, it stops collecting data,
    // which means that the first line in the data file should not be empty
    while (fgets(line, sizeof(line), fptr))
    {
        PowerStruct nextDataPoint;
        int result = sscanf(line, " %lf %lf %lf", &nextDataPoint.GRID, &nextDataPoint.SUSTAIN, &nextDataPoint.USAGE);
        if (result != 3)
        {
            // data point was empty (imagine an empty line at the start of the file)
            // if we did not do this, powerStructs would have a value of `0.0 0.0 0.0` at this index.
            continue;
        }

        powerStructs[index] = nextDataPoint;
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

void printStruct(PowerStruct power[], int powerArrayLength)
{
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

    for (int i = 0; i < powerArrayLength; i++)
    {
        grid = power[i].GRID;
        sustain = power[i].SUSTAIN;
        usage = power[i].USAGE;

        gridArray[i] = grid;
        sustainArray[i] = sustain;
        usageArray[i] = usage;

        if (grid <= 1.0)
        {
            printf("\n\033[31m GRID: OUTAGE\033[0m"); // Rød tekst
        }
        else
        {
            printf("\n\033[32m GRID: %6.2lf\033[0m", grid); // Grøn tekst
        }

        printf("| \033[34mSUSTAINABLE: %6.2lf\033[0m", sustain); // Blå tekst
        printf("| \033[36mUSAGE: %6.2lf\033[0m", usage);         // Cyan tekst
    }

    printf("\n");
    double gridAverage = calcArrayAverage(gridArray, powerArrayLength);
    double sustainAverage = calcArrayAverage(sustainArray, powerArrayLength);
    double usageAverage = calcArrayAverage(usageArray, powerArrayLength);

    printf("-------------------------------------------------\n");
    printf("|                    AVERAGES                   |\n");
    printf("-------------------------------------------------\n");

    if (gridAverage <= 1.0)
    {
        printf("\033[31m GRID: OUTAGE\033[0m"); // Rød tekst
    }
    else
    {
        printf("\033[32m GRID: %6.2lf\033[0m", gridAverage); // Grøn tekst
    }

    printf("| \033[34mSUSTAINABLE: %6.2lf\033[0m", sustainAverage); // Blå tekst
    printf("| \033[36mUSAGE: %6.2lf\033[0m", usageAverage);         // Cyan tekst
    printf("\n");
}

double calcArrayAverage(double array[], int length)
{
    double sum = 0;
    double average = 0;

    for (int i = 0; i < length; i++)
    {
        sum += array[i];
    }

    return average = sum / length;
}

void printDir(FileIndex files[], int fileArrayLength)
{
    for (int i = 0; i < fileArrayLength; i++)
    {
        printf("Number: %d - Filename: %s\n", files[i].number, files[i].filename);
    }
}

/// @brief Choose a file from a user input
/// @param files
/// @param fileArrayLength
/// @param input
/// @return The name of the file from its number
char *getFileName(FileIndex files[], int fileArrayLength, int userInput)
{
    for (int i = 0; i < fileArrayLength; i++)
    {
        if (userInput == files[i].number)
        {
            return files[i].filename;
        }
    }
    return NULL;
}

/// @brief Writes an array of PowerStructs into memory from a file name.
/// @param fileName
/// @param powerStructs
/// @return Success code for writing to memory
int getFile(char *filename, PowerStruct powerStructs[])
{
    return getLinesFromFile(filename, powerStructs);
}

int runStandardProgram(FileIndex files[], PowerStruct powerStructs[])
{
    int numberOfFiles = getDirectoryData(files); // Antallet af filer i en mappe
    printDir(files, numberOfFiles);              // Print filnavne og numre

    int userInput = 0;
    printf("Type the number of the file you want to see:\n> ");
    scanf(" %d", &userInput);

    // get data of file that the user wrote out, if it exists
    // this writes to powerStructs in the process
    char *fileName = getFileName(files, numberOfFiles, userInput);
    if (fileName == NULL)
    {
        printf("No such file was indexed.");
        return -1;
    }
    int powerLength = getFile(fileName, powerStructs);
    if (powerLength == -1)
    {
        printf("No such file was indexed.");
        return -1;
    }

    printStruct(powerStructs, powerLength);
    printf("\n");
    return 0;
}

/// @brief Prompts user to create a new file or edit (append) and existing file.
/// @param files
/// @param powerStructs
/// @returns result code
int editData(FileIndex files[], PowerStruct powerStructs[])
{
    // ask user to write single data point
    printf("Write a single data point, specified as 3 decimal numbers separated by space, e.g. `2.0 3.1 151`\n> ");
    PowerStruct newDataPoint;
    scanf("%lf %lf %lf", &newDataPoint.GRID, &newDataPoint.SUSTAIN, &newDataPoint.USAGE);
    printf("received %lf, %lf, %lf\n\n", newDataPoint.GRID, newDataPoint.SUSTAIN, newDataPoint.USAGE);

    // ask user where to put data point (new file or existing)
    printf("Do you want to create new file (`N`), or append (`A`) an existing file?\n> ");
    char answer;
    scanf(" %c", &answer); // note the empty space, without it the buffer is not flushed for some reason

    switch (answer)
    {
    case 'A':
    case 'a':
    {
        printf("Existing files:\n");
        int numberOfFiles = getDirectoryData(files); // Antallet af filer i en mappe
        printDir(files, numberOfFiles);              // Print filnavne og numre

        char path[1024];
        strcpy(path, "./data/");

        int userInput = 0;
        printf("What is the number of the file you want to append to?\n> ");
        scanf(" %d", &userInput);

        char *filename = getFileName(files, numberOfFiles, userInput);
        FILE *fptr = fopen(strcat(path, filename), "a");
        fprintf(fptr, "%lf %lf %lf", newDataPoint.GRID, newDataPoint.SUSTAIN, newDataPoint.USAGE);
        fclose(fptr);
    }
    break;
    case 'N':
    case 'n':
    {
        char path[1024];
        strcpy(path, "./data/");
        char fileName[1024];
        printf("What do you want the file to be called?\n> ");
        scanf(" %s", &fileName);

        FILE *fptr = fopen(strcat(path, fileName), "w");
        fprintf(fptr, "%lf %lf %lf", newDataPoint.GRID, newDataPoint.SUSTAIN, newDataPoint.USAGE);
        fclose(fptr);
    }
    break;
    default:
        printf("Unknown answer");
        return -1;
    }
    return 0;
}