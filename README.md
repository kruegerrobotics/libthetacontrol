# libthetacontrol

## Warning

This is work in progress and the goal would be to step by step enhance the interface and make it more robust. Currently is shaky and lacks checks and is more a proof of concept and starting point.

## Purpose

This libraries purpose is a dedicate control of a ricoh thetaV. The goal is that the functionalities are provided by this library respective c++ class the underlying mechanism is based on libptp and libusb.

## Usage of libptp code

Big parts of the code, especially in the *ptp_helpers* are directly taken from libptp2's ptpcam. The orginial can be found [here](http://libptp.sourceforge.net/). It is on the todo list to rework and streamline the functions for this project.

## How to build

### prerequisites

#### libptp (modified)

As explained this library relies heavily on [libptp2](http://libptp.sourceforge.net/) but the original was not exposing all functions and I had some problems using the original headers. Thus we use currently this fork of libptp2 to avoid this issues. More information is in the repo of the fork but defentely ideal would be the use of the original.

#### clone, build and install the fork of libptp2

This will clone, build and install this fork of libptp2. If no installation prefix is set via CMAKE_INSTALL_PREFIX then it will install into /usr/local and sudo rigths are required for the install.

``` bash 
git clone https://github.com/kruegerrobotics/libptp2
cd libptp2
mkdir build
cd build
cmake ../
make 
sudo make install
```

#### libusb

The libusb is required to build this library. On Debian systems libusb with lib and headers can be installed with

``` bash
sudo apt install lib libusb-dev 
```

### clone, build this library with examples

``` bash 
git clone https://github.com/kruegerrobotics/libthetacontrol.git
cd libthetacontrol
mkdir build
cd build
cmake ../
make 
