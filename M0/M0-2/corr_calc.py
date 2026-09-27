import yaml
import csv
import math
import argparse
import sys
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
    if dx==0 or dy==0:
        return None
    denom = math.sqrt(dx * dy)
    r = prod / denom
    return r

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config", help="yaml配置文件路径")
    args = parser.parse_args()
    CONFIG_PATH = args.config

    try:
        with open(CONFIG_PATH) as f:
            cfg = yaml.safe_load(f)

        csv_path = cfg["input_csv"]
        col_x = cfg["columns"]["x"]
        col_y = cfg["columns"]["y"]

        xs = []
        ys = []
        with open(csv_path) as f:
            reader = csv.DictReader(f)
            if col_x not in reader.fieldnames or col_y not in reader.fieldnames:
                print("Error: 列名不匹配")
                return
            for row in reader:
                xs.append(float(row[col_x]))
                ys.append(float(row[col_y]))
            if len(xs)==0 or len(ys)==0:
                print("Error:文件为空")

        r = pearson_correlation(xs, ys)
        if r is None:
            print("Error:方差为0,某列取值恒定，无法计算相关系数")
            sys.exit(1)
        else:
            print(f"r = {r}")
    except FileNotFoundError:
        print("Error: 文件不存在")
        sys.exit(1)
    except KeyError:
        print("Error: 配置字段缺失")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()

