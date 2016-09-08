CXX = clang++
CXXFLAGS := -g $(shell llvm-config --cxxflags)
LDFLAGS := -g $(shell llvm-config --ldflags --system-libs --libs core native mcjit)

kaleidoscope: lex.yy.o parser.tab.o ast.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

parser.tab.o: parser.tab.cpp parser.tab.hpp ast.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

parser.tab.cpp parser.tab.hpp: parser.ypp
	bison -d -v $<

lex.yy.o: lex.yy.c parser.tab.hpp ast.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

lex.yy.c: lexer.lex
	flex $<

ast.o: ast.cpp ast.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean

clean:
	rm -rf *~ *tab* lex.yy.* *.o kaleidoscope *.output
