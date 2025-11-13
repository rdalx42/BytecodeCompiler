# Vexa 1.1.0

<h4> A <i>small</i> and <i>compact</i>, <i>lightweight</i> and <i>fast</i> home-made VM programming language </h4>
<i> made in around ~2.3k LOC, runs a 1-1million loop in ~0.049s</i><br>
<br>



> This is Vexa 1.1, keep in mind that everything is subject to change.<br>
> Error handling is currently a bit of a hit or miss.<br>
> Everything is in early stages and there are more features to come!
> Small VM bugs can occur!

<br>

<h1> Short Demo</h1>

<h3>Variables</h3>    

```Pascal
// Variables in Vexa //

int_var = 50 

float_variable = 21.42 + 42.11/533 + (5+int_var)

string_variable = "hello_world"

string_variable = string_variable + string_variable[0]
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
    Functions in Vexa have a simple design, due to this they have some restrictions:
</h4>

<ul>
    <li>Functions cannot be nested inside other functions.</li>
    <li>Functions can't recurse themselves inside their body without a return statement</li>
    <li>Raw function calls cannot be used in binary operations (placeholder name required)</li>
</ul>

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

# How to compile

Make sure you have a C++20+ compiler installed (g++ -as it was mainly tested on it).

## Windows
```bash
cd src  # go into main directory
g++ runtime/*.cpp compiler/*.cpp backend/*.cpp -O3 -ffast-math -march=native -Iinclude -o vexa.exe  # build executable
vexa.exe  # run
```
## Linux/MacOs
```bash
cd src  # go into main directory
g++ runtime/*.cpp compiler/*.cpp backend/*.cpp -O3 -ffast-math -march=native -Iinclude -o vexa  # build executable
./vexa  # run
```
