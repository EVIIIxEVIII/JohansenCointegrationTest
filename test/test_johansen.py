import argparse
import pandas as pd
from statsmodels.tsa.vector_ar.vecm import coint_johansen

def parse_args():
    parser = argparse.ArgumentParser(description="Python Johansen Test Comparison")
    parser.add_argument("--files", type=str, required=True,
                        help="Comma-separated CSV file list")
    parser.add_argument("--lags", type=int, default=1, help="Number of lags (p)")
    parser.add_argument("--detOrder", type=int, default=0,
                        help="Deterministic trend order (-1, 0, 1, etc.)")
    return parser.parse_args()

def load_data(file_paths):
    raw_data = []
    for f in file_paths:
        df = pd.read_csv(f)
        assert "data" in df.columns, f"'data' column missing in {f}"
        print(f"{f}: {len(df['data'])} prices")
        raw_data.append(df["data"].values)
    return raw_data

if __name__ == "__main__":
    args = parse_args()
    files = args.files.split(",")
    lags = args.lags
    det_order = args.detOrder

    print("\n\n---------------------------------")
    print("Input data:")
    print("---------------------------------")
    data = load_data(files)

    print(f"Number of lags: {lags}")
    print(f"Deterministic order: {det_order}")

    import numpy as np
    data_matrix = np.vstack(data).T  # shape: (T, N_assets)

    print("\n\n---------------------------------")
    print("Running Johansen test...")
    print("---------------------------------")

    result = coint_johansen(data_matrix, det_order, lags)

    print("\n\n---------------------------------")
    print("Raw results:")
    print("---------------------------------")

    print(f"Eigenvalues:\n{result.eig}\n")
    print(f"Trace statistic:\n{result.lr1}\n")
    print(f"Max eigenvalue statistic:\n{result.lr2}\n")

    print("\nCointegration vectors (each column):")
    print(result.evec)

    print("\n\n---------------------------------")
    print("Critical Values:")
    print("---------------------------------")
    print("Trace statistic critical values:")
    print(result.cvt)
    print("Max eigenvalue statistic critical values:")
    print(result.cvm)

