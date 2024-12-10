/* # Description
 * P1Projektets C-program.
 * Dette program kan bruges til at interagere med CSV-filer,
 * som dannes af simple målinger over strømgeneration i en af en energifarm.
 * Til vores projekt er energifarmen bestående af et solpanel og en generator.
 *
 * # Design
 * Vi prøver så vidt muligt kun at returnere fejlkoder (exit codes) fra funktioner,
 * sådan så fejlbehandling kan foregå så eksplicit som muligt.
 *
 * # Troubleshooting
 * Hvis projektet har flere analysefejl i din editor, kan det skyldes at du ikke har valgt den rigtige toolchain.
 * Hvis du er i CLion, kan du vælge den version af MinGW som du har installeret via IMPR-kurset.
 *
 * TODO: Fix character array passing.
 * I am trying to make it so we only pass pointers around by parameter,
 * but there is the problem of how long char* / char[] are kept alive.
 * Locally scoped points are kept alive always,
 * but the thing they point to might get deallocated if they point to local value that goes out of scope.
 * This can be fixed with malloc, but I want to avoid this.
 * The initial solution was just to keep a top down reference to every value,
 * like some sort of arena, and that might be what I need to go back to?
 * * Licens: GPL 3.0
 */

#include <dirent.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"

// #include <math.h>
#define new_max(x,y) (((x) >= (y)) ? (x) : (y))
#define new_min(x,y) (((x) <= (y)) ? (x) : (y))

/// @brief En fil og dens index nummer
typedef struct {
    int number;
    char filename[256];
} FileIndex;

/// @brief A struct containing information about the power supplied by the grid,
/// the power generated from sustainable energy, and the usage.
typedef struct {
    double GRID;
    double SUSTAIN;
    double USAGE;
} Power;

#define FILE_ARRAY_SIZE 256
#define POWER_ARRAY_SIZE FILE_ARRAY_SIZE
#define PATH_CHAR_SIZE 1024

int fetchContentFromFile(const char *filename, Power powerData[], int *powerDataLength);

int loadDirectoryData(FileIndex files[], int *fileCount);

void printStruct(Power power[], int powerArrayLength);

void print_point(double grid, double sustain, double usage);

void printDir(FileIndex files[], int fileCount);

int fetchFileName(const FileIndex files[], int fileArrayLength, int number, const char filename[]);

int userPrintFile();

int userEditFile();

int userPlotData();

int main(int argc, char *argv[]) {
    // Input from the user determining what the program should do
    int UserMenuInput;
    // test
    if (argv[1] != NULL) {
        const int number = atoi(argv[1]);
        printf("User input received: %d\n", number);
        UserMenuInput = number;
    } else {
        printf("Hello welcome to program menu please select:\n");
        printf("1 - view csv files\n");
        printf("2 - edit csv files\n");
        printf("3 - plot grid from csv files\n");
        printf("> ");
        scanf(" %d", &UserMenuInput);
    }

    switch (UserMenuInput) {
        case 1:
            return userPrintFile();
        case 2:
            return userEditFile();
        case 3:
            return userPlotData();
        default:
            return EXIT_FAILURE;
    }
}

/// @brief Print all the data in a file within the `./data/` directory.
/// @param filename The name of the file we wish to open
/// @param powerData Array to be filled with data
/// @param powerDataLength Amount of data points
/// @return Exit code (fails if file does not exist)
int fetchContentFromFile(const char *filename, Power powerData[], int *powerDataLength) {
    char filePath[256];

    strcpy(filePath, "./data/"); // `./data/`
    strcat(filePath, filename); // `./data/<fileName>`

    // File pointer that points to an open file in read mode
    FILE *fptr = fopen(filePath, "r"); // file is opened
    if (fptr == NULL) {
        // happens if file does not exist
        printf("File does not exist: %s", filePath);
        return EXIT_FAILURE;
    }

    // Character array that represents a line in the file
    char line[256];
    // Index that keeps track of which position in the array the program is at
    *powerDataLength = 0;

    // note: sscanf scans the line starting from the first line.
    // if any line is empty, it stops collecting data,
    // which means that the first line in the data file should not be empty
    while (fgets(line, sizeof(line), fptr)) {
        Power nextDataPoint;
        const int result = sscanf(line, " %lf %lf %lf", &nextDataPoint.GRID, &nextDataPoint.SUSTAIN,
                                  &nextDataPoint.USAGE);
        if (result != 3) {
            // data point was empty (imagine an empty line at the start of the file)
            // if we did not do this, powerData would have a value of `0.0 0.0 0.0` at this index.
            continue;
        }

        powerData[*powerDataLength] = nextDataPoint;
        powerDataLength++;
    }

    fclose(fptr); // file is closed
    return 0;
}

