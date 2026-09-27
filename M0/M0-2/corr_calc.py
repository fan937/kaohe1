import numpy 
def pearson_correlation(x,y):
    n=len(x)
    sumx=sum(x)
    sumy=sum(y)
    avgx=sum(x)/n
    avgy=sum(y)/n
    fenzi=sum((xi-avgx)*(yi-avgy)for xi,yi in zip(x,y))
    sumxx=sum((xi-avgx)**2 for xi in x)
    sumyy=sum((yi-avgy)**2 for yi in y)
    fenmu=(sumxx*sumyy)**0.5
    return fenzi/fenmu

if __name__=="__main__":
    a=[1,2,3]
    b=[2,4,6]
    print("计算结果为：",pearson_correlation(a,b))
    print("公式结果为：",numpy.corrcoef(a,b)[0,1])
          