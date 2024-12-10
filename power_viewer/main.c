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
 * Licens: GPL 3.0
 */

#include <assert.h>
#include <dirent.h>
#include <float.h>
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

int fetchContentFromFile(const char *filename, Power powerData[], int *powerDataIndex);

int loadDirectoryData(FileIndex files[], int *fileCount);

void printStruct(Power power[], int powerArrayLength);

void print_point(double grid, double sustain, double usage);

void printDir(FileIndex files[], int fileCount);

int fetchFileName(const FileIndex files[], int fileCount, int number, char *filename);

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
/// @param powerDataIndex Amount of data points
/// @return Exit code (fails if file does not exist)
int fetchContentFromFile(const char *filename, Power powerData[], int *powerDataIndex) {
    char filePath[256];

    strcpy(filePath, "./data/"); // `./data/`
    strcat(filePath, filename); // `./data/<fileName>`

    // File pointer that points to an open file in read mode
    FILE *fptr = fopen(filePath, "r"); // file is opened
    if (fptr == NULL) {
        printf("File does not exist: %s\n", filePath);
        return EXIT_FAILURE;
    }

    // Character array that represents a line in the file
    char line[256];
    // Index that keeps track of which position in the array the program is at
    *powerDataIndex = 0;

    // note: sscanf scans the line starting from the first line.
    // if any line is empty, it stops collecting data,
    // which means that the first line in the data file should not be empty
    while (fgets(line, sizeof(line), fptr)) {
        Power nextPoint;
        const int result = sscanf(line, "%lf %lf %lf", &nextPoint.GRID, &nextPoint.SUSTAIN,
                                  &nextPoint.USAGE);
        if (result != 3) {
            // data point was empty (imagine an empty line at the start of the file)
            // if we did not do this, powerData would have a value of `0.0 0.0 0.0` at this index.
            continue;
        }

        powerData[*powerDataIndex] = nextPoint;
        (*powerDataIndex)++;
    }

    fclose(fptr); // file is closed
    return 0;
}

/// @brief Fetch directory data, relative to running directory. Also prints information as it goes.
/// @param files Array to write file data into (only csv files)
/// @param fileCount The number of files (csv files)
/// @return Exit code (may fail when looking for the data folder)
int loadDirectoryData(FileIndex files[], int *fileCount) {
    // Pointer to entry of DIR(Directory) type
    struct dirent *entry; // Pointer to a dirent(directory entity) type - contains info about the directory

    char dirname[] = "./data";
    DIR *directory = opendir(dirname);
    if (directory == NULL) {
        printf("Error opening folder (null) at path %s\n", dirname);
        return EXIT_FAILURE;
    }

    int fileIndex = 0;
    while ((entry = readdir(directory)) != NULL) {
        printf("Found entry '%s'...\n", entry->d_name);
        // Check if entry is a regular file and ends with ".csv"
        if (entry->d_type == DT_REG) {
            // Find the length of the filename
            const size_t len = strlen(entry->d_name);
            printf("Found len (%d) ...\n", len);

            // Check if the file has a ".csv" extension
            if (len > 4 && strcmp(entry->d_name + len - 4, ".csv") == 0) {
                printf("File has csv extension!\n");
                FileIndex *file = &files[fileIndex];
                file->number = fileIndex + 1;
                strcpy(file->filename, entry->d_name);
                fileIndex++;
            }
        }
    }
    *fileCount = fileIndex;

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
    printf("Files in directory (%d):\n", fileCount);
    for (int i = 0; i < fileCount; i++) {
        printf("Number: %d - Filename: %s\n", files[i].number, files[i].filename);
    }
}

