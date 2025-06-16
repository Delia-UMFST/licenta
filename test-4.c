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

int main(){
	test_pointer_arithmetic_bad();
	test_pointer_arithmetic_ok();
	return 0;
}
