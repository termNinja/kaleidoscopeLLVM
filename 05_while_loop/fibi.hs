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


def fibi(n)
var a = 1, b = 1, c = 1 in
(
    while n > 2 do
    (
        c = a + b:
        a = b:
        b = c:
        n = n - 1
    )
    :
    c
);

