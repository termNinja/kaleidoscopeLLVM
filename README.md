# Kaleidoscope Language in LLVM
Code for the official LLVM tutorial of Kaleidoscope language (using yacc and flex).
You can find code [here](http://llvm.org/docs/tutorial/index.html).
I recommend going through the official tutorial, but what I recommend
even more is to first gain a deep understanding of the LLVM IR (LLVM intermediate representation) language.
That doesn't mean that you need to learn it whole, but you should at least understand instructions:
* `alloca`
* `fadd`, `fsub`, ...
* `add`, `sub`, ...
* `load`
* `store`
* `ret`

## Building it up
Here's a list:
* LLVM version 3.6 at least (my version: 3.8.1)
* clang++ compiler (which you get from LLVM)
* bison (my version: 3.0.4)
* flex (my version: 2.6.1)
* make (my version 4.2.1)

`cd` into a directory you like and invoke `make`.
You can then run `kaleidoscope` executable.

## Hint about learning LLVM IR
You can easily get LLVM IR from a simple c program using clang compiler.
For example, let's assume we wrote `main.c` that looks like:

```c
int main() {
	int x = 10;
	int y = 20;
	int z = x + y;
}
```

We invoke the following:
```shell
$ clang -emit-llvm main.c -c -S
$ ls
main.c main.ll
```

And when we open `main.ll` we get:
```
; ModuleID = 'main.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %z = alloca i32, align 4
  store i32 10, i32* %x, align 4
  store i32 20, i32* %y, align 4
  %1 = load i32, i32* %x, align 4
  %2 = load i32, i32* %y, align 4
  %3 = add nsw i32 %1, %2
  store i32 %3, i32* %z, align 4
  ret i32 0
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.1 (tags/RELEASE_381/final)"}
```

Understanding LLVM IR before beginning to generate it from programs is a graceful path towards greatness.

Not understanding LLVM IR is a gracefull path to drowning yourself in mud. 

Choose *wisely!* :D

## Contents
* `00_no_llvm` contains no LLVM, only Kaleidoscope language with parser and lexer;
* `01_simple_ir_gen` contains LLVM for basic things without control flow;
* `02_adding_jit` contains added interpretation for function calls;
* `03_if_for` contains code with added support for *if-then-else* and *for* control flow;
* `04_var` contains support for *var-in* expression (assigning values to variables).

## Sample programs
Here you can find some programs you can try to test it.
Obviously not all test samples can be applied to every version
of the Kaleidoscope language.
To see calculated results you shall need the JIT!

##### Extern function sin and cos
```
extern sin(x);
extern cos(x);
sin(3.14159265358979323846264338327950288419716939937510582097494459230);
cos(3.14159265358979323846264338327950288419716939937510582097494459230);
```

##### Extern function exit
```
extern exit();
exit();
```

##### Recursive fibonacci
```
def fib(n)
	if n < 3 then 1 else fib(n-1) + fib(n-2);

fib(10);
```

##### Recursive factorial
```
def fact(n) if n < 1 then 1 else n * fact(n-1)
```

##### Recursive sum
```
def sum(n) if n < 2 then 1 else n + sum(n-1);
```

##### Iterative sum
```
def sum(n)
var s in
(
	for i = 1, i < n+1, 1.0 in
	(
		s = s + i
	)
	:
	n = s
);
```

##### Iterative fibonacci
```
def fibi(n)
var a = 1, b = 1, c = 1 in 
(
    for i = 2, i < n, 1.0 in
    (
        c = a + b:
        a = b:
        b = c
    ):
    c
);

fibi(10);
```
## Closing thoughts
Have fun.
And LLVM is actually really cool and powerfull. I was honestly
suprised at how easy it was to construct a compiler for an abstract language.

Oh and I always indent using tabs for various reasons. Make sure to set tab size to 4!
