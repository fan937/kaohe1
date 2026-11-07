import argparse
import numpy as np

def complementary_filter(t, ax, ay, az, gx, gy, gz):
    
    alpha = 0.98  
    pitch = 0.0
    pitch_list = []

    
    static_num = 80
    n_samples = len(t)
    static_num = min(static_num, n_samples)
    bias_gx = np.mean(gx[:static_num])
    bias_gy = np.mean(gy[:static_num])
    bias_gz = np.mean(gz[:static_num])

    gx_comp = gx - bias_gx
    gy_comp = gy - bias_gy
    gz_comp = gz - bias_gz

    # 初始角度由加速度计算出
    pitch_acc = np.arctan2(-ax, az) * 180 / np.pi
    pitch = pitch_acc[0]

    for i in range(len(t)):
        if i == 0:
            dt = t[1] - t[0]
        else:
            dt = t[i] - t[i-1]
        # 陀螺积分
        pitch_gyro = pitch + gy_comp[i] * dt
        # 加速度计角度
        pitch_acc = np.arctan2(-ax[i], az[i]) * 180 / np.pi
        # 互补融合
        pitch = alpha * pitch_gyro + (1 - alpha) * pitch_acc
        pitch_list.append(pitch)
    return np.array(pitch_list), bias_gx, bias_gy, bias_gz

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, help="输入csv路径")
    parser.add_argument("--out", required=True, help="输出csv路径")
    args = parser.parse_args()

    
    data = np.loadtxt(args.input, delimiter=",", skiprows=1)
    t = data[:,0]
    ax = data[:,1]
    ay = data[:,2]
    az = data[:,3]
    gx = data[:,4]
    gy = data[:,5]
    gz = data[:,6]

    angle_deg, bgx, bgy, bgz = complementary_filter(t, ax, ay, az, gx, gy, gz)

    
    out_data = np.column_stack((t, angle_deg))
    np.savetxt(args.out, out_data, delimiter=",", header="t,angle_deg", comments="", fmt="%.4f")

    
    
    print(f"bias_gx={bgx:.3f}, bias_gy={bgy:.3f}, bias_gz={bgz:.3f}")

if __name__ == "__main__":
    static_num = 80
    main()