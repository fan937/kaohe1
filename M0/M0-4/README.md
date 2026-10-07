# M0‑4 任务调度模拟器

## 参数说明
`--config <path>` **(必填)**：任务配置文件路径，支持 `.yaml`、`.yml`、`.json`
`--timeout <float>` **(可选)**：覆盖配置文件中的全局超时时间，单位为秒
`--report <path>` **(可选)**：设置报告输出路径，默认为 `report.json`
`--seed <int>` **(可选)**：设置随机种子，便于复现运行结果

通用调用格式：
```bash
python3 scheduler.py --config <path> [--timeout <float>] [--report <path>] [--seed <int>]

运行示例：
示例1：
python3 scheduler.py --config tasks_demo.yaml \
    --timeout 15 --report report.json --seed 42
示例2：
python3 scheduler.py --config tasks_demo.json \
    --timeout 15 --report report.json --seed 42
示例3：
python3 scheduler.py --config tasks_demo.json \
    --timeout 20 --report report.json --seed 5

备注:本人通过查询AI了解HASATTR函数后尝试调用来检查是否为TTY环境，通过AI提供的ANSI颜色码将四种颜色分类，并定义一个还原颜色的变量。
在封装loader.py时，我原本是直接导入yaml文件的，然后添加异常处理，后面报错ModuleNotFoundError,询问AI后发现没装pyyaml，之后将代码修改，检查是否安装pyyaml后再导入yaml文件。
封装graph.py时，询问AI排序方法，AI通过创建字典读取每个任务的名字，然后通过循环读取当前任务的依赖，在这些依赖后面添加该任务，最后通过队列的方法逐个提取出可执行任务，从而实现按依赖顺序执行任务,我依靠AI把每行代码的作用和逻辑理清。
写scheduler.py时，先封装一个输出报告函数，然后按要求读取四个命令行参数，利用AI写非法的处理，按照要求创建taskmap读取三个变量，本人写了调用依赖执行的封装函数并进行异常处理，并按照它定义的字典依要求写入每个任务和初始化输出的六个变量，之后用AI写按定好的依赖顺序开始执行并计时，并在循环中判断标志位来检查每一步是否超时,并用range来限制最多三次尝试按要求输出颜色，如果任务被跳过，通过读取依赖来把所有下游直接依赖和间接依赖的任务全都跳过，极大提高效率，完成所有任务后取得当前时间，与开始时间作差即可得到总耗时。最后调用输出报告函数，输出执行总结。




task_map是读取文件后的字典，task_record是按要求输出的字典