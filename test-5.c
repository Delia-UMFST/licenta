#include <stdio.h>

void test_file_io_no_flush(){
	char buf[10];
    FILE *f = fopen("test.txt", "w+");
    fprintf(f, "Hello\n");
    fscanf(f, "%s", buf);
    fclose(f);
}

int main(){
	test_file_io_no_flush();
	return 0;
}
