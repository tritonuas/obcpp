# obcpp

The `obcpp` is the repository for our `Onboard Computer`, which is currently a Jetson Orin Nano. This is the device that will actually be running software inside of the plane during flight, so it is essential that it works efficiently and without error.

Everyone that works on this project is strongly recommended to work inside of a Docker container. This will allow us to all work on the same underlying hardware, and even let people develop straight from Windows.

To start, you will need to install Docker. You can follow the instructions [here](https://docs.docker.com/get-docker/)

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

It is important to run `cmake` from within the `build` directory as shown so that all of the random build files CMake generates get placed in the `build` directory. Don't worry about messing this up, however, because the `cmake` will error and tell you that you should run it from within the `build` directory.

## Command Line Usage

```
obcpp [config_directory] [config_type] [plane] [flight_type]
```

- `config_directory`: path to the configs directory, relative to the current working directory
    - This should almost always refer to the `configs` directory at the root of the repository
- `config_file`: name of the config file (without the `.json` file extension) to use
    - Currently this should either be `dev` for local development from inside the dev container, or `jetson` if you are running it on the Jetson
- `plane`: name of the plane the obcpp is running on.
    - This is used to determine what mavlink parameters should be uploaded to the plane. The only currently valid input is `stickbug`
- `flight_type`: what kind of flight is being performed
    - This is used to further specify what mavlink parameters should be uploaded to the plane. Current valid values are `sitl`, `test-flight`, or `competition`.

An example, running the program from the `build` directory on the sitl.

```
./bin/obcpp ../configs dev stickbug sitl
```

## Config Files

The repo currently has the following file structure for configuration files. There are two kinds of configuration files: the high level config files (currently `dev.json` and `jetson.json`) and lower level mavlink parameter config files, which are kept
under the `params` directory as shown below.

```
configs |>
    - dev.json
    - jetson.json
    params |>
        stickbug |>
            - common.json
            - competition.json
            - sitl.json
            - test-flight.json
        plane_name |>
            - common.json
            - ...
```

To add support for a new plane, which may require a unique set of mavlink parameters to upload, you should create a new directory adjacent to the `stickbug` directory. Inside of this directory you must have a file titled `common.json` for mavlink parameters that should always be uploaded when this plane is being used. Adjacent to `common.json`, however, you can create as many different json files for more specific parameters that should be uploaded to the plane in specific circumstances.

For example, `competition.json` sets the correct magic number for the Advanced Failsafe System to ensure that the plane will crash itself after 3 minutes of lost communications, in order to comply with SUAS competition rules. However, we really don't want this at a test flight so `test-flight.json` does not set the required magic number for the flight termination to be carried out.

## Jetson Setup

To run everything correctly on the Jetson (and possibly your local computer if you are communicating with the airdrop payloads), you will need to make sure that the `jetspot` wifi hotspot network is online, and that `obcpp` is set to use the correct network interface.

The following commands must be run every time the Jetson is booted to ensure that the networking with the airdrop payloads works
correctly:

```sh
sudo nmcli connection up Hotspot # turn on the jetspot WIFI network for the payloads to connect to
sudo route add default gw 10.42.0.1 # make the default interface the jetspot WIFI hotspot
```

The `jetspot` WIFI network is currently set up like so

SSID: `jetspot`
password: `fortnite`

The airdrop payloads are currently programmed to connect to a WIFI network with this information on boot. To make sure that it is broadcasting correctly, you can try and connect to it via your phone or other device.

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
    ./bin/obcpp ../configs dev stickbug sitl
    ```

13. Verify that the testing framework is set up correctly
    ```sh
    ninja test
    ```

    If you receive output about certain tests passing/failing, then you are good. Ideally the main branch (which should be what
    you cloned) won't have any failing tests, but if there are failing tests then it isn't your fault, nor does it mean your
    installation is messed up.

With that, everything should be set up correctly, and you should be good to go.

## Limiting Cores in Ninja

Ninja, the build tool we are using, tries to use a number of cores on your system based on how more cores your CPU has. For our repo, it seems like this default almost always crashes/freezes your computer because it quickly runs out of memory.

To solve this, in our CMake config we limit the number of core ninja can use to 4. This hasn't crashed anyone's computer so far, but it is possible that you may need to reduce this number, or perhaps you want to increase it if your system can handle it so that you can get faster build times.

To change the number of cores, you have to pass a special flag when you run `cmake`, like so:
```
cmake -D CMAKE_JOB_POOLS="j=[# jobs]" ..
```
where you replace `[# jobs]` with a number specifying the number of jobs.

If you do this once, CMake should remember how you specified it, so as long as you don't clear the CMake cache you won't need to enter this again. (I.e. you can just run `cmake ..` and you should still see the message at the top saying that it is using a user-defined number of jobs).

Anecdotally, on a machine with 16 virtual cores and 16GB of RAM, `-D CMAKE_JOB_POOLS="j=8"` appears to be a good balance between speed and resources usage.

## Advanced Testing

### CV Pipeline

To fully test the CV pipeline, you will need to make sure that [not-stolen-israeli-code](https://github.com/tritonuas/not-stolen-israeli-code) is running. The easiest way to do this currently is to run the following commands from the root of the repository:

```sh
cd docker
make run-sitl-compose
```

When testing the pipeline, you will likely want to also use the `mock` camera. The Mock camera works by randomly selecting images from a specified directory in order to simulate "taking" real pictures. The relevant config options are

```json
    "camera": {
        ...
        "type": "mock",
        ...
        "mock": {
            "images_dir": "/workspaces/obcpp/tests/integration/images/saliency/"
        },
```

When `type` is set to `"mock"`, when `obcpp` tries to take a picture, it will instead select a random image from the given directory.

### SITL

In order to fully test `obcpp` with the SITL, you must be on a Linux machine (so that you can use Docker host networking). If this is the case, then you can do the following steps to test the obcpp with the simulated plane.

1. Set up the [gcs](www.github.com/tritonuas/gcs) repo.
2. Run `make run-compose` inside of the `gcs` repo.
3. Run `./bin/obcpp ../configs dev stickbug sitl` inside of `build`

If everything works correctly, you should get output similar to this

```
2024-06-30 05:52:18.893 (   0.003s) [mav connect     ]            mavlink.cpp:32    INFO| Connecting to Mav at tcp://localhost:14552
2024-06-30 05:52:18.893 (   0.003s) [mav connect     ]            mavlink.cpp:35    INFO| Attempting to add mav connection...
2024-06-30 05:52:18.894 (   0.003s) [mav connect     ]            mavlink.cpp:38    INFO| Mavlink connection successfully established at tcp://localhost:14552
2024-06-30 05:52:18.994 (   0.103s) [mav connect     ]            mavlink.cpp:56    INFO| Mavlink heartbeat received
2024-06-30 05:52:18.994 (   0.104s) [mav connect     ]            mavlink.cpp:71    INFO| Setting AFS_TERM_ACTION to 0
2024-06-30 05:52:19.531 (   0.640s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.531 (   0.640s) [mav connect     ]            mavlink.cpp:71    INFO| Setting WP_RADIUS to 7
2024-06-30 05:52:19.582 (   0.691s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.582 (   0.692s) [mav connect     ]            mavlink.cpp:71    INFO| Setting Q_GUIDED_MODE to 1
2024-06-30 05:52:19.633 (   0.742s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.633 (   0.742s) [mav connect     ]            mavlink.cpp:71    INFO| Setting FS_SHORT_ACTN to 0
2024-06-30 05:52:19.684 (   0.793s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.684 (   0.794s) [mav connect     ]            mavlink.cpp:71    INFO| Setting THR_FAILSAFE to 1
2024-06-30 05:52:19.737 (   0.847s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.738 (   0.847s) [mav connect     ]            mavlink.cpp:71    INFO| Setting FS_LONG_TIMEOUT to 30
2024-06-30 05:52:19.757 (   0.866s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.757 (   0.866s) [mav connect     ]            mavlink.cpp:71    INFO| Setting FS_LONG_ACTN to 1
2024-06-30 05:52:19.794 (   0.904s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.794 (   0.904s) [mav connect     ]            mavlink.cpp:71    INFO| Setting AFS_RC_MAN_ONLY to 0
2024-06-30 05:52:19.847 (   0.957s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.848 (   0.957s) [mav connect     ]            mavlink.cpp:71    INFO| Setting AFS_RC_FAIL_TIME to 180
2024-06-30 05:52:19.865 (   0.975s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.866 (   0.975s) [mav connect     ]            mavlink.cpp:71    INFO| Setting FS_SHORT_TIMEOUT to 1
2024-06-30 05:52:19.884 (   0.993s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.884 (   0.994s) [mav connect     ]            mavlink.cpp:71    INFO| Setting AFS_GEOFENCE to 0
2024-06-30 05:52:19.955 (   1.064s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:19.955 (   1.064s) [mav connect     ]            mavlink.cpp:71    INFO| Setting AFS_ENABLE to 1
2024-06-30 05:52:20.008 (   1.117s) [mav connect     ]            mavlink.cpp:96    INFO| Successfully set param
2024-06-30 05:52:20.008 (   1.117s) [mav connect     ]            mavlink.cpp:111   INFO| Attempting to set telemetry polling rate to 2.000000...
2024-06-30 05:52:20.025 (   1.135s) [mav connect     ]            mavlink.cpp:114   INFO| Successfully set mavlink polling rate to 2.000000
2024-06-30 05:52:20.026 (   1.135s) [mav connect     ]            mavlink.cpp:126   INFO| Setting mavlink telemetry subscriptions
2024-06-30 05:52:20.042 (   1.151s) [        6C92D640]            mavlink.cpp:151   INFO| Mav Flight Mode: Unknown
```

You can technically also do this kind of testing on the Jetson itself (where you are running `obcpp` on the Jetson and the SITL either on your local computer or the Jetson itself). If that is the case, you will need to modify some of the IP addresses and ports throughout the `obcpp` and `gcs` configs / docker compose files. This is fairly complicated, so I would recommend asking for help with this.

### Airdrop

If you specifically want to test `obcpp` with real airdrop payloads, then you will need to run some extra commands to make sure the networking is set up correctly. Note that this also requires a Linux machine to take advantage of Docker host networking.

#### Jetson

1. Follow the steps listed in [Jetson Setup](#jetson-setup)
2. `cd docker`
3. `make jetson-develop`
4. Run the program like normal

NOTE: These steps may change eventually when we actually get the Jetson Docker container up and running, so you don't have to use the development container.

#### Dev Container

1. First you will need to have a WIFI network that the payloads can connect to. If you cannot easily modify the code flashed to the airdrop payloads, then the easiest thing to do would be to rename your phone hotspot to `jetspot` with a password of `fortnite` so that the payloads automatically connect to it.
2. Figure out what the IPs on your WIFI network look like. For example, on the INNOUT WIFI network they are of the form `192.168.1.1`, whereas on the `jetspot` WIFI network they are of the form `10.42.0.1`. To figure this out, first connect to the network on the device that you are going to run the `obcpp` and enter the command `ip a` into your terminal. (On Windows, something like `ipconfig` will work). You will want to search through the output of the command and figure out what IP your device is assigned. If the network is a Hotspot hosted by the same device, it will likely be an IP Address ending in `.1` because your device is acting as the router for that particular subnet.
3. Once you know what the IP addresses on the WIFI network look like, you will want to set the default gateway router in your kernel IP routing table to the IP of the router on the network. For example, if I were to test this while connected to my home Spectrum WIFI, after running `ip a` I would get the following output:
    ```
    1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
        link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
        inet 127.0.0.1/8 scope host lo
        valid_lft forever preferred_lft forever
        inet6 ::1/128 scope host 
        valid_lft forever preferred_lft forever
    3: wlp170s0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
        link/ether 74:3a:f4:33:31:fa brd ff:ff:ff:ff:ff:ff
        inet 192.168.1.220/24 brd 192.168.1.255 scope global dynamic noprefixroute wlp170s0
        valid_lft 38671sec preferred_lft 38671sec
        inet6 2603:8001:8d00:d38f::13a7/128 scope global dynamic noprefixroute 
        valid_lft 600274sec preferred_lft 600274sec
        inet6 2603:8001:8d00:d38f:dcad:5e17:a842:ebb9/64 scope global temporary dynamic 
        valid_lft 600274sec preferred_lft 81374sec
        inet6 2603:8001:8d00:d38f:adb9:2b80:b742:88c9/64 scope global dynamic mngtmpaddr noprefixroute 
        valid_lft 604373sec preferred_lft 604373sec
        inet6 fe80::c60e:4fd8:6f39:cc6b/64 scope link noprefixroute 
        valid_lft forever preferred_lft forever
    4: br-f1387e112f99: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
        link/ether 02:42:21:d3:18:21 brd ff:ff:ff:ff:ff:ff
        inet 172.29.0.1/16 brd 172.29.255.255 scope global br-f1387e112f99
        valid_lft forever preferred_lft forever
        inet6 fe80::42:21ff:fed3:1821/64 scope link 
        valid_lft forever preferred_lft forever
    5: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN group default 
        link/ether 02:42:c0:1c:21:ab brd ff:ff:ff:ff:ff:ff
        inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
        valid_lft forever preferred_lft forever
    6: br-a61b867b0916: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN group default 
        link/ether 02:42:d6:97:87:e4 brd ff:ff:ff:ff:ff:ff
        inet 172.21.0.1/16 brd 172.21.255.255 scope global br-a61b867b0916
        valid_lft forever preferred_lft forever
    8: vethcc8c4be@if7: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master br-f1387e112f99 state UP group default 
        link/ether 1a:eb:7a:ea:97:97 brd ff:ff:ff:ff:ff:ff link-netnsid 0
        inet6 fe80::18eb:7aff:feea:9797/64 scope link 
        valid_lft forever preferred_lft forever
    ```

    After parsing this, I would figure out that the IP Addresses on my home WIFI are of the form `192.168.1.X`. This means that the router on my Home WIFI is `192.168.1.1`, which is the IP I care about right now. If the WIFI network is a Hotspot being hosted by the current device, then your device's IP on the network should already be of the form `X.X.X.1`, so you can just use its IP Address directly.

    With this information, run the following command:
    ```sh
    sudo route add default gw 192.168.1.1 # or whatever IP you found above
    ```

    This will make the airdrop packets `obcpp` sends out default to the correct network interface (that the airdrop payloads are on.)
4.  Last but not least, go into the `.devcontainer/devcontainer.json` file and ensure that the line of the form
    ```json
	"runArgs": ["--network=host"],
    ```
    Is not commented out. This ensures that the Docker container is running in host networking mode, and that all of the packets send from within the Docker container are treated as if you sent them directly from your host machine.
    
    If the line is commented out, then uncomment it and reopen the dev container (I don't think you need to rebuild the container, but I'm not entirely sure).

## Modules

### Camera

This module provides the functionality to interface with all of our physical cameras. This module is designed around one general camera "Interface" for which we provide various implementations for the different specific hardwares.

### Core

This module implements the backbone of the OBC, providing the structure that all other modules rely upon.

### CV

This module encapsulates the entire computer vision pipeline, providing an interface which allows images to be input and identified targets to be output.

### Network

This module handles the various communications with other parts of the larger system. These links include

- an HTTP server that the GCS makes requests to
- an Airdrop Client which communicates via WIFI to the airdrop payloads
- a Mavlink Client which communicates with the flight controller (Pixhawk)

### Pathing

This module implements the RRT* algorithm to plan out smart paths in order to navigate through competition waypoints, plan an airdrop approach path, and (possibly someday) avoid other planes flying in the sky.

### Ticks

This module implements all of the various ticks that the program progresses through throughout the mission.

### [udp_squared](https://github.com/tritonuas/udp_squared)

A git submodule (implemented in the linked repo) which provides helper functions, enumerations, and structs for the communication protocol with the airdrop payloads.

### Utilities

This module provides various helper types and classes used throughout the OBC, as well as some mission-related constants.


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
