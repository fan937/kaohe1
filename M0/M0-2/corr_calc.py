import numpy
import csv 
def pearson_correlation(x,y):
    if len(x)!=len(y):
        raise ValueError("两组数据长度不同")
    n=len(x)
    sumx=sum(x)
    sumy=sum(y)
    avgx=sum(x)/n
    avgy=sum(y)/n
    fenzi=sum((xi-avgx)*(yi-avgy)for xi,yi in zip(x,y))
    sumxx=sum((xi-avgx)**2 for xi in x)
    sumyy=sum((yi-avgy)**2 for yi in y)
    fenmu=(sumxx*sumyy)**0.5
    if fenmu==0:
        raise ZeroDivisionError("某一列取值恒定,方差为0,相关系数无定义")
    return fenzi/fenmu

def read_csv_data(csv_path,col1,col2):
    try:
        f=open(csv_path,"r",encoding="utf-8")
    except FileNotFoundError:
            raise FileNotFoundError("文件不存在")
    with f:
         reader=csv.DictReader(f)
         rows=list(reader)

         if len(rows)==0:
              raise ValueError("空文件")
         if col1 not in reader.fieldnames or col2 not in reader.fieldnames:
              raise KeyError("列名不匹配")
         
         x=[float(row[col1]) for row in rows]
         y=[float(row[col2]) for row in rows]

    return x,y
if __name__=="__main__":
    a=[1,2,3]
    b=[2,4,6]
    print("计算结果为：",pearson_correlation(a,b))
    print("公式结果为：",numpy.corrcoef(a,b)[0,1])
          