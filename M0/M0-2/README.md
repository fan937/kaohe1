在WSL终端进入项目目录M0/M0-2，执行下面的命令，通过--config指定yaml配置文件:python3
corr_calc.py --config config.yaml
输入文件格式要求：yaml包含两个字段，第一个是input_csv用来指定CSV数据文件的路径,第
二个是columns。
CSV需要的列：sensor_a、sensor_b
列名规则：列名在csv的第一行，列里面的内容必须是数字。
输出结果含义：n是样本数量，mean_x是sensor_a的平均值，mean_y是sensor_b的平均值，r是
皮尔逊相关系数，范围[-1，1]
本代码中函数的封装、算法错误的修复和异常处理由本人完成，通过AI增加读取yaml和csv配
置,支持——config指定配置文件路径，并通过AI检查遗漏的异常处理和写test.py。我逐行研究
过AI写的代码，理解其含义和逻辑。

