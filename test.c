#include <stdlib.h>
#include <stdio.h>
void test_mixed_signedness_1(){
	int a = -2;
	unsigned c=6;
	int d = a|c;
}
int signed_function() { return -2; }
unsigned int unsigned_function() { return 6; }
void test_mixed_signedness_2(){
	int a = signed_function() & unsigned_function();
}

void test_use_after_free(){
	int *p = (int *)malloc(sizeof(int));
  	free(p);
  	*p = 42;
}

void test_string_terminator_bad() {
    char unsafe[3] = { 'a', 'b', 'c' };
    puts(unsafe);
}
void test_string_terminator_ok_1() {
    char safe[4] = { 'a', 'b', 'c', '\0' };
    puts(safe);
}
void test_string_terminator_ok_2() {
    char literal[] = "abc";
    puts(literal);
}

void test_pointer_arithmetic_bad() {
    int x;
    int *p = &x;
    p = p + 1;
}
void test_pointer_arithmetic_ok() {
    int arr[5];
    int *p = arr;
    p = p + 1;
}

void test_file_io_no_flush(){
	char buf[10];
    FILE *f = fopen("test.txt", "w+");
    fprintf(f, "Hello\n");
    fscanf(f, "%s", buf);
    fclose(f);
}

int main(){
	test_mixed_signedness_1();
	test_mixed_signedness_2();
	
	test_use_after_free();
	
	test_string_terminator_bad();
	test_string_terminator_ok_1();
	test_string_terminator_ok_2();
	
	test_pointer_arithmetic_bad();
	test_pointer_arithmetic_ok();
	
	test_file_io_no_flush();
	
	return 0;
}


