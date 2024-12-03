double calcArrayAverage(const double array[], int length);

/// @brief Find the average from an array of doubles.
/// @param array Array of values
/// @param length Amount of values
/// @return The average of the values
double calcArrayAverage(const double array[], const int length) {
    double sum = 0;

    for (int i = 0; i < length; i++) {
        sum += array[i];
    }

    return sum / (double)length;
}