/// @brief Fetch directory data, relative to running directory. Also prints information as it goes.
/// @param files Array to write file data into
/// @param fileCount The number of files
/// @return Exit code (may fail when looking for the data folder)
int loadDirectoryData(FileIndex files[], int *fileCount) {
    // Pointer to entry of DIR(Directory) type
    struct dirent *entry; // Pointer to a dirent(directory entity) type - contains info about the directory

    DIR *directory = opendir(".");

    if (directory == NULL) {
        printf("Error opening folder\n");
        return EXIT_FAILURE;
    }

    while ((entry = readdir(directory)) != NULL) {
        // Check if entry is a regular file and ends with ".csv"
        if (entry->d_type == DT_REG) {
            // Find the length of the filename
            const int len = strlen(entry->d_name); // NOLINT(*-narrowing-conversions)

            // Check if the file has a ".csv" extension
            if (len > 4 && strcmp(entry->d_name + len - 4, ".csv") == 0) {
                FileIndex *file = &files[*fileCount];
                file->number = *fileCount + 1;

                // Corrected strncpy:
                strncpy(file->filename, entry->d_name, sizeof(file->filename) - 1);
                file->filename[sizeof(file->filename) - 1] = '\0';

                (*fileCount)++;
            }
        }
    }

    if (closedir(directory) != EXIT_SUCCESS) {
        printf("Error closing directory.\n");
        return EXIT_FAILURE;
    }

    return 0;
}

