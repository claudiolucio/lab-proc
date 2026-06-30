from calculator import *
print("Calculadora Binaria Raspberry Pi")
a=parse_binary(input("A (bin): "))
op=input("Operacao (+,-,*,/,!): ").strip()
b=0 if op=="!" else parse_binary(input("B (bin): "))
try:
    r=calculate(a,b,op)
    print("Decimal:",r)
    print("Binario:",to_binary(r))
except Exception as e:
    print("Erro:",e)
