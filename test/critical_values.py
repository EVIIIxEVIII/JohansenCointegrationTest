import numpy as np
import statsmodels.api as sm
from statsmodels.tsa.vector_ar.vecm import coint_johansen

k = 10
data = np.random.randn(200, k)
res = coint_johansen(data, det_order=0, k_ar_diff=1)

print("Trace crit vals:", res.cvt)
print("Max eigen crit vals:", res.cvm)

