import argparse
import random
import time
import json

from color import Color, is_tty
from loader import load_config
from graph import topological_sort


def save_report(report_data, out_path):
    try:
        with open(out_path, "w", encoding="utf-8") as f:
            json.dump(report_data, f, indent=2, ensure_ascii=False)
        print(f"{Color.GREEN}报告已写入：{out_path}{Color.RESET}")
    except Exception as e:
        print(f"{Color.RED}Error: 无法写入报告 {out_path},{e}{Color.RESET}")
        exit(1)


def main():
    parser = argparse.ArgumentParser(description="M0‑4 Task Scheduler")
    parser.add_argument("--config", required=True, help="任务配置文件 (.yaml/.yml/.json)")
    parser.add_argument("--timeout", type=float, default=None, help="全局超时(秒)，覆盖配置文件")
    parser.add_argument("--report", default="report.json", help="报告输出路径，默认 report.json")
    parser.add_argument("--seed", type=int, default=None, help="随机种子，便于复现")
    args = parser.parse_args()

    if args.seed is not None:
        random.seed(args.seed)

    cfg = load_config(args.config)
    global_timeout = args.timeout if args.timeout is not None else cfg.get("timeout", 15.0)
    raw_tasks = cfg.get("tasks", [])

    if not isinstance(raw_tasks, list) or len(raw_tasks) == 0:
        print(f"{Color.RED}Error: tasks 列表为空或格式错误{Color.RESET}")
        exit(1)
    for idx, t in enumerate(raw_tasks):
        name = t.get("name")
        if not isinstance(name, str) or len(name.strip()) == 0:
            print(f"{Color.RED}Error: 第{idx+1}个任务的name非法或为空{Color.RESET}")
            exit(1)
        rate = t.get("success_rate")
        if not isinstance(rate, (int, float)) or not (0.0 <= rate <= 1.0):
            print(f"{Color.RED}Error: 任务[{name}] success_rate={rate} 越界,必须在0~1之间{Color.RESET}")
            exit(1)
        dur = t.get("duration")
        if not isinstance(dur, (int, float)) or dur <= 0:
            print(f"{Color.RED}Error: 任务[{name}] duration={dur} 非法,必须大于0{Color.RESET}")
            exit(1)
        deps = t.get("dependencies", [])
        if not isinstance(deps, list):
            print(f"{Color.RED}Error: 任务[{name}] dependencies必须是数组/列表{Color.RESET}")
            exit(1)
    task_map = {}
    for t in raw_tasks:
        name = t.get("name")
        if not name:
            print(f"{Color.RED}Error: 存在没有name的任务{Color.RESET}")
            exit(1)
        if name in task_map:
            print(f"{Color.RED}Error: 重复任务名 '{name}'{Color.RESET}")
            exit(1)
        task_map[name] = {
            "duration": t.get("duration", 1.0),
            "success_rate": t.get("success_rate", 0.8),
            "dependencies": t.get("dependencies", [])
        }

    try:
        exec_order = topological_sort(task_map)
    except RuntimeError as e:
        print(f"{Color.RED}Error: {e}{Color.RESET}")
        exit(1)

    start_real = time.time()
    is_timeout_flag = False
    must_skip = set()
    task_records = {}

    for name in task_map:
        task_records[name] = {
            "name": name,
            "status": None,
            "attempts": 0,
            "duration": task_map[name]["duration"],
            "started_at": None,
            "ended_at": None
        }

    print("TaskScheduler启动")
    print(f"全局超时：{global_timeout}s | 拓扑顺序：{exec_order}\n")

    for task_name in exec_order:
        now = time.time()
        if now - start_real >= global_timeout:
            is_timeout_flag = True
            rec = task_records[task_name]
            rec["status"] = "TIMEOUT"
            rec["started_at"] = now
            rec["ended_at"] = now
            print(f"{Color.RED}[TIMEOUT] 已超过全局 {global_timeout}s,终止后续所有任务！{Color.RESET}")
            break

        rec = task_records[task_name]
        task = task_map[task_name]

        if task_name in must_skip:
            print(f"{Color.BLUE}[SKIPPED] {task_name}: 前置任务失败，自动跳过{Color.RESET}")
            rec["status"] = "SKIPPED"
            rec["started_at"] = now
            rec["ended_at"] = time.time()
            continue

        rec["started_at"] = time.time()
        print(f"{Color.GREEN}[RUN] {task_name} | duration={task['duration']}s rate={task['success_rate']}{Color.RESET}")
        ok = False
        MAX_TRIES = 3

        for att in range(MAX_TRIES):
            now2 = time.time()
            if now2 - start_real >= global_timeout:
                is_timeout_flag = True
                rec["status"] = "TIMEOUT"
                rec["ended_at"] = now2
                print(f"{Color.RED}[TIMEOUT] 执行途中超时！{Color.RESET}")
                break

            rec["attempts"] += 1
            if att > 0:
                print(f"    {Color.YELLOW}[RETRY] {task_name} 第 {att+1}/3 次尝试{Color.RESET}")
            else:
                print(f"    尝试 {att+1}/3 ... ", end="", flush=True)

            t_sleep = task["duration"]
            step = 0.1
            slept = 0.0
            while slept < t_sleep:
                remain = t_sleep - slept
                if remain > step:
                    time.sleep(step)
                    slept += step
                else:
                    time.sleep(remain)
                    slept += remain
                if time.time() - start_real >= global_timeout:
                    is_timeout_flag = True
                    break
            if is_timeout_flag:
                rec["status"] = "TIMEOUT"
                rec["ended_at"] = time.time()
                print(f"\n{Color.RED}[TIMEOUT] 任务执行时超时！{Color.RESET}")
                break

            r = random.random()
            if r < task["success_rate"]:
                print(f"{Color.GREEN}SUCCESS{Color.RESET}")
                ok = True
                break
            else:
                print(f"{Color.RED}FAILED{Color.RESET}")

        if is_timeout_flag:
            break

        rec["ended_at"] = time.time()
        if ok:
            rec["status"] = "SUCCESS"
        else:
            rec["status"] = "SKIPPED"
            print(f"{Color.BLUE}[SKIPPED] {task_name}: 3次全失败,下游全部跳过{Color.RESET}")
            q = [task_name]
            while q:
                cur = q.pop(0)
                for cand in task_map:
                    deps = task_map[cand]["dependencies"]
                    if cur in deps and cand not in must_skip:
                        must_skip.add(cand)
                        q.append(cand)

    total_dur = time.time() - start_real
    if is_timeout_flag:
        for name in task_records:
            r = task_records[name]
            if r["status"] is None:
                r["status"] = "TIMEOUT"
                now_t = time.time()
                r["started_at"] = now_t if r["started_at"] is None else r["started_at"]
                r["ended_at"] = now_t

    report = {
        "timeout": is_timeout_flag,
        "total_duration": round(total_dur, 3),
        "tasks": list(task_records.values())
    }
    save_report(report, args.report)

    print("执行总结")
    for name, r in task_records.items():
        s = r["status"]
        if s == "SUCCESS":
            c = Color.GREEN
        elif s == "SKIPPED":
            c = Color.BLUE
        else:
            c = Color.RED
        print(f"  {c}{name:<18s} → {s} (尝试:{r['attempts']}){Color.RESET}")
    print(f"\n总真实耗时:{total_dur:.3f} s")


if __name__ == "__main__":
    main()