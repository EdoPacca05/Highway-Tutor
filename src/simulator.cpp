#include "highway.h"

#include <fstream>
#include <random>
#include <cmath>
#include <iostream>
#include <set>

// Simple structure needed to represent speed profiles.
struct Interval
{
    int v,t;

    Interval(){v=0; t=0;};
    std::string toString() const {return std::string(std::to_string(v)+" "+std::to_string(t));}
};

// Random generator for double values.
double randBetweenDouble(double a, double b, std::mt19937_64& rng)
{
    std::uniform_real_distribution<double> dist(a, b);
    return dist(rng);
}

// Random generator for int values.
int randBetweenInt(int a, int b, std::mt19937_64& rng)
{
    std::uniform_int_distribution<int> dist(a, b);
    return dist(rng);
}

// Prints a vehicle's (identified by plate) transit through a checkpoint.
void PrintToPassages(std::ofstream& file,int checkpoint, const std::string& plate, double time)
{
    file<<checkpoint<<" "<<plate<<" "<<time<<std::endl;
}

// Prints the vehicles' trips.
void PrintToRuns(std::ofstream& file, const std::string& plate, int entrance, int exit, double entranceTime, std::vector<Interval>& speedProfile)
{
    file<<plate<<" "<<entrance<<" "<<exit<<" "<<entranceTime<<" ";
    for(int i=0; i<speedProfile.size();i++)
    {
        file<<speedProfile.at(i).toString();
        if(i!=speedProfile.size()-1)
        {
            file<<", ";
        }
    }
    file<<std::endl;
}

