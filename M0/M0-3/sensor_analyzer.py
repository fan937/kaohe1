#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
sensor_analyzer.py  —— 上一届学长留下的"能用"的脚本

注释（学长原话）：
    "处理一下传感器数据就能用"

原本意图：
    1. 读取 sensor_data.csv（列：time, value）
    2. 计算 value 的平均值、标准差
    3. 剔除离群值（|value - mean| > 2 * std）
    4. 把清洗后的数据保存为 cleaned_data.csv
    5. 打印一份统计摘要

现状：跑不通 / 跑出来数不对。就交给你了。
"""
import math
import csv
import os
import sys

INPUT_FILE = "sensor_data.csv"
OUTPUT_FILE = "cleaned_data.csv"
OUTPUT_DIR = "out"  # 输出目录

data = []
times = []
cleaned = []
cleaned_times=[]

print("=== 传感器数据分析 ===")

# --- 读取数据 ---
try:
    with open(INPUT_FILE, "r") as f:
        reader = csv.DictReader(f)
        if "time" not in reader.fieldnames or "value" not in reader.fieldnames:
            print("Error: 列名不匹配或缺列")
            sys.exit(1)
        for row in reader:
            try:
                t = float(row["time"])
                v = float(row["value"])
            except ValueError:
                print("Error: 非数值内容")
                sys.exit(1)
            times.append(t)
            data.append(v)
except FileNotFoundError:
    print("Error: 文件不存在")
    sys.exit(1)
if len(data) == 0:
    print("Error: 文件为空")
    sys.exit(1)
print("共读取 %d 条数据" % len(data))
# --- 计算平均值 ---
total = 0
for v in data:
    total += v
mean = total / len(data)

# --- 计算标准差 ---
acc = 0
for v in data:
    acc += (v - mean)**2
std = math.sqrt(acc / len(data))

# --- 剔除离群值 ---
i=0
for v in data:
    if math.fabs(v -mean)<  2 * std:
        cleaned.append(v)
        cleaned_times.append(times[i])
    i+=1

# --- 输出清洗后的数据 ---
output_path = "cleaned_data.csv"
if OUTPUT_DIR:
    os.makedirs(OUTPUT_DIR,exist_ok=True)
f = open(output_path, "w")
writer = csv.writer(f)
writer.writerow(["time", "value"])
for v in range(len(cleaned)):
    a=cleaned_times[v]
    b=cleaned[v]
    writer.writerow([a,b])

print("均值 mean = %.4f" % mean)
print("标准差 std = %.4f" % std)
print("清洗后剩余 %d 条" % len(cleaned))
print("已保存到 %s" % output_path)
