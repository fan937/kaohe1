from corr_calc import pearson_correlation
import numpy as np

def test_positive_corr():
    xs = [1, 2, 3]
    ys = [2, 4, 6]
    r_my = pearson_correlation(xs, ys)
    r_np = np.corrcoef(xs, ys)[0, 1]
    print(f"r = {r_my}, numpy验证r = {r_np}")

def test_negative_corr():
    xs = [1, 2, 3]
    ys = [6, 4, 2]
    r_my = pearson_correlation(xs, ys)
    r_np = np.corrcoef(xs, ys)[0, 1]
    print(f"r = {r_my}, numpy验证r = {r_np}")

def test_zero_variance():
     xs = [3, 3, 3, 3]
     ys = [1, 2, 3, 4]
     r_my = pearson_correlation(xs, ys)
     print(f"零方差")
def test_empty_data():
     xs = []
     ys = []
     r_my = pearson_correlation(xs, ys)
     print(f"空文件")
def test_missing_column():
     print("缺列")

if __name__ == "__main__":
    test_positive_corr()
    test_negative_corr()
    test_zero_variance()
