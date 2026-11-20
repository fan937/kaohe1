import argparse
import numpy as np

def calc_step_info(t, y, ref, noise_thr=1.2):
    """
    noise_thr：稳态允许自身波动标准差(rpm)，根据你的噪声调
    兼容：P(有静差) / PI / PID
    """
    # ---------- 超调 ----------
    peak_val = np.max(y)
    overshoot_pct = 0.0
    if peak_val > ref:
        overshoot_pct = (peak_val - ref) / ref * 100.0

    # ---------- 上升时间 10% →90% ref ----------
    y10 = ref * 0.1
    y90 = ref * 0.9
    idx10 = np.argmax(y >= y10)
    idx90 = np.argmax(y >= y90)
    rise_time = t[idx90] - t[idx10]

    # ---------- 稳态检测：从末尾向前扫描，看片段自身标准差，不看离ref远近 ----------
    n = len(y)
    win_len = int(n * 0.25)   # 滑动窗口长度，取总长度25%
    steady_idx = None
    # 从后往前滑动
    for i in range(n - win_len, 0, -1):
        seg = y[i:i+win_len]
        std_seg = np.std(seg)
        if std_seg < noise_thr:
            steady_idx = i
            break

    if steady_idx is None:
        # 全程没有找到平稳段
        return {
            "ref": ref,
            "overshoot_pct": overshoot_pct,
            "rise_time_s": rise_time,
            "steady_error_rpm": np.nan,
            "steady_mean": np.nan,
            "steady_ripple_rpm": np.nan
        }

    steady_y = y[steady_idx:]
    steady_mean = np.mean(steady_y)
    steady_ripple = np.std(steady_y)
    steady_error = ref - steady_mean

    return {
        "ref": ref,
        "overshoot_pct": overshoot_pct,
        "rise_time_s": rise_time,
        "steady_error_rpm": steady_error,
        "steady_mean": steady_mean,
        "steady_ripple_rpm": steady_ripple
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="阶跃响应分析，兼容纯P(有静差)/PI/PID")
    parser.add_argument("--input", required=True)
    parser.add_argument("--ref", type=float, required=True)
    parser.add_argument("--noise_thr", type=float, default=1.5, help="稳态段自身波动阈值rpm")
    args = parser.parse_args()

    data = np.genfromtxt(args.input, delimiter=",", skip_header=1)
    t = data[:,0]
    actual_rpm = data[:,2]

    res = calc_step_info(t, actual_rpm, args.ref, noise_thr=args.noise_thr)

    print("===== 阶跃响应分析结果 =====")
    print(f"参考目标 ref        = {res['ref']:.2f} rpm")
    print(f"超调量               = {res['overshoot_pct']:.2f} %")
    print(f"上升时间(10%→90%)   = {res['rise_time_s']:.3f} s")
    print(f"稳态实际平均转速     = {res['steady_mean']:.2f} rpm")
    print(f"稳态误差(ref‑实际)   = {res['steady_error_rpm']:.2f} rpm")
    print(f"稳态波动(标准差)     = {res['steady_ripple_rpm']:.2f} rpm")