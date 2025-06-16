#include <stdlib.h>

void test_use_after_free(){
	int *p = (int *)malloc(sizeof(int));
  	free(p);
  	*p = 42;
}

void test_no_use_after_free(){
	int *p = (int *)malloc(sizeof(int));
  	*p = 42;
  	free(p);
}

int main(){
	test_use_after_free();
	test_no_use_after_free();
	return 0;
}
