# A Makefile for simple lex and yacc examples

# Comment out the proper lines below according to the scanner and
# parser generators available in your system

LEX = lex
YACC = yacc -d
# LEX = flex 
# YACC = bison -d

# your C-compiler is called gcc

CC = cc

# ijcompiler is the final object that we will generate, it is produced by
# the C compiler from the y.tab.o and from the lex.yy.o

ijcompiler: y.tab.o lex.yy.o functions.o symbol_table.o print.o semantics.o codeGeneration.o
	$(CC) -o ijcompiler y.tab.o lex.yy.o functions.o symbol_table.o print.o semantics.o codeGeneration.o -ll -lm 

functions.o: functions.c
symbol_table.o: symbol_table.c
print.o:  print.c
semantics.o: semantics.c
codeGeneration.o: codeGeneration.c

## This rule will use yacc to generate the files y.tab.c and y.tab.h
## from our file calc.y

y.tab.c y.tab.h: ijcompiler.y
	$(YACC) -v ijcompiler.y

## this is the make rule to use lex to generate the file lex.yy.c from
## our file calc.l

lex.yy.c: ijcompiler.l
	$(LEX) ijcompiler.l

## Make clean will delete all of the generated files so we can start
## from scratch

clean:
	-rm -f *.o lex.yy.c *.tab.*  calc *.output