/// @brief Given some power data, pretty print it
/// @param power the data
/// @param powerArrayLength the length of the data
void printStruct(Power power[], const int powerArrayLength) {
    double gridArray[powerArrayLength];
    double sustainArray[powerArrayLength];
    double usageArray[powerArrayLength];

    printf("-------------------------------------------------\n");
    printf("|                      DATA                     |\n");
    printf("-------------------------------------------------\n");

    for (int i = 0; i < powerArrayLength; i++) {
        const double grid = power[i].GRID;
        const double sustain = power[i].SUSTAIN;
        const double usage = power[i].USAGE;

        gridArray[i] = grid;
        sustainArray[i] = sustain;
        usageArray[i] = usage;

        print_point(grid, sustain, usage);
    }
    printf("\n");

    const double gridAverage = calcArrayAverage(gridArray, powerArrayLength);
    const double sustainAverage = calcArrayAverage(sustainArray, powerArrayLength);
    const double usageAverage = calcArrayAverage(usageArray, powerArrayLength);

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
void print_point(const double grid, const double sustain, const double usage) {
    // line begin
    if (grid <= 1.0) {
        printf("\033[31m GRID: OUTAGE\033[0m"); // red code
    } else {
        printf("\033[32m GRID: %6.2lf\033[0m", grid); // green code
    }

    printf("| \033[34mSUSTAINABLE: %6.2lf\033[0m", sustain); // blue code
    printf("| \033[36mUSAGE: %6.2lf\033[0m", usage); // cyan code
    // line end
    printf("\n");
}


/// @brief Given an array of file indexes, pretty print their number and name (in file system).
/// @param files Files to print out
/// @param fileCount Amount of files
void printDir(FileIndex files[], const int fileCount) {
    printf("Files in directory: %d\n", fileCount);
    for (int i = 0; i < fileCount; i++) {
        printf("Number: %d - Filename: %s\n", files[i].number, files[i].filename);
    }
}

/// @brief Return the name of a file from its number.
/// @param files All the files
/// @param fileArrayLength Amount of files
/// @param number The file number to look for
/// @param filename
/// @return The name of the file from its number (can be null)
int fetchFileName(const FileIndex files[], const int fileArrayLength, const int number, const char filename[]) {
    for (int i = 0; i < fileArrayLength; i++) {
        if (number == files[i].number) {
            filename = (char *) files[i].filename;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}


/// @brief Lets the user select a file, and reads the file contents.
/// @param powerData The data found in the file
/// @param powerLength The amount of data points found in the file
/// @return Exit code (fails if unable to read file)
int userSelectsFile(Power powerData[], int *powerLength) {
    FileIndex files[FILE_ARRAY_SIZE];
    int fileCount;
    const int loadSucces = loadDirectoryData(files, &fileCount);
    if (loadSucces == EXIT_FAILURE) {
        printf("Failed to load directory.\n");
        return EXIT_FAILURE;
    }
    printDir(files, fileCount);

    int userInput = 0;
    printf("Type the number of the file you want to see:\n> ");
    scanf("%d", &userInput);

    // get data of file that the user wrote out, if it exists
    // this writes to powerData in the process
    const char filename[PATH_CHAR_SIZE];
    const int filenameFetchResult = fetchFileName(files, fileCount, userInput, filename);
    if (filenameFetchResult != EXIT_SUCCESS) {
        printf("No such file was indexed.\n");
        return EXIT_FAILURE;
    }
    const int dataFetchResult = fetchContentFromFile(filename, powerData, powerLength);
    if (dataFetchResult != EXIT_SUCCESS) {
        printf("No such file was indexed.\n");
        return EXIT_FAILURE;
    }
    printf("File found and loaded.\n");
    return 0;
}

/// @brief Lets the user read the data from a file (pretty printed).
/// @return Exit code (fails if looking for non-existent file)
int userPrintFile() {
    Power powerData[POWER_ARRAY_SIZE];
    int powerLength;
    const int result = userSelectsFile(powerData, &powerLength);
    if (result == 0) {
        printStruct(powerData, powerLength);
        printf("\n");
    }
    return result;
}

/// @brief Prompts user to create a new file or edit (append) and existing file.
/// @returns Result code
int userEditFile() {
    // ask user to write single data point
    printf("Write a single data point, specified as 3 decimal numbers separated by space, e.g. `2.0 3.1 151`\n> ");
    Power newDataPoint;

    scanf("%lf %lf %lf", &newDataPoint.GRID, &newDataPoint.SUSTAIN, &newDataPoint.USAGE);
    printf("received %lf, %lf, %lf\n\n", newDataPoint.GRID, newDataPoint.SUSTAIN, newDataPoint.USAGE);

    // ask user where to put data point (new file or existing)
    printf("Do you want to create new file (`N`), or append (`A`) an existing file?\n> ");
    char answer;
    scanf(" %c", &answer); // note the empty space, without it the buffer is not flushed for some reason

    // both upper and lower case a/n are used
    switch (answer) {
        case 'A':
        case 'a': {
            printf("Existing files:\n");
            FileIndex files[FILE_ARRAY_SIZE];
            int fileCount;
            const int dirLoadResult = loadDirectoryData(files, &fileCount); // Antallet af filer i en mappe
            if (dirLoadResult != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            printDir(files, fileCount); // Print filnavne og numre

            char path[PATH_CHAR_SIZE];
            strcpy(path, "./data/");

            int userInput = 0;
            printf("What is the number of the file you want to append to?\n> ");
            scanf(" %d", &userInput);

            char filename[PATH_CHAR_SIZE];
            const int filenameFetchResult = fetchFileName(files, fileCount, userInput, filename);
            if (filenameFetchResult != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            FILE *fptr = fopen(strcat(path, filename), "a");
            fprintf(fptr, "%lf %lf %lf", newDataPoint.GRID, newDataPoint.SUSTAIN, newDataPoint.USAGE);
            fclose(fptr);
        }
        break;
        case 'N':
        case 'n': {
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
            return EXIT_FAILURE;
    }
    return 0;
}

/// @brief Lets the user view a point plot of file data.
/// @return Exit code (reading a file may fail)
int userPlotData() {
    int dataLength;
    Power data[];
    const int selectResult = userSelectsFile(data, &dataLength);
    if (selectResult != EXIT_SUCCESS) {

    }

    return 0;
}

/// @brief Display a basic plot over the data points, cut to evenly scale to fit an 80-width console.
/// @param powerData The data that is displayed
/// @param length The amount of data points
void print_plot_whole_cut_stretched(const Power powerData[], const int length) {
    printf("GRID PLOT:\n");
    // print plot by going over each data point *per* print height
    const int HEIGHT = 8;
    for (int y = 0; y < HEIGHT; y++) {
        const int cut_length = length - length % 80;
        for (int x = 0; x < cut_length; x++) {
            const double MAX_VALUE = 5.0;
            const int scale = new_max(1, length / 80);
            double sum = 0;
            for (int i = 0; i < scale; i++) {
                sum += powerData[i].GRID;
            }
            const double average = sum / (double) scale;
            const double fraction = average / MAX_VALUE;
            const double height_lower = x / (double) HEIGHT;
            const double height_upper = (x + 1) / (double) HEIGHT;
            if (fraction >= height_lower && fraction <= height_upper) {
                printf(" "); // single space char
            } else {
                printf("."); // single point char
            }
        }
        printf("\n"); // end line
    }
}

// /// @brief Display a basic plot over the data points, averaged to fit an 80-width console.
// /// @param powerData The data that is displayed
// /// @param length The amount of data points
// void print_plot_evenly_stretched(const Power powerData[], const int length) {
//     // todo: impl
// }
