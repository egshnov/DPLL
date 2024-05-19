# Description
Implementation of a simple DPLL sat solver
# Build
To build the library do the following:  
`mkdir build`  
`cd build`  
`cmake -DCMAKE_BUILD_TYPE=Release ..`  
`make`
# Verifying solver
You need picosat and python3  
`cd tests`  
`python3 ./verify.py`  
**Note:** if your build directory name differs from 'build' you should change variable my_solver in `verify.py` script
