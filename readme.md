# File descriptor leak bug

When running this the client we see file descriptor leaks.
The client continually recreates a channel and a stub, which is probably not
the recommended behaviour?

The leak is observed when building against the latest versions of `grpc` (1.18)
and `protoc` (3.6.1) fetched by the `vcpkg` package manager.

When building against the latest `grpc/cxx` Docker image, the leak is *not* present.
The  image contains `grpc` 1.10.x (based on [this dockerfile](https://hub.docker.com/r/grpc/cxx/dockerfile/)
and `protoc` 3.5.0.

# Reproducing the bug

Start either with the latest grpc/cxx docker image, or Debian Stretch with build
essentials installed.

Install `vcpkg`:

    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    sudo ./vcpkg integrate install
    ./vcpkg install fftw3 protobuf cppzmq grpc

    cd /usr/local/bin
    sudo ln -s ${HOME}/vcpkg/vcpkg vcpkg
    Add following to .bashrc file: "export VCPKG_ROOT=${HOME}/vcpkg"

Check out this repo and run `cmake`

    git clone https://github.com/IzakMarais/grpc-bug.git
    cd grpc-bug
    mkdir build
    cd build
    ${VCPKG_ROOT}/downloads/tools/cmake-3.12.4-linux/cmake-3.12.4-Linux-x86_64/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake ../
    make

Run the `route_guide_server` and `route_guide_client` in separate terminals. In a third
terminal do

    ls /proc/`pidof route_guide_client`/fd

One can see that the number of file descriptors increases after each rpc call the client made.

# Bug not present when built against grpc/cxx docker image

If we just check out the repo in the `grpc/cxx` Docker and run make, the bug is
not present.

    git clone https://github.com/IzakMarais/grpc-bug.git
    cd grpc-bug
    make

Run the route_guide_client and route_guide_server in separate terminals. In a third
terminal do

    ls /proc/`pidof route_guide_client`/fd

One can see that the number of file descriptors remain constant.