#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MIN(a, b) a < b ? a : b

extern char *memchrasm(void *data, char target, unsigned int count);
extern char *memchr2(void *data, char target, unsigned int count);
extern char *memchr_zbb(void *data, char target, unsigned int count);

static char *buffer, *buffer2;

void test_memchr(int current_size, int count)
{
	clock_t start, end;
	double mean_old = 0, mean_new = 0;
	int i, cnt, offset;
	size_t max_delta = current_size * 5 / 100, delta = 0, size;
	char *res1, *res2;

	for (i = 0; i < count; i++) {
		if (max_delta)
			delta = rand() % max_delta;
		size = current_size - delta;

		offset = rand() % 8;
		if (offset >= size)
			offset = 0;

		memset(buffer, 'a', size + offset);
		buffer[size + 1] = 'b';

		memset(buffer2, 'a', size + offset);
		buffer2[size + 1] = 'b';


		int t = rand() % size;
		start = clock();
		res1 = memchr2((void *)(buffer + offset), 'b', size + t);
		end = clock();

		mean_old += (double)(end - start);

		start = clock();
		res2 = memchr_zbb((void *)(buffer2 + offset), 'b', size + t);
		end = clock();
		
		mean_new += (double)(end - start);
		
		if (res1 - buffer != res2 - buffer2 && !(res1 == res2 && res1 == 0)) {
			printf("Wrong test result. size: %d, offset: %d, res1: %p, res2: %p\n", size, offset, res1 ? res1 - buffer : 0, res2 ? res2 - buffer2 : 0);
			return;
		}
	}

	mean_old /= count;
	mean_new /= count;
	printf("Size: %d, mean old: %lf, mean new: %lf. K: %lf\n", current_size, mean_old, mean_new, mean_old / mean_new);
}


int main(int argc, char *argv[])
{	
	int i, current_size = 1;
	char *s = "held";

	srand(time(NULL));

	printf("%p %p\n", memchr_zbb(s, 'e', 1), memchr(s, 'e', 1));

	buffer = malloc(4 * 1024 * 1024);
	buffer2 = malloc(4 * 1024 * 1024);

	for (i = 0; i < 20; i++) {
		test_memchr(current_size, 1000);
		current_size *= 2;
	}


	free(buffer);
	free(buffer2);

	return 0;
}
