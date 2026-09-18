#ifndef HIGHWAY_H
#define HIGHWAY_H

#include <vector>
#include <string>


class Highway {
public:
    // Default constructor, needed to declare a Highway object outside the scope of a
    // try block so it can later be correctly assigned inside the try block without
    // being destroyed at the end of it.
    // See src/simulator.cpp, around line 83.
    Highway(){}
    // Constructor that takes the input file name.
    Highway(const std::string& filename);

    // Methods used to get the distance from the origin of a given checkpoint or junction.
    double getCheckpoint(int checkpointId) const;
    double getJunction(int junctionId) const;

    // Methods to get the number of checkpoints or junctions.
    int checkpointsSize() const { return checkpoints.size(); }
    int junctionsSize() const { return junctions.size(); }

    // Method to check that the provided highway is valid according to the assignment.
    bool validate() const;

private:
    // Vectors containing the junctions and the checkpoints.
    std::vector<double> checkpoints;
    std::vector<double> junctions;

    // Method used to fill the two vectors by reading the highway layout from the input file.
    void readLocationsFromFile(const std::string& filename, std::vector<std::pair<double, char>>& locations);

};

#endif
