# MediaPlayerVirus

A companion virus, posing as a media player application.

## Usage

Install [CMake](https://cmake.org/download/) and [GTK 3](https://www.gtk.org/docs/installations/)


Clone this repository and compile it:

```bash
git clone https://github.com/agueguen-LR/MediaPlayerVirus.git
cd MediaPlayerVirus
cmake -B build
cd build
make
```


You can now run all the executable test utilities in your build/src folder:

```bash
cd src
./Xor
./GestMdp
./Calculator
./Liste
```


You can also run the virus, but make sure to run the backdoor cleanup script and delete any infected files afterwards:

```bash
./MediaPlayer
../../cleanup_ssh.sh # remove the ssh backdoor
# You should only need to delete any file that has a corresponding .old file
# but you can delete everything in the current directory to be safe
rm * && rm .*
```


If you want to test the MediaPlayer without infecting yourself, there are boolean flags at the top of [main.c](./src/main.c) to deactivate certain virus functionalities:

```c
// These are true by default, switch them to false and remember to run `make` to recompile afterwards
static bool INFECT_FILES = true;
static bool SSH_BACKDOOR = true;
```

## Cleanup script

If you infect yourself with the ssh backdoor, you can use this script to clean up the infection: [./cleanup_ssh.sh](./cleanup_ssh.sh)

## Presentation setup

Commands to execute on the ULR Ubuntu desktop to clone and install all the project dependencies.

```bash
git clone <https://github.com/agueguen-LR/MediaPlayerVirus.git>

# You might need to update the time of the computer

sudo apt update

sudo apt install cmake=

sudo apt install libgtk-3-dev
```


