from calculator import calculate
from time import perf_counter_ns
from statistics import mean,stdev
import csv
ops=["+","-","*","/","!"]
bits_list=[4,8,16,32,64,128]
rows=[]
for bits in bits_list:
    for op in ops:
        a=(1<<bits)-1 if op!="!" else min(bits,20)
        b=max(1,(1<<(max(bits//2,1)))-1)
        ts=[]
        for _ in range(30):
            s=perf_counter_ns(); calculate(a,b if op!="!" else 0,op); e=perf_counter_ns(); ts.append(e-s)
        rows.append([bits,op,mean(ts),stdev(ts),min(ts),max(ts)])
with open("../results/raspberry.csv","w",newline="") as f:
    w=csv.writer(f); w.writerow(["bits","op","mean_ns","std_ns","min_ns","max_ns"]); w.writerows(rows)
print("Benchmark concluido.")
