def topological_sort(task_dict: dict):
    in_degree = {name: 0 for name in task_dict}
    adjacency = {name: [] for name in task_dict}

    for name, task in task_dict.items():
        for dep in task.get("dependencies", []):
            if dep not in task_dict:
                raise RuntimeError(f"任务「{name}」依赖不存在的任务「{dep}」")
            adjacency[dep].append(name)
            in_degree[name] += 1

    queue = [n for n in in_degree if in_degree[n] == 0]
    order = []
    while queue:
        u = queue.pop(0)
        order.append(u)
        for v in adjacency[u]:
            in_degree[v] -= 1
            if in_degree[v] == 0:
                queue.append(v)

    if len(order) != len(task_dict):
        remaining = set(task_dict.keys()) - set(order)
        raise RuntimeError(f"检测到依赖环！涉及任务：{sorted(remaining)}")
    return order