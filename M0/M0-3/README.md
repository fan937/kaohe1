# 问题1：
## 位置：第37行，v=float(row["Value"])
## 现象：KeyError: 'Value'
## 原因： 没有匹配到Value,Value不存在
## 解决方法： 把Value改成value
# 问题2：
## 位置：第61行，output_path = os.path.join( '/',OUTPUT_DIR, OUTPUT_FILE)
## 现象：File "/home/acer/kaohe1/M0/M0-3/sensor_analyzer.py"
## 原因： 写了'/',变成绝对路径
## 解决方法： 去掉'/'
# 问题3：
## 位置：第62行，f = open(output_path, "w")
## 现象：FileNotFoundError:[Errno 2]No such file or directory: '/out/cleaned_data.csv'
## 原因： 没有out这个文件
## 解决方法： 在前面设立条件，如果out文件夹不存在则新建
# 问题4：
## 位置：第52行，acc += (v-mean)
## 现象：std= -0.0000
## 原因： 标准差计算公式错误，先开根后求和
## 解决方法： 先求出平方和再开根
# 问题5：
## 位置：第57，58行，if v > mean + 2 * std: data.remove(v)
## 现象：清洗后剩余 0 条
## 原因： 没有取绝对值，只是把data数组离群值移除，没有转入新数组，
## 解决方法： 反向思考，取绝对值将满足条件的值转入cleaned，
# 问题6：
## 位置：第65，66行，writer.writerow([v])
## 现象：打开cleaned_data.csv，只有time那一列有值，且写入的是value的值
## 原因： cleaned数组只存了value的值，只写入value没写入time
## 解决方法： 再开一个cleaned_times数组，存满足条件的time值，利用循环定义两个变量存每行满足条件的time和value值并写入。
# 问题7：
## 位置：第38行等
## 现象：报错直接traceback,0退出码
## 原因： 没有异常处理
## 解决方法：利用sys实现非零退出，利用with打开文件后并判断列名是否匹配，并读取data的长度判断是否为空文件，用except处理非数据内容和文件不存在的报错。
# 问题8：cleaned_data.csv文件写死路径
## 位置：第61行
## 现象：输出为已保存到out/cleaned_data.csv
## 原因： os.path.join的第一个形参为out，默认加到out文件
## 解决方法：让直接把cleaned_data.csv的当前地址赋值给output_path
# 备注：
## 问题1，2，4，6是本人解决，问题3借用AI写判断条件和新建的代码。问题5询问AI如何写入两个值给输出文件，自己根据它的提示新建数组，实现time和value的同时赋值。问题7借助AI输出判断各种异常情况并实现非零退出的代码。问题8借助AI写修改路径的代码，并让AI创建命令参数解析器来支持两种调用方式。