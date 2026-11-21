import argparse
import re
import struct
import sys

HEAD1 = 0xAA
HEAD2 = 0x55
MAX_PAYLOAD_LEN = 32

MSG_LEN = {0x01: 4, 0x02: 14}
MSG_NAME = {
    0x01: "速度帧（上位机 → MCU）",
    0x02: "状态帧（MCU → 上位机）",
}
MSG_FIELDS = {
    0x01: [
        ("left_rpm", "rpm"),
        ("right_rpm", "rpm"),
    ],
    0x02: [
        ("l_rpm", "rpm"),
        ("r_rpm", "rpm"),
        ("pitch", "×0.01°"),
        ("heartbeat", "帧计数"),
        ("bad_frame", "坏帧数"),
        ("timeout", "超时数"),
        ("overflow", "溢出数"),
    ],
}


# ----------------------------------------------------------------------------
# 基础算法（与 C 实现完全独立，按文档重写一遍）
# ----------------------------------------------------------------------------
def crc16_modbus(buf):
    """CRC16/Modbus：初值 0xFFFF，多项式 0xA001（反向 0x8005）。"""
    crc = 0xFFFF
    for b in buf:
        crc ^= b
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


def s16(v):
    """uint16 → int16（补码）"""
    return v - 0x10000 if v >= 0x8000 else v


def to_hex(buf):
    return " ".join("%02X" % b for b in buf)


def hex_to_bytes(text):
    """把任意写法的十六进制串转成 bytes：容忍空格、逗号、0x 前缀、下划线。"""
    cleaned = re.sub(r"[^0-9a-fA-F]", "", text)
    if not cleaned:
        raise ValueError("没有找到任何十六进制字符")
    if len(cleaned) % 2 != 0:
        raise ValueError("十六进制字符个数为奇数（%d），无法按字节切分" % len(cleaned))
    return bytes(int(cleaned[i:i + 2], 16) for i in range(0, len(cleaned), 2))


# ----------------------------------------------------------------------------
# 解析
# ----------------------------------------------------------------------------
def parse_frame(data, tag=""):
    """返回 (是否通过, 输出行列表)"""
    lines = []
    ok = True
    head = ("【%s】" % tag) if tag else ""
    lines.append("%s帧长度 %d 字节：%s" % (head, len(data), to_hex(data)))

    if len(data) < 6:
        lines.append("  X 长度不足 6 字节（帧头 2 + LEN 1 + MSG_ID 1 + CRC 2），不可能构成合法帧")
        return False, lines

    b0, b1 = data[0], data[1]
    lines.append("  HEAD1   = %02X  %s" % (b0, "OK" if b0 == HEAD1 else "X 应为 AA"))
    lines.append("  HEAD2   = %02X  %s" % (b1, "OK" if b1 == HEAD2 else "X 应为 55"))
    if b0 != HEAD1 or b1 != HEAD2:
        lines.append("  => 帧头不匹配，整条不是本协议的帧（视为垃圾字节）")
        return False, lines

    length = data[2]
    msg_id = data[3]
    lines.append("  LEN     = %d (0x%02X)" % (length, length))
    lines.append("  MSG_ID  = 0x%02X  -> %s" % (msg_id, MSG_NAME.get(msg_id, "未知消息（语义校验应判坏帧）")))

    if length > MAX_PAYLOAD_LEN:
        lines.append("  X LEN 超过上限 %d，MCU 端会直接判坏帧" % MAX_PAYLOAD_LEN)
        return False, lines

    total = 6 + length
    if len(data) != total:
        lines.append("  X 整帧长度应为 6+LEN = %d，实际 %d（多出 %d 字节 / 少了 %d 字节）"
                     % (total, len(data), max(0, len(data) - total), max(0, total - len(data))))
        ok = False

    if msg_id in MSG_LEN and length != MSG_LEN[msg_id]:
        lines.append("  X LEN 与 MSG_ID 不匹配：0x%02X 应为 %d 字节，实际 %d"
                     % (msg_id, MSG_LEN[msg_id], length))
        ok = False

    payload = data[4:4 + length]
    crc_recv = None
    if len(data) >= 4 + length + 2:
        crc_recv = (data[4 + length + 1] << 8) | data[4 + length]
    crc_calc = crc16_modbus(data[:4 + length])

    if crc_recv is None:
        lines.append("  CRC     = 数据不完整，拿不到 CRC 两字节")
        ok = False
    else:
        match = (crc_recv == crc_calc)
        lines.append("  CRC     计算=0x%04X  接收=0x%04X  %s" % (crc_calc, crc_recv, "OK 匹配" if match else "X 不匹配（应判坏帧）"))
        if not match:
            ok = False

    # 字段拆解
    fields = MSG_FIELDS.get(msg_id)
    lines.append("  PAYLOAD = %d 字节：%s" % (len(payload), to_hex(payload)))
    if fields and len(payload) == 2 * len(fields):
        lines.append("  字段（小端 int16 解包）：")
        for i, (name, unit) in enumerate(fields):
            raw = struct.unpack("<H", payload[2 * i:2 * i + 2])[0]
            val = s16(raw)
            shown = "%.2f" % (val / 100.0) if unit.startswith("×") else str(val)
            lines.append("    %-11s = %-8s (%s)" % (name, shown, unit))
    elif fields:
        lines.append("  X payload 字节数与字段表不符，无法解包")
        ok = False
    else:
        lines.append("  （未知 MSG_ID，不做字段解包）")

    lines.append("  => %s" % ("通过：帧结构、长度、CRC、字段全部一致" if ok else "失败：见上方 X 项"))
    return ok, lines


