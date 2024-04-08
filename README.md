# obcpp

The `obcpp` is the repository for our `Onboard Computer`, which is currently a Jetson Orin Nano. This is the device that will actually be running software inside of the plane during flight, so it is essential that it works efficiently and without error.

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

Now you can use our build targets.

- `ninja obcpp`: Makes the binary which you can run using `./bin/obcpp`
- `ninja test`: Run the tests in `tests/unit`
- `ninja playground`: Runs the `tests/integration/playground.cpp` test which makes sure all dependencies work correctly
- `ninja lint`: Check code for problems using `cpplint`

## A Note on Ninja

Ninja, the build tool we are using, tries to use a number of cores on your system based on how more cores your CPU has. It is possible that it will attempt to use too many cores, and you will run out of memory on your system and everything will freeze up.

To solve this, in our CMake config we limit the number of core CMake can use to 4. This hasn't crashed anyone's computer so far, but it is possible that you may need to reduce this number, or perhaps you want to increase it if your system can handle it so that you can get faster build times.

To change the number of cores, you can run `ninja` with the following flag.
```
ninja -j [# cores]
```

Anecdotally, on a machine with 16 virtual cores and 16GB of RAM, `-j 8` appears to be a good balance between speed and resources usage.

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

<details>
  <summary>6. Optional steps. Only needed if you get auth issues pulling devcontainer.</summary>
            
6. Authenticate Docker
    1. Go to Github
    2. Click on your profile icon in the top right
    3. Select "Settings"
    4. Select "Developer Settings" (At the moment of writing this, it is the bottom-most option on the left sidebar).
    5. Select "Personal access tokens"
    6. Select "Tokens (classic)"
    7. Select "Generate new token", then select classic again.
    8. Give it a name, expiration date, and then select "read:packages"
    9. Generate the token and save it to your clipboard
    10. Go to a terminal and enter 
        ```sh
        export CR_PAT=YOUR_TOKEN
        ``` 
        replacing YOUR_TOKEN with your token.

    11. In the same terminal, enter 
        ```sh
        sudo echo $CR_PAT | docker login ghcr.io -u USERNAME --password-stdin
        ```
        replacing USERNAME with your Github username.

    12. If you get an error along the lines of "Error Saving Credentials", you may need to run these commands:
        ```sh
        service docker stop
        rm ~/.docker/config.json
        ```
        And then you can rerun the command from step 11. On a Windows system the filepath may be different.

    13. This should authenticate your Docker so that you can now pull containers from the github container registry. You may receive a warning saying that your token is being stored unencrypted in the `~/.docker/config.json` file. If this concerns you, you can follow the link provided and fix this.
</details>

7. Pull the Docker Container:
    1. In the bottom left of the screen, click on the remote window icon. It should look like two angle brackets next to each other. Also, you can instead press "Ctrl+Shift+P"
    2. Select (or type) reopen in container.


8. If the container was successfully loaded, then in the terminal you should see something along the lines of 
    ```sh
    tuas@6178f65ec6e2:/workspaces/obcpp$ 
    ```

9. Create a build directory and enter it (All the following commands should be run from inside the build directory)
    ```sh
    mkdir build
    cd build
    ```

10. Build CMake files with the following command:
    ```sh
    cmake ..
    ```

11. Build executable with the following command. (You will need to do this anytime you edit code.)
    ```sh
    ninja obcpp 
    ```

12. Run the generated executable to verify it was created correctly.
    ```sh
    bin/obcpp
    ```

13. Verify that the testing framework is set up correctly
    ```sh
    ninja test
    ```

    If you receive output about certain tests passing/failing, then you are good. Ideally the main branch (which should be what
    you cloned) won't have any failing tests, but if there are failing tests then it isn't your fault, nor does it mean your
    installation is messed up.

With that, everything should be set up correctly, and you should be good to go.

## Modifying `CMakeLists.txt`

There is a `CMakeLists.txt` folder inside of each of the obcpp's module directories. If you add a new file to, say, the `network` module inside of `src/network/`, then you will need to add that filename to `src/network/CMakeLists.txt`.

Note: you may need to clear you CMake cache if things get messed up.
`find -name "*Cache.txt" -delete`

## Style

### Linting

[`cpplint`](https://github.com/cpplint/cpplint) is the linter that statically analyzes the code for style issues and other errors. It follows [Google's C++ style guide](http://google.github.io/styleguide/cppguide.html).

#### Setup
If you're using the Devcontainer, `cpplint` will already be installed.

If you're working outside the container, install it on your host system.

For Ubuntu/Debian Linux distributions:
```sh
sudo apt-get install python3-pip
pip install cpplint
```

For macOS, ensure you have `pip` installed or download it using one of the methods [here](https://pip.pypa.io/en/stable/installation/).

Then, run:
```sh
pip install cpplint
```

Or with brew:
```sh
brew install cpplint
```

#### Usage
To run the linter locally:

```sh
make lint
```

#### Best Practices

Normally we want to fix every lint error that comes up, but in some cases it doesn't make sense to fix them all. To ignore linting for a specific line, add the following nolint comment as shown:

```cpp
int x = 0; // NOLINT
```

### Formatting

No formatter has been added yet. Formatting will be enforced once one is set up.
