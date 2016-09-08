#! /bin/sh

DOT1 = cfg.main.dot
DOT2 = cfg.add.dot

all: unoptimized_ir.ll optimized_ir.ll $(DOT1) $(DOT2) main.png add.png

unoptimized_ir.ll: main.c
	clang -emit-llvm -c -S $< -o $@

# get optimized ir
optimized_ir.ll: main.c
	clang -emit-llvm -c -S -O3 $< -o $@

# generate drawing for unoptimized ir
$(DOT1) $(DOT2): unoptimized_ir.ll
	opt -dot-cfg $<

main.png: $(DOT1)
	dot -Tpng $< > $@

add.png: $(DOT2)
	dot -Tpng $< > $@

.PHONY: clean

clean:
	rm -rf *.dot *.ll *.png
