import math
import csv
import yaml

def pearson_correlation(xs, ys):
    n = len(xs)
    if n == 0:
        return None
    sum_x = 0.0
    sum_y = 0.0
    for i in range(n):
        sum_x += xs[i]
        sum_y += ys[i]

    mean_x = sum_x / n
    mean_y = sum_y / n

    dx = 0.0
    dy = 0.0
    prod = 0.0
    for i in range(n):
        a = xs[i] - mean_x
        b = ys[i] - mean_y
        dx += a * a
        dy += b * b
        prod += a * b
    denom = math.sqrt(dx * dy)
    r = prod / denom
    return r
def main():
     CONFIG_PATH = "config.yaml"
     with open(CONFIG_PATH) as f:
         cfg = yaml.safe_load(f)
     csv_path = cfg["input_csv"]
     col_x = cfg["columns"]["x"]
     col_y = cfg["columns"]["y"]
     xs = []
     ys = []
     with open(csv_path) as f:
         reader = csv.DictReader(f)
         for row in reader:
             xs.append(float(row[col_x]))
             ys.append(float(row[col_y]))
     r = pearson_correlation(xs, ys)
     print(f"r = {r}")
if __name__ == "__main__":
     main()
