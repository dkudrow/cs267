# Makefile
CC = gcc
CFLAGS = -g -Wall
TARGET = dmc
OBJ = main.o ll2_parser.o ast.o cfg.o draw.o
SRCDIR = .
TESTDIR = testing
TEST = test.verf
# Cudd libraries and inlcudes
CUDDLIB = ./cudd/lib/libcudd.a ./cudd/lib/libmtr.a ./cudd/lib/libst.a ./cudd/lib/libutil.a ./cudd/lib/libepd.a
CUDDINC = ./cudd/include

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)
	rm -f $(OBJ) *~

%.o: %.c
	$(CC) $(CFLAGS) -c $<

main.o: ast.h cfg.h
ll2_parser.0: ast.h
ast.o: ast.h
cfg.o: ast.h cfg.h
draw.o: ast.h cfg.h

cudd: testcudd.c
	$(CC) -c testcudd.c -I$(CUDDINC) -g -DDD_STATS
	$(CC) -g -DDD_STATS -o testcudd testcudd.o $(CUDDLIB) -lm

clean:
	rm -f $(OBJ) $(TARGET) *~

test: $(TARGET)
	./$(TARGET) < $(TESTDIR)/$(TEST) > $(TESTDIR)/$(TEST).dot
	dot -Tpng $(TESTDIR)/$(TEST).dot -O
	display $(TESTDIR)/$(TEST).dot.png
