<p align="center">
  <a href="https://www.fix8mt.com"><img src="fix8mt_Master_Logo_Green_Trans.png" width="200"></a>
</p>

# Fix8pro C++ example
Simple example client/server that can be used as a starting point for development using the Fix8Pro C++ Framework.

## To download
```bash
git clone https://github.com/fix8mt/fix8pro_example.git
cd fix8pro_example
```

## Before you build
You will need the following to build this example:
1. A supported C++ compiler and build environment
1. A Fix8Pro license from Fix8MT (or an evaluation license)
1. An installed Fix8Pro binary package

## To build
```bash
mkdir build
cd build
cmake -DFIX8PRO_LICENSE_FILE=<path to license file> -DFIX8PRO_ROOT=<path to installed Fix8Pro package> -DCMAKE_INSTALL_PREFIX=<install path> -DCMAKE_BUILD_TYPE=Release ..
make install
```

## To setup your run environment
1. Add your Fix8Pro binary and library installation directories to your `$PATH` and `$LD_LIBRARY_PATH`
```bash
export PATH=$PATH:<path of your Fix8Pro bin directory>
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<path of your Fix8Pro lib directory>
```
1. Set your `$FIX8PRO_LICENSE_FILE` environment variable
```bash
export FIX8PRO_LICENSE_FILE=<path of your xml license file>
```

## To run
This example has been designed to run as two instances - a client and a server. For simplicity we'll run the test from the ./build directory.
In one temrinal we'll run our server:
```bash
./simpleclisrv -c ../config/simple_server.xml -s
```
In our other temrinal we'll run our client:
```bash
./simpleclisrv -c ../config/simple_client.xml
```

