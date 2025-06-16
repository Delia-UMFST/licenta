#include <stdio.h>

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

int main(){
	test_string_terminator_bad();
	test_string_terminator_ok_1();
	test_string_terminator_ok_2();
	return 0;
}
