# BytecodeCompiler

A simple bytecode compiler which I'll keep updating every now and than
<br>

```pascal

// example of capabilities

do // scopes
    5
    top
end


// variables
x = 10 
exited=0

// while loop
while x < 100 do 
    x=x+1
    x // this just sets the top value of the stack
    // we access it via 'top'

    if x == 50 do  // if statements 
        exited=1
        goto ::loop_break  // goto statements
    end
    top
end

::loop_break

if exited==1 do 
    1 
    top
else // else statement
    5
    top // currently this is how we print stuff in a stack based machine
end

```

# Bytecode

```
//PROGRAM START
PUSH 0
STORE iterator
>loop_start
SAFETY_LABEL
LOAD iterator
PUSH 1
ADD
STORE iterator
LOAD iterator
PUSH 10000
EQ
GOTOZERO_BUILTIN_IF_ELSE_0
BLOCK_START
BLOCK_START
GOTO_loop_break
BLOCK_END
GOTO_BUILTIN_IF_END_0
>BUILTIN_IF_ELSE_0
SAFETY_LABEL
BLOCK_START
GOTO_loop_start
BLOCK_END
>BUILTIN_IF_END_0
SAFETY_LABEL
GOTO_loop_start
BLOCK_START
>loop_break
SAFETY_LABEL
PUSH 5
PUSH 5
ADD
PUSH 5
TOP
BLOCK_END
CLEANUP
```

# Output

```
5
```

