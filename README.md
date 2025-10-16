# BytecodeCompiler

A simple bytecode compiler which I'll keep updating every monday


```lua

iterator=0 // variable declarations 

::loop_start // goto label
    iterator=iterator+1 
    if iterator == 10000 do // if statements 
        
        goto ::loop_break // goto statemept
    else // else brach
      goto ::loop_start
    end // end of block
    
goto ::loop_start

do  // <-- scope block [do,end]
  ::loop_break
  5+5
  5
  top // top of stack call
end

// currently there is no unary minus operation, i'll add a builtin called neg() for that in the near future
```

# Bytecode

```
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
>BUILTIN_IF_ELSE_0
SAFETY_LABEL
GOTO_loop_start
>loop_break
SAFETY_LABEL
PUSH 5
PUSH 5
ADD
PUSH 5
TOP
CLEANUP
```

# Output

```
5
```