/// @brief Return the name of a file from its number.
/// @param files All the files
/// @param fileCount Amount of files
/// @param number The file number to look for
/// @param filename
/// @return The name of the file from its number (can be null)
int fetchFileName(const FileIndex files[], const int fileCount, const int number, char filename[]) {
    for (int i = 0; i < fileCount; i++) {
        if (number == files[i].number) {
            strcpy(filename, files[i].filename);
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}


/// @brief Lets the user select a file, and reads the file contents.
/// @param powerData The data found in the file
/// @param powerLength The amount of data points found in the file
/// @return Exit code (fails if unable to read file)
int userLoadsFile(Power powerData[], int *powerLength) {
    FileIndex files[FILE_ARRAY_SIZE];
    int fileCount;
    const int loadSucces = loadDirectoryData(files, &fileCount);
    if (loadSucces == EXIT_FAILURE) {
        printf("Failed to load directory.\n");
        return EXIT_FAILURE;
    }
    printDir(files, fileCount);

    int userInput = 0;
    printf("Type the number of the file you want to select:\n> ");
    scanf("%d", &userInput);

    // get data of file that the user wrote out, if it exists
    // this writes to powerData in the process
    char filename[FILENAME_MAX];
    const int filenameFetchResult = fetchFileName(files, fileCount, userInput, filename);
    if (filenameFetchResult != EXIT_SUCCESS) {
        printf("No such file was indexed.\n");
        return EXIT_FAILURE;
    }
    printf("File name: %s\n", filename);
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
    const int result = userLoadsFile(powerData, &powerLength);
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
    printf("Write a single data point, specified as 3 decimal numbers separated by space.\n");
    printf("A decimal number is some amount of digits (e.g. `5`) and an optional fractional (e.g. `.81`).\n");
    printf("Example: `2.0 3.1 151`\n> ");
    Power newPoint;

    scanf("%lf %lf %lf", &newPoint.GRID, &newPoint.SUSTAIN, &newPoint.USAGE);
    printf("received %lf, %lf, %lf\n\n", newPoint.GRID, newPoint.SUSTAIN, newPoint.USAGE);

    // ask user where to put data point (new file or existing)
    printf("Do you want to create new file (`N`), or append (`A`) an existing file?\n> ");
    char answer;
    scanf(" %c", &answer); // note the empty space, without it the buffer is not flushed for some reason

    // both upper and lower case a/n are used
    switch (answer) {
        case 'A':
        case 'a': {
            FileIndex files[FILE_ARRAY_SIZE];
            int fileCount;
            const int dirLoadResult = loadDirectoryData(files, &fileCount); // Antallet af filer i en mappe
            if (dirLoadResult != EXIT_SUCCESS) {
                printf("Failed to load directory.\n");
                return EXIT_FAILURE;
            }
            printf("Existing files:\n");
            printDir(files, fileCount); // Print filnavne og numre

            char path[PATH_CHAR_SIZE];
            strcpy(path, "./data/");
            // char path[PATH_CHAR_SIZE] = "./data/"; // why would this not work?

            int userInput = 0;
            printf("What is the number of the file you want to append to?\n> ");
            scanf(" %d", &userInput);

            char filename[PATH_CHAR_SIZE];
            const int filenameFetchResult = fetchFileName(files, fileCount, userInput, filename);
            if (filenameFetchResult != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            printf("Fetched file. Appending...\n");
            FILE *fptr = fopen(strcat(path, filename), "a");
            fprintf(fptr, "%lf %lf %lf", newPoint.GRID, newPoint.SUSTAIN, newPoint.USAGE);
            fclose(fptr);
            printf("Wrote [%lf %lf %lf] to %s", newPoint.GRID, newPoint.SUSTAIN, newPoint.USAGE, filename);
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
            fprintf(fptr, "%lf %lf %lf", newPoint.GRID, newPoint.SUSTAIN, newPoint.USAGE);
            fclose(fptr);
        }
        break;
        default:
            printf("Unknown answer");
            return EXIT_FAILURE;
    }
    return 0;
}

/// @brief linear interpolation
/// @param a starting value (t = 0)
/// @param b ending value (t = 1)
/// @param t fraction (standard range would be 0 to 1)
/// @note not clamped
/// @return value between a and b
double lerp(const double a, const double b, const double t) {
    return ((1.0 - t) * a) + (b * t);
}

/// @brief inverse linear interpolation
/// @param a starting value (t = 0)
/// @param b ending value (t = 1)
/// @param v value we want a fraction from (standard range would be between a and b)
/// @note not clamped
/// @return fraction between a and b
double inv_lerp(const double a, const double b, const double v) {
    return (v - a) / (b - a);
}

#define HEIGHT 8
#define WIDTH 80

/// @brief Display a basic point plot from the data,
/// scaled by stretching and weighing to fit discrete coordinates.
/// @param source_data The data that is displayed
/// @param source_length The amount of data points
void print_plot_weighted_stretched(const Power source_data[], const int source_length) {
    // # if weight = 0.9:
    // point[0] = (data[0] * 0.9 + data[1] * (1.0-0.9)) / 0.9
    // point[1] = (data[1] * (1.0-0.9 * 0.9)
    // # if weight = 1.1:
    // point[0] = data[0] * 1.1 + data[1] * (1.0-1.1)) / 1.1
    // # if weight = 2.0:
    // point[0] = data[0] + data[1] / 2

    // There are `source_length` weights in total.
    const double points_per_weight = (double) WIDTH / (double) source_length;
    const double weights_per_point = (double) source_length / (double) WIDTH; // equivalent to `1/points_per_weight`

    double max_point_value = 0.0;
    double min_point_value = FLT_MAX;

    double points[WIDTH];
    double rolling_value_total = 0.0;
    double rolling_point_total = 0.0; // if WIDTH <= source_length, this is always <2.0
    size_t point_index = 0;
    for (size_t source_index = 0; source_index < source_length; source_index++) {
        const double value = source_data[source_index].GRID;
        rolling_point_total += points_per_weight;
        while (rolling_point_total >= weights_per_point) {
            // there may be multiple points per index (WIDTH > source_length)
            const double fill_point_weight = (rolling_point_total - points_per_weight) / weights_per_point;
            const double fill_value_weight = value * fill_point_weight;
            const double proportional_value = rolling_value_total + fill_value_weight;

            if (proportional_value > max_point_value) {
                max_point_value = proportional_value;
            } // not else-if, because if there may be only one point
            if (proportional_value < min_point_value) {
                min_point_value = proportional_value;
            }
            points[point_index] = proportional_value;
            point_index += 1;
            rolling_value_total += value - fill_value_weight;
            rolling_point_total -= weights_per_point;
        }
    }
    // note: the above math does not do what I want it to do; it takes from rolling value,
    // but it does not apply "proportionally" according to point/data relevancy
    // print plot by going over each data point *per* print height
    for (int y = HEIGHT; y > 0; y--) {
        for (int x = 0; x < WIDTH; x++) {
            // XY coordinate in point plot (going left->right, up->down, where the y-axis is *up*)
            // every point may be multiple points
            const double raw_point = points[x];
            const double t = inv_lerp(min_point_value, max_point_value, raw_point);
            const int show_height = HEIGHT * t;
            if (show_height >= y && show_height < (y + 1)) {
                printf("O"); // single point char
            } else {
                printf(" "); // single space char
            }
        }
        printf("\n"); // end line
    }


    printf("GRID PLOT:\n");
}

/// @brief Display a basic plot over the data points, cut to evenly scale to fit the terminal.
/// @param source_data The data that is displayed
/// @param source_length The amount of data points
void print_plot_whole_cut_stretched(const Power source_data[], const int source_length) {
    // make graph fit on screen, averaging out multiples of values if there are too many
    const int cut_length = source_length % WIDTH;
    const int values_per_point = new_max(1, source_length / WIDTH);
    double max_point = 0; // in order to scale
    double min_point = FLT_MAX; // idk what the right thing to use is
    double points[cut_length]; // each element is one discrete x coordinate
    for (int x = 0; x < cut_length; x++) {
        double pointSum = 0.0;
        for (int i = 0; i < values_per_point; i++) {
            pointSum += source_data[x + i].GRID;
        }
        const double point = pointSum / (double) values_per_point;
        if (point > max_point) {
            max_point = pointSum;
        } else if (point < min_point) {
            min_point = pointSum;
        }
        points[x] = point;
    }
    assert(max_point > min_point);

    printf("Points (width): %d\n", cut_length);
    printf("Largest point: %lf\n", max_point);
    printf("Smallest point: %lf\n", min_point);
    printf("\n");
    printf("GRID PLOT:\n");
    // print plot by going over each data point *per* print height
    for (int y = HEIGHT; y > 0; y--) {
        for (int x = 0; x < cut_length; x++) {
            // XY coordinate in point plot (going left->right, up->down, where the y-axis is *up*)
            // every point may be multiple points
            const double raw_point = points[x];
            const double t = inv_lerp(min_point, max_point, raw_point);
            const int show_height = HEIGHT * t;
            if (show_height >= y && show_height < (y + 1)) {
                printf("O"); // single point char
            } else {
                printf(" "); // single space char
            }
        }
        printf("\n"); // end line
    }
}


/// @brief Lets the user view a point plot of file data.
/// @return Exit code (reading a file may fail)
int userPlotData() {
    int dataLength;
    Power data[POWER_ARRAY_SIZE];
    const int selectResult = userLoadsFile(data, &dataLength);
    if (selectResult != EXIT_SUCCESS) {
        printf("Failed to get a file.");
        return EXIT_FAILURE;
    }
    // print_plot_weighted_stretched(data, dataLength);
    print_plot_whole_cut_stretched(data, dataLength);


    return EXIT_SUCCESS;
}

// /// @brief Display a basic plot over the data points, averaged to fit an 80-width console.
// /// @param powerData The data that is displayed
// /// @param length The amount of data points
// void print_plot_evenly_stretched(const Power powerData[], const int length) {
//     // todo: impl
// }
