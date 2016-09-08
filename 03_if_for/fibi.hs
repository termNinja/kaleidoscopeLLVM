def fib(n)
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

