# Highway Speed Tutor

![C++](https://img.shields.io/badge/language-C%2B%2B11-blue)
![CMake](https://img.shields.io/badge/build-CMake-informational)

A C++ simulation of an Italian-style average-speed traffic enforcement system ("Tutor") for highways. The project was developed for the *Laboratorio di Programmazione* (Programming Lab) course at the University of Padua, School of Computer Engineering.

It's made of two independent command-line programs:

- **Simulator** — generates random vehicle trips on a highway and produces the raw data of when each vehicle crosses each checkpoint.
- **Tutor** — replays those checkpoint crossings against a virtual clock, computes each vehicle's average speed between consecutive checkpoints, and reports drivers exceeding the speed limit.

## How it works

A highway is modeled as a sequence of **junctions** (entry/exit points, `S`) and **checkpoints** (speed cameras, `V`), each placed at a given distance (km) from the origin. Vehicles enter at one junction, drive at a randomly generated speed profile, and exit at a later junction. Every time a vehicle crosses a checkpoint, a timestamped record is produced. The Tutor uses the timestamps of two consecutive checkpoints to compute a vehicle's average speed over that stretch of highway and flags it if it exceeds 130 km/h.

## Project structure

```
.
├── CMakeLists.txt
├── README.md
├── Data/
│   └── Highway.txt      # highway layout: junctions & checkpoints
├── include/
│   └── highway.h        # Highway class interface
└── src/
    ├── highway.cpp       # Highway class implementation
    ├── simulator.cpp     # Simulator executable
    └── tutor.cpp         # Tutor executable
```

## Highway layout format (`Data/Highway.txt`)

Each line follows:

```
<distance in km> <V|S>
```

where `V` marks a checkpoint and `S` marks a junction. IDs are assigned automatically and independently for checkpoints and junctions, in increasing order of distance from the origin (checkpoint 1, junction 1, etc. both exist). The layout is validated against the following rules:

- at least two checkpoints;
- at least one junction before the first checkpoint and one after the last checkpoint;
- a minimum distance of 1 km between any junction and any checkpoint;
- entries can be listed in any order in the file.

Both executables refuse to start and exit with an error if `Data/Highway.txt` is missing or violates any of these constraints.

## Building

Requires CMake ≥ 3.6 and a C++11 compiler.

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

This produces the `Simulator` and `Tutor` executables. Both read/write files using the relative path `../Data/...`, so run them from inside the `build/` directory (i.e. one level below the project root, where `Data/` lives).

## Usage

### 1. Run the simulator

```bash
./Simulator
```

It generates a constant number of vehicles (`numberOfVehicles`, default `10000`), each with:

- a random license plate (`LL999LL` format);
- a random entrance and exit junction;
- a random entrance time (0.5–10.0 s after the previous vehicle);
- a random speed profile made of consecutive constant-speed intervals (80–190 km/h, lasting 5–15 minutes each).

Output files (written to `Data/`):

- **`Runs.txt`** — one line per vehicle: plate, entrance/exit junction IDs, entrance time, and the full list of `(speed, duration)` intervals.
- **`Passages.txt`** — one line per checkpoint crossing: checkpoint ID, plate, timestamp. This is the input consumed by the Tutor.

### 2. Run the tutor

```bash
./Tutor
```

The Tutor starts at time 0 and waits for commands on standard input:

| Command | Description |
|---|---|
| `set_time <t>` | Advances the internal clock by `<t>` seconds, or `<t>m` for minutes (e.g. `set_time 20m`), and processes every checkpoint crossing that happened in between, printing any speed violation found. |
| `stats` | Prints, for each checkpoint, the total number of transits and the average number of transits per minute; the overall average speed of all vehicles measured so far; and the total number of violations. |
| `reset` | Resets the system to its initial state (clock, statistics, and violation history). |
| `exit` / `quit` | Terminates the program. |

Each violation is printed as:

```
<plate> <entrance checkpoint id> <exit checkpoint id> <average speed km/h> <time at entrance> <time at exit>
```

## Implementation notes

- `Highway` (`include/highway.h`, `src/highway.cpp`) is responsible for parsing and validating `Highway.txt`, exposing checkpoints and junctions by their 1-based ID.
- The Tutor loads and sorts all checkpoint passages once, then processes them incrementally as the clock advances with `set_time`, instead of rescanning the whole dataset on every command.
- The running average speed is tracked with a sum and a counter rather than storing every individual measurement, to keep memory usage low with a large number of vehicles.

## Authors

Edoardo Paccagnella

## License

Educational project developed for a university course. No license has been specified; all rights remain with the authors.
