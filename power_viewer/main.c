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
//#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.c"

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
#define POWER_ARRAY_SIZE 2048
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
        printf("3 - plot grid from csv files (experimental)\n");
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
        // printf("Found entry '%s'...\n", entry->d_name);
        // Check if entry is a regular file and ends with ".csv"
        if (entry->d_type == DT_REG) {
            // Find the length of the filename
            const size_t len = strlen(entry->d_name);
            // printf("Found len (%d) ...\n", len);

            // Check if the file has a ".csv" extension
            if (len > 4 && strcmp(entry->d_name + len - 4, ".csv") == 0) {
                // printf("File has csv extension!\n");
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
            fprintf(fptr, "%lf %lf %lf\n", newPoint.GRID, newPoint.SUSTAIN, newPoint.USAGE);
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

/**
 * A way of dynamically selecting fields of data points.
 * @param data the source data
 * @param field the field index (0..3)
 * @return the field (always a double for Power data)
 */
double read_field(const Power data, const int field) {
    switch (field) {
        case 0:
            return data.GRID;
        case 1:
            return data.USAGE;
        case 2:
            return data.SUSTAIN;
        default:
            return -1;
    }
}

void print_field(const int field) {
    switch (field) {
        case 0:
            printf("GRID");
        case 1:
            printf("SUSTAIN");
        case 2:
            printf("USAGE");
        default:
            printf("UNKNOWN");
    }
}

#define HEIGHT 8
#define WIDTH 80

/// @brief Display a basic point plot from the data,
/// scaled by stretching and weighing to fit discrete coordinates.
/// WARNING: This does not average out points, and simply picks the two closest points to scale with.
/// This means that it will literally skip certain points for source_length values above WIDTH*2.
/// @param source_data The data that is displayed
/// @param source_length The amount of data points
/// @param field the field to display
void print_plot_linear_fit(const Power source_data[], const int source_length, const int field) {
    double draw_points[WIDTH];
    for (size_t x = 0; x < WIDTH; x++) {
        const double x_t = inv_lerp(0, WIDTH, x);
        const double i_v = lerp(0, source_length - 1, x_t);
        // again, below only handles *up to two values*
        const size_t lower = (size_t)i_v; // round down
        const size_t upper = lower + 1; // round down: this works unless lower is the same as i_v
        const double i_t = inv_lerp(lower, upper, i_v);
        // see: only handles two points
        const double a = read_field(source_data[lower], field);
        const double b = read_field(source_data[upper], field);
        const double point = lerp(a, b, i_t);
        draw_points[x] = point;
    }

    double min_point = draw_points[0];
    double max_point = draw_points[0];
    for (size_t x = 0; x < WIDTH; x++) {
        const double point = draw_points[x];
        if (point > max_point) {
            max_point = point;
        }
        if (point < min_point) {
            min_point = point;
        }
    }
    assert(min_point <= max_point);

    printf("Data source length, which will be stretched to fit: %d\n", source_length);
    printf("Largest point: %lf\n", max_point);
    printf("Smallest point: %lf\n", min_point);
    printf("\n");
    print_field(field);
    printf(" PLOT:\n");
    for (int y = HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < WIDTH; x++) {
            // XY coordinate in point plot (going left->right, up->down, where the y-axis is *up*)
            // every point may be multiple points
            const double raw_point = draw_points[x];
            const double t = inv_lerp(min_point, max_point, raw_point);
            const double point_height = round(lerp(0, HEIGHT - 1, t));
            const int display_lower = y;
            const int display_upper = y + 1;
            if (point_height >= display_lower && point_height < display_upper) {
                printf("O"); // single point char
            } else {
                printf(" "); // single space char
            }
        }
        printf("\n"); // end line
    }
}

/// @brief Display a basic plot over the data points, cut to evenly scale to fit the terminal.
/// @param source_data The data that is displayed
/// @param source_length The amount of data points
/// @param field the field to display
void print_plot_whole_cut_scaled(const Power source_data[], const int source_length, const int field) {
    // make graph fit on screen, averaging out multiples of values if there are too many
    const int cut_length = ((source_length - 1) % WIDTH) + 1;
    const int values_per_point = new_max(1, source_length / (int)WIDTH);
    double max_point = read_field(source_data[0], field); // in order to scale
    double min_point = read_field(source_data[0], field); // idk what the right thing to use is
    double points[cut_length]; // each element is one discrete x coordinate
    for (int x = 0; x < cut_length; x++) {
        double pointSum = 0.0;
        for (int i = 0; i < values_per_point; i++) {
            pointSum += read_field(source_data[x + i], field);
        }
        const double point = pointSum / (double) values_per_point;
        if (point > max_point) {
            max_point = point;
        }
        if (point < min_point) {
            min_point = point;
        }
        points[x] = point;
    }
    assert(max_point >= min_point);

    printf("Points (width of graph): %d\n", cut_length);
    printf("Largest point: %lf\n", max_point);
    printf("Smallest point: %lf\n", min_point);
    printf("\n");
    print_field(field);
    printf(" PLOT:\n");
    // print plot by going over each data point *per* print height
    for (int y = HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < cut_length; x++) {
            // XY coordinate in point plot (going left->right, up->down, where the y-axis is *up*)
            // every point may be multiple points
            const double raw_point = points[x];
            const double t = inv_lerp(min_point, max_point, raw_point);
            const double point_height = round(lerp(0, HEIGHT - 1, t));
            const int display_lower = y;
            const int display_upper = y + 1;
            if (point_height >= display_lower && point_height < display_upper) {
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

     // ask user where to put data point (new file or existing)
    printf("Do you want to plot GRID ('G'), SUSTAIN (`S`), or USAGE (`U`) in the data?\n> ");
    char field_answer;
    scanf(" %c", &field_answer); // note the empty space, without it the buffer is not flushed for some reason

    int field = 0;
    // both upper and lower case are used
    switch (field_answer) {
        case 'G':
        case 'g': {
            field = 0;
        }
        break;
        case 'S':
        case 's': {
            field = 1;
        }
        break;
        case 'U':
        case 'u': {
            field = 2;
        }
        break;
        default:
            printf("Unknown answer");
        return EXIT_FAILURE;
    }

    // ask user where to put data point (new file or existing)
    printf("Do you want to use whole scale only ('W'), or stretch to fit the data (`S`) for the dataset?\n> ");
    char plot_answer;
    scanf(" %c", &plot_answer); // note the empty space, without it the buffer is not flushed for some reason

    // both upper and lower case are used
    switch (plot_answer) {
        case 'W':
        case 'w': {
            print_plot_whole_cut_scaled(data, dataLength, field);
        }
        break;
        case 'S':
        case 's': {
            print_plot_linear_fit(data, dataLength, field);
        }
        break;
        default:
            printf("Unknown answer");
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
