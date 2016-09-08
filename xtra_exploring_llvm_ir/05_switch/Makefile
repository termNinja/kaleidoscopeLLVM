#! /bin/sh

DOT = cfg.example.dot

all: unoptimized_ir.ll optimized_ir.ll $(DOT) example.png

unoptimized_ir.ll: main.c
	clang -emit-llvm -c -S $< -o $@

# get optimized ir
optimized_ir.ll: main.c
	clang -emit-llvm -c -S -O3 $< -o $@

# generate drawing for unoptimized ir
$(DOT): unoptimized_ir.ll
	opt -dot-cfg $<

example.png: $(DOT)
	dot -Tpng $< > $@

.PHONY: clean

clean:
	rm -rf *.dot *.ll *.png
