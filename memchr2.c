#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 200
#define TEST_COUNT 100000

extern char *memchrasm(void *data, char target, unsigned int count);
extern char *memchr2(void *data, char target, unsigned int count);
char *data;

void *memchr_c(const void *s, int c, size_t n)
{
	const unsigned char *p = s;
	while (n-- != 0) {
        	if ((unsigned char)c == *p++) {
			return (void *)(p - 1);
		}
	}
	return NULL;
}

void test(void)
{
	clock_t start, end;
	double mean = 0, mean2 = 0;
	int i, num, cnt, offset;
	char *res1, *res2;

	for (i = 0; i < TEST_COUNT; i++) {
		printf("Running test number %d\n", i);
		num = rand() % (SIZE / 4);
		offset = rand() % (SIZE / 4);
		cnt = SIZE - offset - 1;
		data[SIZE - 1 - num] = 'b';
		start = clock();
		res1 = memchrasm((void *)(data + offset), 'b', cnt);
		end = clock();
		mean += (double)(end - start);
		start = clock();
		res2 = memchr2((void *)(data + offset), 'b', cnt);
		end = clock();
		printf("%p %p %p %d %d %d\n", data, res1, res2, num, cnt, offset);
		mean2 += (double)(end - start);
		if (res1 != res2) {
			printf("AHTUNG!!!\n");
			return;
		}
		data[SIZE - 1 - num] = 'A';
	}

	mean /= TEST_COUNT;
	mean2 /= TEST_COUNT;

	printf("Mean for old func: %lf; Mean for new: %lf\n", mean2, mean);
	printf("Efficiency coefficient: %lf\n", mean2 / mean);
}

void fill_array(void)
{
	int i;

	for (i = 0; i < SIZE; i++)
		data[i] = 'A';
}

int main(void)
{
	srand(time(NULL));

	data = calloc(SIZE, 1);

	fill_array();

	test();

	free(data);
	return 0;
}