int main()
{
    // Adjustable constant for the number of vehicles.
    const int numberOfVehicles=10000;

    // Random number generator, initialized once and passed by reference.
    std::mt19937_64 rng{ std::random_device{}() };

    // Constants defined by the assignment.
    const int minVel=80, maxVel=190;
    const int minTime=5, maxTime=15;
    const double minEntranceTime=0.5, maxEntranceTime=10;

    // Set used to make sure there are no duplicate plates.
    std::set<std::string> plateCheck;

    // Opens the files; std::ios::trunc clears any previous content before writing.
    std::ofstream passagesStream("../Data/Passages.txt", std::ios::out | std::ios::trunc);
    std::ofstream runsStream("../Data/Runs.txt", std::ios::out | std::ios::trunc);
    if(!passagesStream)
    {
        std::cout<<"Error opening Passages.txt";
        return 1;
    }
    if(!runsStream)
    {
        std::cout<<"Error opening Runs.txt";
        return 1;
    }

    // Loads the highway from the Data folder and checks that it is valid as required by the assignment.
    Highway highway;
    try
    {
        highway = Highway(std::string("../Data/Highway.txt"));
    }
    catch(const std::exception& e)
    {
        std::cout<<"Error while reading Highway.txt: " << e.what() << '\n';
        return 1;
    }
    if(!highway.validate())
    {
        std::cout<<"Invalid highway";
        return 1;
    }

    // Keeps track of vehicles' entrance time, since each vehicle must start some interval after the previous one.
    double currentEntranceTime=0;

    // Iterates over all vehicles; std::max avoids a division by zero in %progress.
    // Progress can exceed 100 (e.g. 110) if numberOfVehicles < 100 and isn't a multiple of 10.
    // These are cases well below the target numberOfVehicles of 10000, so I didn't consider it an issue.
    int progress=std::max(1,numberOfVehicles/10);
    std::cout<<"Simulation started"<<std::endl;
    for(int i=0; i<numberOfVehicles; i++)
    {
        if(i%progress==0)
        {
            std::cout<<"Progress: "<<(i/progress*10)<<"%"<<std::endl;
        }

        // Updates the counter by adding the interval; this becomes the current vehicle's entrance time.
        currentEntranceTime+=randBetweenDouble(minEntranceTime, maxEntranceTime, rng);
        double entranceTime=currentEntranceTime;

        // Draws the entrance and exit junctions; junction ids start at 1, so entrance ranges from the first to the second-to-last.
        int entrance=randBetweenInt(1,highway.junctionsSize()-1, rng);

        // The exit, instead, is drawn from the one after entrance up to the last junction.
        int exit=randBetweenInt(entrance+1,highway.junctionsSize(), rng);

        // Generates the plate, also checking for duplicates.
        std::string plate;
        do{
            plate="";
            plate += static_cast<char>('A' + randBetweenInt(0, 25, rng));
            plate += static_cast<char>('A' + randBetweenInt(0, 25, rng));
            plate += static_cast<char>('0' + randBetweenInt(0, 9, rng));
            plate += static_cast<char>('0' + randBetweenInt(0, 9, rng));
            plate += static_cast<char>('0' + randBetweenInt(0, 9, rng));
            plate += static_cast<char>('A' + randBetweenInt(0, 25, rng));
            plate += static_cast<char>('A' + randBetweenInt(0, 25, rng));
        }while (plateCheck.find(plate)!=plateCheck.end());
        plateCheck.insert(plate);

        // Distance travelled so far, relative to the entrance junction.
        double distanceTravelled=0;

        // Time elapsed since the entrance junction.
        double timePassed=0;

        // Distance to travel between the two junctions (in meters).

        double entranceKm=highway.getJunction(entrance);
        double exitKm=highway.getJunction(exit);
        if(entranceKm <0 || exitKm <0)
        {
            std::cout<<"Something went wrong with the random generation; it shouldn't be possible to draw an invalid entrance or exit ID";
        }
        double distanceToTravel=(exitKm-entranceKm) *1000;

        // Next checkpoint that will be encountered along the trip.
        int currentCheckpoint=1;

        // Finds the first checkpoint (if any) located after the entrance junction.
        while(currentCheckpoint<highway.checkpointsSize() +1  && highway.getCheckpoint(currentCheckpoint)<entranceKm)
        {
            currentCheckpoint+=1;
        }

        // Initializes the vehicle's speed profile as a vector of Interval.
        std::vector<Interval> speedProfile=std::vector<Interval>();

        // Generates speed intervals until the exit junction is reached.
        while(distanceTravelled<distanceToTravel)
        {
            Interval interval{};
            // The assignment requires the speed in km/h as an int.
            int speedkmh = randBetweenInt(minVel,maxVel,rng);
            interval.v=speedkmh;

            // The assignment requires the time in minutes as an int.
            int timeMinutes = randBetweenInt(minTime,maxTime,rng);
            interval.t=timeMinutes;

            // Adds the interval to the speed profile.
            speedProfile.push_back(interval);

            // To keep the calculations correct we need to convert the speed to m/s and the time to seconds.
            double speed=speedkmh/3.6;
            double time=timeMinutes*60;
            double distance=speed*time;

            // The first condition makes sure we stay within the existing checkpoints' range; the second checks
            // that the next checkpoint doesn't go past the vehicle's exit (this can happen because, per the
            // assignment, the last speed interval can go past the exit, but it must not be checked against
            // further checkpoints); the third checks whether, within the interval just generated, a checkpoint
            // is crossed, meaning we need to record the speed there. A while loop (rather than an if) is used
            // because a single interval could cross more than one checkpoint.
            while(currentCheckpoint<highway.checkpointsSize() +1 &&
            highway.getCheckpoint(currentCheckpoint)<exitKm &&
            ((highway.getCheckpoint(currentCheckpoint)-entranceKm)*1000)<distanceTravelled+distance)
            {
                // Computes the exact instant at which the vehicle reaches the checkpoint.

                // spaceBetweenCJ is the checkpoint's distance relative to the entrance junction.
                double spaceBetweenCJ=(highway.getCheckpoint(currentCheckpoint)-entranceKm)*1000;

                // The time is computed as the vehicle's starting time, plus the time already elapsed in
                // previous intervals, plus the time given by the distance between the vehicle's position at
                // the start of this interval and the checkpoint, divided by the speed of this interval.
                // (deltaS/deltaT)
                double timeOfCheckpoint=entranceTime + timePassed+(spaceBetweenCJ-distanceTravelled)/speed;

                // Write to the passages file.
                PrintToPassages(passagesStream,currentCheckpoint,plate,timeOfCheckpoint);

                // Advance the next-checkpoint counter.
                currentCheckpoint+=1;
            }
            // Add the distance and time covered in this interval to the totals accumulated so far.
            distanceTravelled+=distance;
            timePassed+=time;

        }
        // Once the speed profile is complete, the vehicle is written to the runs file.
        PrintToRuns(runsStream,plate,entrance,exit,entranceTime,speedProfile);
    }
    std::cout<<"Simulation completed"<<std::endl;
    // Close the files.
    runsStream.close();
    passagesStream.close();
}