# ----------------------------------------------------------------------------
# 生成
# ----------------------------------------------------------------------------
def build_frame(msg_id, payload):
    body = bytes([HEAD1, HEAD2, len(payload), msg_id]) + payload
    crc = crc16_modbus(body)
    return body + bytes([crc & 0xFF, (crc >> 8) & 0xFF])


def gen_status(args):
    payload = struct.pack("<h", args.l) + struct.pack("<h", args.r) + \
              struct.pack("<h", args.pitch) + struct.pack("<h", args.hb) + \
              struct.pack("<H", args.bad) + struct.pack("<H", args.timeout) + \
              struct.pack("<H", args.overflow)
    return build_frame(0x02, payload)


def gen_speed(args):
    payload = struct.pack("<h", args.l) + struct.pack("<h", args.r)
    return build_frame(0x01, payload)


def swap_endian_demo(data):
    """把 payload 里每个 int16 的两个字节对调，然后重算 CRC。
    结果：CRC 仍然正确，但数值全错 —— 用来证明 CRC 管不了字节序。"""
    if len(data) < 6:
        return None
    length = data[2]
    if len(data) != 6 + length:
        return None
    payload = bytearray(data[4:4 + length])
    for i in range(0, len(payload) - 1, 2):
        payload[i], payload[i + 1] = payload[i + 1], payload[i]
    return build_frame(data[3], bytes(payload))


# ----------------------------------------------------------------------------
def collect_inputs(args):
    """收集待解析的十六进制串"""
    items = []
    for h in args.hex:
        items.append((h, "命令行"))
    if args.file:
        with open(args.file, "r", encoding="utf-8", errors="ignore") as f:
            for lineno, line in enumerate(f, 1):
                s = line.strip()
                if not s:
                    continue
                # 只挑“看起来像一整条帧”的十六进制串：长度 >= 6 字节
                if not re.search(r"[0-9a-fA-F]", s):
                    continue
                try:
                    b = hex_to_bytes(s)
                except ValueError:
                    continue
                if len(b) >= 6:
                    items.append((s, "文件第 %d 行" % lineno))
    return items


def main():
    p = argparse.ArgumentParser(
        description="协议帧独立解析/生成工具（用于文档交叉验证）",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="示例：\n"
               "  python3 parse_frame.py AA 55 0E 02 ...\n"
               "  python3 parse_frame.py -f docs/protocol_v1.0.md\n"
               "  python3 parse_frame.py --gen-speed --l 30 --r 30\n"
               "  python3 parse_frame.py --swap-demo \"AA 55 0E 02 ...\"\n")
    p.add_argument("hex", nargs="*", help="一条或多条帧的十六进制字符串")
    p.add_argument("-f", "--file", help="从文件读取（每行一条帧）")
    p.add_argument("--gen-speed", action="store_true", help="生成 0x01 速度帧")
    p.add_argument("--gen-status", action="store_true", help="生成 0x02 状态帧")
    p.add_argument("--swap-demo", metavar="HEX", help="字节序对调演示（输入一条合法帧）")
    p.add_argument("--l", type=int, default=0, help="left / l_rpm")
    p.add_argument("--r", type=int, default=0, help="right / r_rpm")
    p.add_argument("--pitch", type=int, default=300, help="pitch 原始值（×100）")
    p.add_argument("--hb", type=int, default=1000, help="heartbeat")
    p.add_argument("--bad", type=int, default=0, help="bad_frame")
    p.add_argument("--timeout", type=int, default=0, help="timeout")
    p.add_argument("--overflow", type=int, default=0, help="overflow")
    args = p.parse_args()

    # ---------- 生成模式 ----------
    if args.gen_speed or args.gen_status:
        frame = gen_speed(args) if args.gen_speed else gen_status(args)
        print("生成帧（可直接贴进协议文档）：\n")
        print("  十六进制：" + to_hex(frame))
        print("  紧凑写法：" + "".join("%02X" % b for b in frame))
        print()
        _, lines = parse_frame(frame, tag="自校验")
        print("\n".join(lines))
        return 0

    # ---------- 字节序演示 ----------
    if args.swap_demo:
        try:
            orig = hex_to_bytes(args.swap_demo)
        except ValueError as e:
            print("输入错误：%s" % e)
            return 2
        swapped = swap_endian_demo(orig)
        if swapped is None:
            print("输入错误：这不是一条结构完整（长度=6+LEN）的帧")
            return 2

        print("=" * 70)
        print("原始帧（文档规定小端）")
        print("=" * 70)
        _, l1 = parse_frame(orig, tag="原始")
        print("\n".join(l1))

        print()
        print("=" * 70)
        print("把 payload 每两个字节对调、并重算 CRC（模拟『一端改了字节序』）")
        print("=" * 70)
        print("  新帧：" + to_hex(swapped))
        ok2, l2 = parse_frame(swapped, tag="改序后")
        print("\n".join(l2))
        print()
        print("结论：CRC 依然通过 —— 说明 CRC 只防『位翻转』，不防『字节序被改』。")
        print("      这也是为什么上位机必须再加一层语义校验（msg_id / len / 取值范围）。")
        return 0

    # ---------- 解析模式 ----------
    items = collect_inputs(args)
    if not items:
        p.print_help()
        return 2

    passed = 0
    failed = 0
    for idx, (text, src) in enumerate(items, 1):
        try:
            data = hex_to_bytes(text)
        except ValueError as e:
            print("【#%d %s】跳过：%s\n" % (idx, src, e))
            failed += 1
            continue
        ok, lines = parse_frame(data, tag="#%d %s" % (idx, src))
        print("\n".join(lines))
        print("-" * 70)
        if ok:
            passed += 1
        else:
            failed += 1

    print("合计：%d 条通过，%d 条失败" % (passed, failed))
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())