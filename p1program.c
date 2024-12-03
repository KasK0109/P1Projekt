#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

/// @brief En fil og dens index nummer
typedef struct
{
    int number;
    char filename[256];
} FileIndex;

/// @brief A struct containing information about the power supplied by the grid, the power generated from sustainable energy and the usage.
typedef struct
{
    double GRID;
    double SUSTAIN;
    double USAGE;
} Power;

int getLinesFromFile(const char *fileName, Power powerData[]);
int getDirectoryData(FileIndex *files);
void printStruct(Power power[], int powerArrayLength);
void print_point(double grid, double sustain, double usage);
double calcArrayAverage(double array[], int length);
void printDir(FileIndex files[], int fileArrayLength);
char *getFileName(FileIndex files[], int fileArrayLength, int number);
int getFile(char *fileName, Power powerData[]);
int userReadFiles(FileIndex files[], Power powerData[]);
int userEditData(FileIndex files[], Power powerData[]);

int main(int argc, char *argv[])
{ // Input from the user determining what the program should do
    int UserMenuInput;
    // test
    if (argv[1] != NULL)
    {
        const int number = atoi(argv[1]);
        printf("User input received: %d\n", number);
        UserMenuInput = number;
    }
    else
    {
        printf("Hello welcome to program menu please select:\n");
        printf("1 - view csv files\n");
        printf("2 - edit csv files\n");
        printf("> ");
        scanf(" %d", &UserMenuInput);
    }
    // Array of files in a directory
    FileIndex files[1024]; // Array of structs based on a line in a file

    // Switch statement based on the user input
    Power powerData[2048];

    switch (UserMenuInput)
    {
    case 1:
        return userReadFiles(files, powerData);
    case 2:
        return userEditData(files, powerData);
    default:
        return -1;
    }
}

/// @brief Print all the data in a file within the `./data/` directory.
/// @param fileName The
/// @param powerData Array to be filled with data
/// @return a status code?
int getLinesFromFile(const char *fileName, Power powerData[])
{
    UU // Full name of hethe              path and filename
        char fileToOpen[256];
    copies into the fileTopOpen varia ble
        strcpy(fileToOpen, "./data/"); // concats the filename into the fileToOpen variable `./data/`
    strcat(fileToOpen, fileName);      // `./data/<fileName>`
                                       // File pointerr tto that opoints to  theFile pointer that points to an open file in read mode
    FILE *fptr = fopen(fileToOpen, "r");
    if (fptr == NULL)
        printf(File doesnot eist : % c, fileToOpen);
    { // happens if file does not exist
        exit(EXIT_FAILURE);
    }
    // / Char array
    //  Character array that represents a line in the file   char line[1024];
    int index = 0; // Index that keeps track of which position in the powerstruct array the program is at

    // note: sscanf scans the line starting from the first line.
    // if any line is empty, it stops collecting data,
    // which means that the first line in the data file should not be empty
    while (fgets(line, sizeof(line), fptr))
    {
        Power nextDataPoint;
        int result = sscanf(line, " %lf %lf %lf", &nextDataPoint.GRID, &nextDataPoint.SUSTAIN, &nextDataPoint.USAGE);
        if (result != 3)
        {
            // data point was empty (imagine an empty line at the start of the file)
            // if we did not do this, powerData would have a value of `0.0 0.0 0.0` at this index.
            continue;
        }

        powerData[index] = nextDataPoint;
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
    DIR *directory;       // Pointer to entry of DIR(Directory) type
    struct dirent *entry; // Pointer to a dirent(directory entity) type - contains info about the directory
    int fileNumber = 0;   // Counter for the number of files

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

/// @brief Given some power data, pretty print it
/// @param power the data
/// @param powerArrayLength the length of the data
void printStruct(Power power[], int powerArrayLength)
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

        print_point(grid, sustain, usage);
    }

    printf("\n");
    double gridAverage = calcArrayAverage(gridArray, powerArrayLength);
    double sustainAverage = calcArrayAverage(sustainArray, powerArrayLength);
    double usageAverage = calcArrayAverage(usageArray, powerArrayLength);

    printf("-------------------------------------------------\n");
    printf("|                    AVERAGES                   |\n");
    printf("-------------------------------------------------\n");

    print_point(gridAverage, sustainAverage, usageAverage);
    printf("\n"); // extra space
}

/// @brief Pretty prints a data point.
/// @param grid The grid value
/// @param sustain The sustain value
/// @param usage The usage value
void print_point(double grid, double sustain, double usage)
{
    // line begin
    if (grid <= 1.0)
    {
        printf("\033[31m GRID: OUTAGE\033[0m"); // red code
    }
    else
    {
        printf("\033[32m GRID: %6.2lf\033[0m", grid); // green code
    }

    printf("| \033[34mSUSTAINABLE: %6.2lf\033[0m", sustain); // blue code
    printf("| \033[36mUSAGE: %6.2lf\033[0m", usage);         // cyan code
    // line end
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

/// @brief Given an array of file indexes, pretty print their number and name (in file system).
/// @param files Files to print out
/// @param fileArrayLength Amount of files
void printDir(FileIndex files[], int fileArrayLength)
{
    for (int i = 0; i < fileArrayLength; i++)
    {
        printf("Number: %d - Filename: %s\n", files[i].number, files[i].filename);
    }
}

/// @brief Return the name of a file from its number.
/// @param files All the files
/// @param fileArrayLength Amount of files
/// @param number The file number to look for
/// @return The name of the file from its number (can be null)
char *getFileName(FileIndex files[], int fileArrayLength, int number)
{
    for (int i = 0; i < fileArrayLength; i++)
    {
        if (number == files[i].number)
        {
            return files[i].filename;
        }
    }
    return NULL;
}

/// @brief Reads an array of power structs from entries found in a file.
/// @param filename The name of the file (should be found in current directory)
/// @param powerData Array to write to (size <= amount of entries found in file)
/// @return Success code for reading from file
int getFile(char *filename, Power powerData[])
{
    return getLinesFromFile(filename, powerData);
}

/// @brief Lets the user read the data from a file (pretty printed).
/// @param files All the files found
/// @param powerData All the
/// @return Success code (fails if looking for non-existent file)
int userReadFiles(FileIndex files[], Power powerData[])
{
    int numberOfFiles = getDirectoryData(files); // Antallet af filer i en mappe
    printDir(files, numberOfFiles);              // Print filnavne og numre

    int userInput = 0;
    printf("Type the number of the file you want to see:\n> ");
    scanf(" %d", &userInput);

    // get data of file that the user wrote out, if it exists
    // this writes to powerData in the process
    char *fileName = getFileName(files, numberOfFiles, userInput);
    if (fileName == NULL)
    {
        printf("No such file was indexed.");
        return -1;
    }
    int powerLength = getFile(fileName, powerData);
    if (powerLength == -1)
    {
        printf("No such file was indexed.");
        return -1;
    }

    printStruct(powerData, powerLength);
    printf("\n");
    return 0;
}

/// @brief Prompts user to create a new file or edit (append) and existing file.
/// @param files Array to store files
/// @param powerData Array to store power data
/// @returns Result code
int userEditData(FileIndex files[], Power powerData[])
{
    // ask user to write single data point
    printf("Write a single data point, specified as 3 decimal numbers separated by space, e.g. `2.0 3.1 151`\n> ");
    Power newDataPoint;
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