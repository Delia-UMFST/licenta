#include <stdlib.h>

void test_use_after_free(){
	int *p = (int *)malloc(sizeof(int));
  	free(p);
  	*p = 42;
}

int main(){
	test_use_after_free();
	return 0;
}
