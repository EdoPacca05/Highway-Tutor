#include "highway.h"

#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <cmath>

/*
 * Private utility function.
 * Reads the Highway.txt file and stores the distances (in km); ids are given
 * by the position of each element in the sorted array.
 *
 * The type can be:
 *  - 'V' for a checkpoint ("Varco")
 *  - 'S' for a junction ("Svincolo")
 *
 * If the file cannot be opened or contains errors, an exception is thrown.
 */
void Highway::readLocationsFromFile(
    const std::string& filename,
    std::vector<std::pair<double, char>>& locations)
{
    std::ifstream file(filename);

    // Check that the file was opened correctly.
    if (!file)
        throw std::runtime_error("Error: unable to open Highway.txt");

    double km;
    char type;

    // Read the file line by line (km + type).
    while (file >> km >> type) {
        // Check that the type and the distance are valid.
        if (type != 'V' && type != 'S')
            throw std::runtime_error("Error: invalid type in Highway.txt");
        if (km < 0)
            throw std::runtime_error("Error: negative distance in Highway.txt");

        // Constructs the element directly (in-place) in the vector, forwarding the
        // constructor's parameters. This avoids copying/moving the object.
        // See the difference between emplace_back and push_back.
        locations.emplace_back(km, type);
    }

    // The file cannot be empty.
    if (locations.empty())
        throw std::runtime_error("Error: Highway.txt is empty");
}
/*
 * Highway class constructor.
 *
 * - Reads the Highway.txt file
 * - Splits checkpoints and junctions
 * - Sorts everything by increasing distance
 * - Checks that the highway respects the assignment's constraints
 */
Highway::Highway(const std::string& filename)
{
    // Temporary vector holding all the locations read from the file.
    std::vector<std::pair<double, char>> locations;

    // Read the file.
    readLocationsFromFile(filename, locations);

    // Split checkpoints and junctions.
    for (const std::pair<double, char>& loc : locations) {

        if (loc.second == 'V')
            checkpoints.push_back(loc.first);
        else
            junctions.push_back(loc.first);
    }

    // Sort checkpoints by distance from the origin.
    std::sort(checkpoints.begin(), checkpoints.end(),
              [](double a, double b) {
                  return a < b;
              });

    // Sort junctions by distance from the origin.
    std::sort(junctions.begin(), junctions.end(),
              [](double  a, double  b) {
                  return a < b;
              });

    // Check the constraints required by the assignment.
    if (!validate())
        throw std::runtime_error("Error: Highway.txt does not satisfy the constraints");
}

/*
 * Returns the checkpoint with the given ID.
 *
 * If the ID does not exist, returns -1.
 */
double Highway::getCheckpoint(int checkpointId) const {
    if (checkpointId < 1 || checkpointId > checkpoints.size())
        return -1;
    return checkpoints[checkpointId - 1];
}


/*
 * Returns the junction with the given ID.
 *
 * If the ID does not exist, returns -1.
 */
double Highway::getJunction(int junctionId) const {
    if (junctionId < 1 || junctionId > junctions.size())
        return -1;
    return junctions[junctionId - 1];
}

/*
 * Checks that the highway satisfies all the constraints
 * specified in the project assignment.
 */
bool Highway::validate() const
{
    // There must be at least two checkpoints.
    if (checkpoints.size() < 2)
        return false;

    // There must be at least one junction.
    if (junctions.empty())
        return false;

    double firstCheckpoint = checkpoints.front();
    double lastCheckpoint  = checkpoints.back();

    bool hasBefore = false;
    bool hasAfter  = false;

    // Check that there is:
    // - at least one junction before the first checkpoint
    // - at least one junction after the last checkpoint
    for (const double j : junctions) {
        if (j <= firstCheckpoint - 1.0)
            hasBefore = true;
        if (j >= lastCheckpoint + 1.0)
            hasAfter = true;
    }

    if (!hasBefore || !hasAfter)
        return false;

    // Check the minimum distance between junctions and checkpoints (>= 1 km).
    for (const double j : junctions) {
        for (const double  v : checkpoints) {
            if (std::abs(j - v) < 1.0)
                return false;
        }
    }

    return true;
}
