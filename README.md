# obcpp

The `obcpp` is the repository for our `Onboard Computer`, which is currently a Jetson Orin Nano. This is the device that will actually be running software inside of the plane during flight, so it is essential that it works efficiently and without error.

(Thankfully the pixhawk is a completely separate piece of hardware, so if this code crashes the plane will not immediately crash, but let's try not to do that!)

## Modules

### Airdrop

This module defines a series of functions that communicate over GPIO to trigger the airdrop mechanism to either swap bottles or release the current bottle.

### Camera

This module provides the functionality to interface with all of our physical cameras. This module is designed around one general camera "Interface" for which we provide various implementations for the different specific hardwares.

### Core

This module provides the main functionality of the program's state machine, facilitating the progress of the mission

### CV

This module encapsulates the entire computer vision pipeline, providing an interface which allows images to be input and identified targets to be output.

### Network

This module handles the various communications with other parts of the larger system. The current main two links are with the Ground Control Station via an HTTP server and a serial connection with the plane over which we send Mavlink messages.

### Pathing

This module implements the RRT* algorithm to plan out smart paths in order to navigate through competition waypoints, plan an airdrop approach path, and avoid other planes flying in the sky.

### Utilities

This module provides various helper types and classes used throughout the OBC, as well as some mission-related constants.

## Build Requirements

You will need:

1. `gcc/g++` (a version capable of compiling C++20 code)
2. `cmake` (minimum version 3.10)

First, check if you alread have these installed.

```title="g++"
<tyler obcpp> $ g++ --version
g++ (GCC) 12.2.1 20221121 (Red Hat 12.2.1-4)
Copyright (C) 2022 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

```title="cmake"
<tyler obcpp> $ cmake --version
cmake version 3.26.3

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

If you receive output like this, then that program is already installed. If it says something along the lines of "command not found" then you will need to install it.

### Ubuntu

To install `gcc`/`g++` on Ubuntu, run the following command:

```sh
sudo apt install build-essential
```

To install `cmake` on Ubuntu, run the following command:

```sh
sudo apt install cmake
```

### MacOS

First, you will need `homebrew` installed. You can verify this by typing `brew` into your terminal and reading the output. If it is not installed, you can install it by following [these](https://mac.install.guide/homebrew/3.html) instructions.

To install `gcc`/`g++` on MacOS, run the following command:

```sh
xcode-select --install
```

To install `cmake` on MacOS, run the following command:

```sh
brew install cmake
```

### Other Operating Systems

For other operating systems, you should be able to find instructions online. Make sure to verify the versions are new enough.

## Setup

Now that everything is installed, here is the process to build and run the application

1. Clone the repo
    ```sh
    git clone git@github.com:tritonuas/obcpp.git
    ```

2. Navigate into the directory
    ```sh
    cd obcpp
    ```

3. Run cmake (verify you are at the root level of the repo)
    ```sh
    cmake .
    ```

4. Run make (do this whenever you change code)
    ```sh
    make
    ```

5. Run the generated executable
    ```sh
    bin/obcpp
    ```

## Tests

To run tests, run the following command:

```
ctest .
```

## Modifying `CMakeLists.txt`

The `CMakeLists.txt` file tells `cmake` how to build the program. It will need to be modified if any of the following occurs:

1. A new `.cpp` file is created
2. A `.cpp` file is renamed
3. A new module is created

Each module has its own folder in `include/` and `src/`. Currently all of the header files that we expect to need are planned out, but many do not have accompanying source files. As we add these source files, new libraries will need to be added to the CMake file. You can follow the example for the libraries already included to make this change.

## Style

We are still deciding on a linting/style pipeline. When this is decided, this section should be updated.