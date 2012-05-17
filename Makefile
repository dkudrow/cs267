# Makefile
CC = gcc
CFLAGS = -g -Wall
TARGET = dmc
OBJ = main.o ll2_parser.o ast.o cfg.o draw.o
SRCDIR = .
TESTDIR = testing
TEST = test.verf

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

clean:
	rm -f $(OBJ) $(TARGET) *~

test: $(TARGET)
	./$(TARGET) < $(TESTDIR)/$(TEST) > $(TESTDIR)/$(TEST).dot
	dot -Tpng $(TESTDIR)/$(TEST).dot -O
	display $(TESTDIR)/$(TEST).dot.png