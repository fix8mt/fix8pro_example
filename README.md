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

