# BytecodeCompiler

A simple bytecode compiler which I'll keep updating every monday


```
x = 5 + 3
y = -2
-y-(-x)
top
```

# Bytecode

```
PUSH 5
PUSH 3
ADD
STORE x
PUSH 2
NEG
STORE y
LOAD y
NEG
LOAD x
NEG
SUB
TOP
CLEANUP
```

# Output

```
10
```

