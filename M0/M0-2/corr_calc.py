def pearson_correlation(x,y):
    n=len(x)
    sumx=sum(x)
    sumy=sum(y)
    sumxy=sum(xi*yi for xi,yi in zip (x,y))
    sumxx=sum(xi**2 for xi in x)
    sumyy=sum(yi**2 for yi in y)
    fenzi=n*sumxy-sumx*sumy
    fenmu=((n*sumxx-sumx**2)*(n*sumyy-sumy**2))**0.5
    return fenzi/fenmu