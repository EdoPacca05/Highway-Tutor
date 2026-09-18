#include "highway.h"

#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <cmath>

// Passage structure representing a vehicle's transit through a checkpoint.
struct Passage {
    int checkpointId;
    std::string plate;
    double time;

    Passage(int c, const std::string& p, double t){
        checkpointId=c;
        plate=p;
        time=t;
    }
    Passage(){}
};

// Structure representing a speeding violation.
struct Violation
{
    std::string plate;
    int entranceId, exitId;
    double averageSpeed;
    double timeStart, timeEnd;

    Violation(){
        plate="";
        entranceId=0;
        exitId=0;
        averageSpeed=0;
        timeStart=0;
        timeEnd=0;
    }

    Violation(const std::string& p, int en, int ex, double as, double ts, double te){
        plate=p;
        entranceId=en;
        exitId=ex;
        averageSpeed=as;
        timeStart=ts;
        timeEnd=te;
    }
    void print() const
    {
        std::cout<<plate<<" "<<entranceId<<" "<<exitId<<" "<<averageSpeed<<" "<<timeStart<<" "<<timeEnd<<std::endl;
    }
};

int main()
{
    // Speed limit constant, as defined by the assignment.
    const double speedLimit=130/3.6;

    std::cout<<"AVAILABLE COMMANDS"<<
    std::endl<<"set_time <instant>: move to a new point in time and process all transits between the current instant and the given one"<<
    std::endl<<"reset: fully resets the system"<<
    std::endl<<"stats: prints the statistics"<<
    std::endl<<"exit/quit: terminates the program"<<std::endl;


    // Loads the highway from the Data folder, checks that the file opens correctly and that the highway is valid as required by the assignment.
    Highway highway;
    try
    {
        highway = Highway(std::string("../Data/Highway.txt"));
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
        return 1;
    }
    if(!highway.validate())
    {
        std::cout<<"Invalid highway";
        return 1;
    }

    // Control variable used to stop the program.
    bool wantsToExit=false;
    // Variable that keeps track of the elapsed time.
    double currentTime=0;

    // Vectors holding the violations and the vehicles' checkpoint transits.
    std::vector<Violation> violations;
    std::vector<Passage> allPassages;

    // Loads the passages file from the Data folder and checks that it opens correctly.
    std::ifstream passagesFile = std::ifstream("../Data/Passages.txt");
    if(!passagesFile)
    {
        std::cout<<"Error opening the passages file";
        return 1;
    }

    // Variables making up a single passage record.
    int checkpointId;
    std::string plate;
    double time;

    // Variables used to compute the average speed.
    int nMeasuredSpeed=0;      // number of measured speeds
    double currentSpeedSum=0;  // sum of all measured speeds

    // Syntax found online to extract the passage fields from the file.
    while (passagesFile >> checkpointId >> plate >> time) {
        allPassages.push_back(Passage(checkpointId, plate, time));
    }
    // Sort the passages using the STL sort algorithm with a lambda expression.
    std::sort(allPassages.begin(), allPassages.end(), [](const Passage& a, const Passage& b) { return a.time < b.time; });
    passagesFile.close();

    // Variable keeping track of how far we've processed the passages vector.
    int nextPassage=0;
    // Map used to look up the last recorded passage for each vehicle.
    std::map<std::string, Passage> lastByPlate;

    // Vector holding the number of transits for each checkpoint up to the current time.
    std::vector<int> checkpointCounter(highway.checkpointsSize(), 0);

    // Main loop reading commands from the user.
    while(!wantsToExit)
    {
        // Read the user's command into the 'command' string.
        std::string command;
        std::getline(std::cin, command);

        // 'reset' command.
        if(command=="reset")
        {
            // Reset everything back to the initial state.
            currentTime=0;
            violations.clear();
            nextPassage=0;
            lastByPlate.clear();
            nMeasuredSpeed=0;
            currentSpeedSum=0;
            checkpointCounter.assign(highway.checkpointsSize(), 0);
            std::cout<<"AVAILABLE COMMANDS"<<std::endl<<"set_time <instant>: move to a new point in time and process all transits between the current instant and the given one"<<
            std::endl<<"reset: fully resets the system"<<std::endl<<"stats: prints the statistics"<<std::endl<<"exit/quit: terminates the program"<<std::endl;
        }
        // 'stats' command.
        else if (command == "stats")
        {
            // Print the information required by the assignment.
            for (int i = 0; i < highway.checkpointsSize(); ++i)
            {
                std::cout<<"Checkpoint "<<i+1<<": "<<checkpointCounter[i]<<", average value: ";
                // Guard against division by zero.
                if(currentTime==0)
                {
                    std::cout<<0<<std::endl;
                }
                else
                {
                    std::cout<<checkpointCounter[i]/currentTime*60<<std::endl;
                }
            }
            // Guard against division by zero.
            if(nMeasuredSpeed==0)
            {
                std::cout<<"Average speed: 0"<<std::endl;
            }
            else
            {
                std::cout<<"Average speed: "<<currentSpeedSum/nMeasuredSpeed *3.6<<std::endl;
            }
            std::cout<<"Number of violations: "<<violations.size()<<std::endl;
        }
        // 'set_time' command.
        else if(command.substr(0,9)=="set_time ")
        {
            double time;
            try
            {
                // Used for the time conversion, to prevent trailing letters after the numeric value from
                // being silently ignored. charactersConverted counts how many characters were converted to
                // a number; if it doesn't match the length of the command's argument, the user is told the
                // command is invalid. This prevents commands like "set_time 30m " (note the trailing space)
                // where std::stod would convert "30m" to 30 and it would be interpreted as seconds instead
                // of minutes. To avoid ambiguity, we chose to treat the command in that example as invalid.
                std::size_t charactersConverted;
                std::string subCommand;
                // Time given in minutes.
                if(command.at(command.size()-1)=='m')
                {
                    subCommand=command.substr(9, command.size() - 10);
                    time = std::stod(subCommand,&charactersConverted);
                    if (charactersConverted < subCommand.size())
                    {
                        std::cout<<"Error converting the time value, please re-enter a valid command"<<std::endl;
                        continue;
                    }
                    // Multiply by 60 to get delta_sec in seconds.
                    time*=60;
                }
                // Time given in seconds.
                else
                {
                    subCommand=command.substr(9);
                    time=std::stod(subCommand,&charactersConverted);
                    if (charactersConverted < subCommand.size())
                    {
                        std::cout<<"Error converting the time value, please re-enter a valid command"<<std::endl;
                        continue;
                    }
                }
                // Guard against a negative time value.
                if(time < 0)
                {
                    std::cout<<"Please enter a positive value"<<std::endl;
                    continue;
                }
            }
            // Catches the exception thrown for an invalid format.
            catch(std::invalid_argument exc)
            {
                std::cout<<"Error converting the time value, please re-enter a valid command"<<std::endl;
                continue;
            }
            // Catches the exception thrown on out-of-range conversion.
            catch(std::out_of_range exc)
            {
                std::cout<<"Error converting the time value, please re-enter a valid command"<<std::endl;
                continue;
            }

            // Keep track of the time before and after set_time (used to recompute the average speed).
            double oldTime = currentTime;
            currentTime += time;

            // Iterate over all passages that occurred between the previous and the new instant.
            int i;
            for (i = nextPassage; i < allPassages.size(); i++)
            {
                // Check whether the current passage falls within the current set_time window; otherwise stop the loop.
                if (allPassages[i].time > currentTime) break;

                // Extract the passage we're currently processing.
                Passage current = allPassages[i];
                // Record the transit in checkpointCounter for the corresponding checkpoint.
                checkpointCounter[current.checkpointId-1]++;

                // If the vehicle already passed through an earlier checkpoint, compute its average speed.
                if (lastByPlate.find(current.plate) != lastByPlate.end())
                {
                    // Look up the vehicle's previous passage via the map.
                    Passage prev = lastByPlate[current.plate];

                    // Get the positions of the two checkpoints the vehicle passed through.
                    double c1 = highway.getCheckpoint(prev.checkpointId);
                    double c2 = highway.getCheckpoint(current.checkpointId);

                    // Check that they exist.
                    if (c1>0 && c2>0)
                    {
                        // Compute the distance between them.
                        double distanceKm = c2 - c1;

                        // Compute the vehicle's average speed between the two checkpoints.
                        double speedMs = distanceKm / (current.time - prev.time) * 1000;
                        nMeasuredSpeed++;
                        currentSpeedSum+=speedMs;

                        // Check for a violation; if present, create it, print it, and store it in the vector.
                        if (speedMs > speedLimit)
                        {
                            Violation v=Violation(current.plate,prev.checkpointId,current.checkpointId,round(speedMs*3.6),prev.time,current.time);
                            v.print();
                            violations.push_back(v);
                        }
                    }

                }

                // Insert the current passage into the map, overwriting any previous entry for this plate.
                lastByPlate[current.plate] = current;
            }
            // Update the index of the next passage to process.
            nextPassage=i;
        }
        // 'exit' command we added ourselves to terminate the program.
        else if (command == "exit" || command == "quit")
        {
            wantsToExit = true;
        }
        // Handle an unrecognized command.
        else
        {
            std::cout<<"Unrecognized command, please enter a valid command"<<std::endl;
            continue;
        }
    }
}
