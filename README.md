# BytecodeCompiler

<h4> A <i>small</i> and <i>compact</i>, extremly <i>memory lightweight</i> on runtime and quite <i>fast</i> home-made programming language </h4>

<br>

```pascal
// example of capabilities

do // scopes
    5
    top
end

// variables
x = 10 
exited = 0

// string variables
greeting = "hi "
username = "idk"
message = greeting + ", " + username  // string concatenation
message
top  // push result of string concatenation to stack

// while loop
while x < 100 do 
    x = x + 1
    x // this just sets the top value of the stack

    if x == 50 do  // if statements 
        exited = 1
        goto ::loop_break  // goto statements
    end
    top
end

::loop_break

if exited == 1 do 
    "exited early"
    top
else // else statement
    5
    top
end

// string manipulation
welcome_msg = "Welcome "
user = "idkwho"
welcome_msg = welcome_msg + user
welcome_msg
top

// for loops
for i = 0, 5 do
    
    i
    top
end


for i , 2 do // < -- by default i will be zero if nothing is declared
    for j = 0, 2 do
        x = i * 10 + j // nested for loopa
        x
        top
    end
end
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
PUSH "hi "
STORE greeting
PUSH "idk"
STORE username
LOAD greeting
PUSH ", "
ADD
LOAD username
ADD
STORE message
LOAD message
TOP
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
PUSH "exited early"
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
PUSH "Welcome "
STORE welcome_msg
PUSH "idkwho"
STORE user
LOAD welcome_msg
LOAD user
ADD
STORE welcome_msg
LOAD welcome_msg
TOP
PUSH 0
STORE i
>BUILTIN_FOR_START_3
SAFETY_LABEL
LOAD i
PUSH 5
LTE
GOTOZERO_BUILTIN_FOR_END_3
BLOCK_START
LOAD i
TOP
BLOCK_END
LOAD i
PUSH 1
ADD
STORE i
GOTO_BUILTIN_FOR_START_3
>BUILTIN_FOR_END_3
SAFETY_LABEL
PUSH 0
STORE i
>BUILTIN_FOR_START_4
SAFETY_LABEL
LOAD i
PUSH 2
LTE
GOTOZERO_BUILTIN_FOR_END_4
BLOCK_START
PUSH 0
STORE j
>BUILTIN_FOR_START_5
SAFETY_LABEL
LOAD j
PUSH 2
LTE
GOTOZERO_BUILTIN_FOR_END_5
BLOCK_START
LOAD i
PUSH 10
MUL
LOAD j
ADD
STORE x
LOAD x
TOP
BLOCK_END
LOAD j
PUSH 1
ADD
STORE j
GOTO_BUILTIN_FOR_START_5
>BUILTIN_FOR_END_5
SAFETY_LABEL
BLOCK_END
LOAD i
PUSH 1
ADD
STORE i
GOTO_BUILTIN_FOR_START_4
>BUILTIN_FOR_END_4
SAFETY_LABEL
CLEANUP
...

Stack top: 5
Stack top: hi, idk
Stack top: 11
Stack top: 12
Stack top: 13

....

Stack top: exited early
Stack top: Welcome idkwho

....

Stack top: 0
Stack top: 1
Stack top: 2
Stack top: 3

...

Execution finished in 0.003 seconds.
```



