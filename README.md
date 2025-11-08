# Vexa 1.0 

<h4> A <i>small</i> and <i>compact</i>, <i>lightweight</i> on runtime and quite <i>fast</i> home-made VM programming language </h4>
<i> made in around ~2.1k LOC, quite fast in hotloops</i><br>
<br>



> This is Vexa 1.0, keep in mind that everything is subject to change.<br>
> Error handling is currently a bit of a hit or miss.<br>
> Everything is in early stages and there are more features to come!

<br>

<h1> Short Demo</h1>

<h3>Variables</h3>    

```Pascal
// Variables in Vexa //

integer_variable = 50 

float_variable = 21.42 + 42.11/533 + (5+integrer_variable)

string_variable = "hello_world"

string_variable += string_variable[0]
```

<h3>Unary Operations</h3>

```Pascal
// Unary ops in Vexa
is_zero = !(0 + -(5) + +(-5)) // returns 1
```

<h3>Conditionals</h3>

```Pascal
// Conditionals in Vexa
condition = 5+5

if condition >= 5 do
    "Yes"
    top // currently we print things this way
else
    "Hello"
    top
end

```

<h3>Loops</h3>

```Pascal

// for loops

for i=0, 1000 do // <-- by default i is 0 so i=0 isn't necessary 
    i
    top
end

// while loops

i=0

while i < 10000 do
    i=i+1
end

```

<h3>Goto Labels</h3>

```Pascal

// Goto Labels

::hi 

goto ::hi

// goto labels can also have invalid placement such as when jumping in a scope of same level

```

<h3>Functions (Beta)</h3>

<h4>
    Functions in Vexa have a simple design, a function can  <br>
    only recurse itself if called in return statement, speaking of which <br>
    a function can't be returned in a binary operation, empty do...end scopes can't be amde within functions
</h4>

```Pascal
// Functions in Vexa
func say(x) do
    return x 
end

func Count(n) do
    z=say(2)
    if n == 100 do
        return n
    end
    return Count(n + 1)
end

C = Count(1)
C top
```
