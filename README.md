<p align="center">
  <a href="https://www.fix8mt.com"><img src="fix8mt_Master_Logo_Green_Trans.png" width="200"></a>
</p>

# Fix8pro C++ example
Simple example client/server that can be used as a starting point for development using the Fix8Pro C++ Framework.

## Before you start
You will need the following
1. A Fix8Pro license from Fix8MT (this could be an evaluation license)
2. An installed Fix8Pro binary package
3. A supported C++ compiler and biuld environment

## To build

```bash
git clone https://github.com/fix8mt/fix8pro_example.git
cmake -DFIX8PRO_LICENSE_FILE=<path to your license file> -DFIX8PRO_ROOT=<path to installed Fix8Pro package> -DCMAKE_BUILD_TYPE=Release ..
make install
```

