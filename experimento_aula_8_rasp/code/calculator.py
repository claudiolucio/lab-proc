def parse_binary(text: str) -> int:
    text = text.strip().lower().replace("0b", "")
    if text == "":
        raise ValueError("entrada vazia")
    if any(c not in "01" for c in text):
        raise ValueError("digite apenas 0 e 1")
    return int(text, 2)

def to_binary(value: int) -> str:
    return "-" + bin(abs(value))[2:] if value < 0 else bin(value)[2:]

def add(a,b): return a+b
def subtract(a,b): return a-b
def multiply(a,b): return a*b
def divide(a,b):
    if b==0: raise ZeroDivisionError("divisao por zero")
    return a//b
def factorial(a):
    if a<0: raise ValueError("fatorial nao definido")
    r=1
    for i in range(2,a+1): r*=i
    return r
def calculate(a,b,op):
    return {"+":add,"-":subtract,"*":multiply,"/":divide}.get(op, lambda x,y: factorial(x))(a,b)
