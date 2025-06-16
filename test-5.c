#include <stdio.h>

void test_file_io_no_flush(){
	char buf[10];
    FILE *f = fopen("test.txt", "w+");
    fprintf(f, "Hello\n");
    fscanf(f, "%s", buf);
    fclose(f);
}

void test_file_io_with_flush(){
	char buf[10];
    FILE *f = fopen("test.txt", "w+");
    fprintf(f, "Hello\n");
    fflush(f);
    fscanf(f, "%s", buf);
    fclose(f);
}

int main(){
	test_file_io_no_flush();
	test_file_io_with_flush();
	return 0;
}
