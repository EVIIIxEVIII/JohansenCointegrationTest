# C++ Johansen test implementation

# Compiling
In order to compile this project create a build directory ```build/``` directory and inside of it run
```
cmake ..
```
and then 
```
make
```

# Running 
The executable has 3 parameters: 
- ```--files=``` is a list of files which will be used in the test 
- ```--lags=``` is an integer which specifies the number of lags the test should include
- ```--detOrder=``` which is an integer used to specify the deterministic trend assumption

