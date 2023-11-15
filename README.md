# obcpp

The `obcpp` is the repository for our `Onboard Computer`, which is currently a Jetson Orin Nano. This is the device that will actually be running software inside of the plane during flight, so it is essential that it works efficiently and without error.

(Thankfully the pixhawk is a completely separate piece of hardware, so if this code crashes the plane will not immediately crash, but let's try not to do that!)

## Quick Setup

See [full setup](https://github.com/tritonuas/obcpp#setup) below.

```
git clone git@github.com:tritonuas/obcpp.git
cd obcpp
code .      # once in VSCode, use the Devcontainers extension to
            # launch our Devcontainer environment (using Docker)

mkdir build && cd build
cmake ..    # configures the CMake build system
```

Now you can use our Make targets.

- `make obcpp`: Makes the binary which you can run using `./bin/obcpp`
- `make test`: Run the tests in `tests/unit`
- `make playground`: Runs the `tests/integration/playground.cpp` test which makes sure all dependencies work correctly
- `make lint`: Check code for problems using `clang-tidy`

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

## Docker

Everyone that works on this project is strongly recommended to work inside of a Docker container. This will allow us to all work on the same underlying hardware, and even let people develop straight from Windows.

To start, you will need to install Docker. You can follow the instructions [here](https://docs.docker.com/get-docker/)

## Setup

Now that everything is installed, here is the process to build and run the application. These instructions are for VSCode because it provides very nice integration with Docker containers. If you absolultely do not want to use VSCode, then you will have to find an equivalent way to do this with the IDE you want to use.

1. Clone the repo. If you are using git from the command line, this is the command you will use.
    ```sh
    git clone git@github.com:tritonuas/obcpp.git
    ```

2. Navigate into the directory
    ```sh
    cd obcpp
    ```

3. Verify Docker is installed correctly by entering the following command into your terminal:
    ```sh
    docker run hello-world
    ```
    If everything works correctly, you should receive a message saying that everything worked correctly. If instead you get
    a message saying that Docker was not recognized, then something went wrong in your installation. If restarting your 
    computer does not fix it, then you should try and refollow the installation instructions again, verifying that you
    didn't make any mistakes.

4. Open the project's directory in VSCode (Make sure you are still in the `obcpp` directory, otherwise you will open
   your current directory in vscode, whatever that may be.)
    ```sh
    code .
    ```

5. Download the following extensions:
    1. [Remote-SSH](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-ssh)
    2. [Remote Explorer](https://marketplace.visualstudio.com/items?itemName=ms-vscode.remote-explorer)
    3. [Docker](https://marketplace.visualstudio.com/items?itemName=ms-azuretools.vscode-docker)
    4. [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
    5. [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)

6. Build the Docker Container:
    1. In the bottom left of the screen, click on the remote window icon. It should look like two angle brackets next to each other.
    2. Select reopen in container.
    3. Select "From Dockerfile"
    4. Select Skip Features
    5. Wait for several minutes while the container builds. You will only need to do this once.

    _Note: you will probably want to enable some of your extensions, especially the C/C++ one, to be available inside of the container.
    To do this, just navigate back to the extensions page, search for whichever extension you want to use inside of the container,
    and click the button that says "Install in dev container"._

7. If the container was successfully loaded, then in the terminal you should see something along the lines of 
    ```sh
    root@d26aba74c8cd:/workspaces/obcpp# 
    ```

8. Create a build directory and enter it (All the following commands should be run from inside the build directory)
    ```sh
    mkdir build
    cd build
    ```

8. Build CMake files with the following command:
    ```sh
    cmake ..
    ```

9. Build executable with the following command. (You will need to do this anytime you edit code.)
    ```sh
    make
    ```

10. Run the generated executable to verify it was created correctly.
    ```sh
    bin/obcpp
    ```

11. Verify that the testing framework is set up correctly
    ```sh
    make test
    ```

    If you receive output about certain tests passing/failing, then you are good. Ideally the main branch (which should be what
    you cloned) won't have any failing tests, but if there are failing tests then it isn't your fault, nor does it mean your
    installation is messed up.

With that, everything should be set up correctly, and you should be good to go.

## Modifying `CMakeLists.txt`

The `CMakeLists.txt` file tells `cmake` how to build the program. It will need to be modified if any of the following occurs:

1. A new `.cpp` file is created
2. A `.cpp` file is renamed
3. A new module is created

Each module has its own folder in `include/` and `src/`. Currently all of the header files that we expect to need are planned out, but many do not have accompanying source files. As we add these source files, new libraries will need to be added to the CMake file. You can follow the example for the libraries already included to make this change.

## Style

### Linting

[`clang-tidy`](https://clang.llvm.org/extra/clang-tidy/) is the linter that statically analyzes the code for style issues and other errors.

#### Setup
If you're using the Devcontainer, `clang-tidy` will already be installed.

If you're working outside the container, install it on your host system.

For Ubuntu/Debian Linux distributions:
```sh
sudo apt-get install clang-tidy
```

For macOS, try the proposed solution on this post: https://stackoverflow.com/questions/53111082/how-to-install-clang-tidy-on-macos. If anyone on macOS has an easier solution, feel free to replace this section in a PR.

#### Usage
To run the linter locally:

```sh
make lint
```

### Formatting

No formatter has been added yet. Formatting will be enforced once one is set up.