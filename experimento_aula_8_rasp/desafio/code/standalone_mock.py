from calculator import *
while True:
    try:
        a=parse_binary(input("A: "))
        op=input("Op: ")
        b=0 if op=="!" else parse_binary(input("B: "))
        r=calculate(a,b,op)
        print("[LCD] Dec:",r)
        print("[LCD] Bin:",to_binary(r))
    except Exception as e:
        print("[LCD] Erro:",e)
    if input("Continuar? (s/n): ").lower()!="s":
        break
