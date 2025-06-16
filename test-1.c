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

int main(){
	test_mixed_signedness_1();
	test_mixed_signedness_2();
	return 0;
}
