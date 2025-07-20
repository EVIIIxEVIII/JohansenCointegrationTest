# C++ Johansen's Cointegration test
You can find the `johansen_test.cpp` and `johansen_test.hpp` files in the root of the project which you can just drag and drop into your project which uses Eigen. 

All the other files are added for you to make sure that my implementation is correct and has somewhat decent performance.

# Compiling
In order to compile this project create a build directory `build/` directory and inside of it run
```
cmake ..
```
and then 
```
make
```

# Running 
The executable has 3 parameters: 
- `--files=` is a list of files which will be used in the test 
- `--lags=` is an integer which specifies the number of lags the test should include
- `--detOrder=` which is an integer used to specify the deterministic trend assumption

Example:
```bash
./build/johansen_test --files=data/BTC_USD.csv,data/ETH_USD.csv --detOrder=-1 --lags=1
```
```bash
python test/test_johansen.py --files=data/BTC_USD.csv,data/ETH_USD.csv --detOrder=1 --lags=1
```


The absolutely same parameters are used in the `test/test_johansen.py` file which you can use to make sure that the results of my implementation are the same as in the reference **statsmodel** library.

**Importat:** the program expects the data in CSV format and the column which the test will be performed must be called **data**!

More on the deterministic trend assumption
------
The following implementation supports 3 types of det. trend assumptions: 
- `-1` means no deterministic terms (no intercept, no trend) are included in the model. The time series is treated as purely stochastic with no deterministic mean or trend component. Only lagged differences and lagged levels enter the model.

- `0` means the time series can have level shifts, and the cointegration relationship includes a constant term. The test detects mean reversion **to a constant mean** but does **not** account for linear trends in the data.

- `1` means the time series can have both a constant and a linear trend. The cointegration relationship includes a constant, while the data is allowed to have **deterministic linear trends**. The test detects mean-reverting relationships after accounting for these trends.

# Source of Critical Values
The critical values have been taken directly from the statsmodel repo, specifically from this [file](https://github.com/statsmodels/statsmodels/blob/main/statsmodels/tsa/coint_tables.py).

# Performance
On my i7-12700h these are the results in Relase mode: 
```
---------------------------------
Input data: 
---------------------------------
data/BTC_USD.csv: 62759 prices
data/ETH_USD.csv: 62759 prices
Number of lags: 1
Det order: -1


---------------------------------
Profiler results: 
---------------------------------

 Total time: 4ms (CPU freq 2687987388)
   solveGenerEigenvalProb[1]: 23308 (0.008671ms) (0.21%)
   buildCovarianceMatrices[1]: 1381936 (0.514116ms) (12.49%)
   regress[1]: 5671872 (2.110081ms) (51.25%)
   buildRegressionMatrices[1]: 3177128 (1.181973ms) (28.71%)
```
**Note:** when you run the program you might notice a slight halt which takes more than 4ms. This halt is caused by the profiler which sleeps for 1 second in order to estimate the time step counter frequency in order to be able to conver `rdtsc` ticks into actual time.
