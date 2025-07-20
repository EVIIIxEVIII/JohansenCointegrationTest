# C++ Johansen's Cointegration test
You can find the ```johansen_test.cpp``` and ```johansen_test.hpp``` files in the root of the project which you can just drag and drop into your project which uses Eigen. All other files are added for you to make sure that my implementation is correct and has somewhat decent performance.

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



The absolutely same parameters are used in the ```test/test_johansen.py``` file which you can use to make sure that the results of my implementation are the same as in the reference **statsmodel** library.

**Importat:** the program expects the data in CSV format and the column which the test will be performed must be called **data**!
