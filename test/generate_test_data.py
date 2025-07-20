import numpy as np
import pandas as pd

np.random.seed(42)

n_obs = 1000
drift = 0.05
noise_std = 1.0
cointegration_noise_std = 0.5

eps1 = np.random.normal(0, noise_std, n_obs)
series1 = np.cumsum(drift + eps1)

eps2 = np.random.normal(0, cointegration_noise_std, n_obs)
series2 = 2.0 * series1 + eps2

pd.DataFrame(series1, columns=["price"]).to_csv("data/series1.csv", index=False)
pd.DataFrame(series2, columns=["price"]).to_csv("data/series2.csv", index=False)

print("Generated two cointegrated series in 'series1.csv' and 'series2.csv'.")

