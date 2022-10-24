CC_FLAGS = -Wall -Werror -Wextra
CC = gcc
NAME = s21_decimal.a test
TEST_LIBS = -lcheck -lm -lpthread
OBJECTS =  s21_decimal.o
SOURSES =  s21_decimal.c

.PHONY: all clean test s21_decimal.a gcov_report

all: $(NAME)

test: s21_decimal_test.o s21_decimal.a $(OBJECTS)
	$(CC) $(OBJECTS) s21_decimal_test.o s21_decimal.a $(TEST_LIBS) -o test
	./test

main: main.o s21_decimal.o
	$(CC) main.o s21_decimal.o -o main
	./main

s21_decimal.o: s21_decimal.c
	$(CC) $(CC_FLAGS) -c s21_decimal.c -g

s21_decimal_test.o: s21_decimal_test.c	
	$(CC) $(CC_FLAGS) -c  s21_decimal_test.c

gcov_report: s21_decimal.a 
	$(CC) $(CC_FLAGS) --coverage s21_decimal_test.c s21_decimal.c  s21_decimal.a $(TEST_LIBS) -o gcov_test
	chmod +x *
	./gcov_test
	lcov -t "gcov_test" -o gcov_test.info --no-external -c -d .
	genhtml -o report/ gcov_test.info
	open ./report/index.html

check: s21_decimal.a
	cppcheck *.h *.c
	cp ../materials/linters/CPPLINT.cfg CPPLINT.cfg
	python3 ../materials/linters/cpplint.py --extension=c *.c *.h
	$(CC) s21_decimal_test.c s21_decimal.a -lcheck
	$(CC) $(CC_FLAGS) -c s21_decimal_test.c -o test.o
	$(CC) test.o s21_decimal.a $(TEST_LIBS) -o test
	CK_FORK=no leaks --atExit -- ./test

s21_decimal.a: s21_decimal.o
	$(CC) -c $(SOURSES)
	ar rcs s21_decimal.a $(OBJECTS)

clean:
	rm -rf *.o test s21_decimal.a s21_decimal_test.c.gcov s21_decimal.c.gcov s21_decimal_test.gc* s21_decimal.gc* report gcov_report.info CPPLINT.cfg a.out gcov_test*

lo:
	for i in `seq 100 $(OP)`;	do ./test; done;

vg: s21_decimal.a
	CK_FORK=no valgrind --tool=memcheck ./test
	 
lik: s21_decimal.a
	CK_FORK=no leaks -atExit -- ./test

install lcov:
	curl -fsSL https://rawgit.com/kube/42homebrew/master/install.sh | zsh
	brew install lcov