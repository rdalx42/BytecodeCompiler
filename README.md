# BytecodeCompiler

<h4> A <i>small</i> and <i>compact</i>, extremly <i>memory lightweight</i> on runtime and quite <i>fast</i> programming language </h4>

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

// currently there is no unary operations will add not(), negate() builtins later on
 
```

# Bytecode

```

//PROGRAM START
BLOCK_START
PUSH 5
TOP
BLOCK_END
PUSH 10
STORE x
PUSH 0
STORE exited
>BUILTIN_WHILE_START_0
SAFETY_LABEL
LOAD x
PUSH 100
LT
BLOCK_START
GOTOZERO_BUILTIN_WHILE_END_0
LOAD x
PUSH 1
ADD
STORE x
LOAD x
LOAD x
PUSH 50
EQ
GOTOZERO_BUILTIN_IF_ELSE_1
BLOCK_START
PUSH 1
STORE exited
GOTO_loop_break
BLOCK_END
>BUILTIN_IF_ELSE_1
SAFETY_LABEL
TOP
GOTO_BUILTIN_WHILE_START_0
>BUILTIN_WHILE_END_0
SAFETY_LABEL
BLOCK_END
>loop_break
SAFETY_LABEL
LOAD exited
PUSH 1
EQ
GOTOZERO_BUILTIN_IF_ELSE_2
BLOCK_START
PUSH 1
TOP
BLOCK_END
GOTO_BUILTIN_IF_END_2
>BUILTIN_IF_ELSE_2
SAFETY_LABEL
BLOCK_START
PUSH 5
TOP
BLOCK_END
>BUILTIN_IF_END_2
SAFETY_LABEL
CLEANUP
Stack top: 5
Stack top: 11
Stack top: 12
Stack top: 13
...

Stack top: 1
```



