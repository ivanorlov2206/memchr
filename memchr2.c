#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN(a, b) a < b ? a : b

struct test_result {
	const struct test_description *desc;

	double mean_old;
	double mean_new;
};

struct test_description {
	int count;
	int array_size;
};

extern char *memchrasm(void *data, char target, unsigned int count);
extern char *memchr2(void *data, char target, unsigned int count);

extern int memcmp_asm(void *src1, void *src2, int count);
extern int memcmp_orig(void *src1, void *src2, int count);

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

void test_memcmp(const struct test_description *desc, struct test_result *result)
{
	unsigned char *buffer1 = calloc(desc->array_size, 1);
	unsigned char *buffer2 = calloc(desc->array_size, 1);

	clock_t start, end, start2, end2;
	int i, res1, res2;
	double mean_new = 0, mean_old = 0;

	result->desc = desc;

	for (i = 0; i < desc->count; i++) {
		int num = desc->array_size - (rand() % (desc->array_size / 10)) - 1;
		buffer1[num] = rand() % 2;
		buffer2[num] = rand() % 2;
		int st = rand() % (desc->array_size / 20);
		int st2 = rand() % (desc->array_size / 20);
		int cnt = MIN(desc->array_size - st, desc->array_size - st2);

		start = clock();
		res1 = memcmp_asm(buffer1 + st, buffer2 + st2, cnt);
		end = clock();
		start2 = clock();
		res2 = memcmp_orig(buffer1 + st, buffer2 + st2, cnt);
		end2 = clock();
		mean_new += end - start;
		mean_old += end2 - start2;

		if (res1 * res2 < 0) {
			printf("AHTUNG! %d %d\n", res1, res2);
			return;
		}

		if (res1 == 0x1337) {
			printf("Impossible happened... st: %d, num: %d, cnt: %d\n", st, num, cnt);
			return;
		}

		buffer1[num] = 0;
		buffer2[num] = 0;
	}

	mean_new /= desc->count;
	mean_old /= desc->count;

	result->mean_old = mean_old;
	result->mean_new = mean_new;

	free(buffer1);
	free(buffer2);

}

void test_memchr(const struct test_description *desc, struct test_result *result)
{
	unsigned char *data = calloc(desc->array_size, 1);
	clock_t start, end;
	double mean_old = 0, mean_new = 0;
	int i, cnt, offset;
	char *res1, *res2;

	result->desc = desc;
	memset(data, 'A', desc->array_size);

	data[desc->array_size - 1] = 'b';
	for (i = 0; i < desc->count; i++) {
		//printf("Running test number %d\n", i);
		offset = rand() % 8;
		cnt = desc->array_size - offset;
		start = clock();
		res1 = memchr2((void *)(data + offset), 'b', cnt);
		end = clock();
		mean_old += (double)(end - start);
		start = clock();
		res2 = memchrasm((void *)(data + offset), 'b', cnt);
		end = clock();
		mean_new += (double)(end - start);
		if (res1 != res2) {
			printf("Wrong test result. size: %d, offset: %d, count: %d\n", desc->array_size, offset, cnt);
			goto out_err;
		}
	}

	mean_old /= desc->count;
	mean_new /= desc->count;

	result->mean_old = mean_old;
	result->mean_new = mean_new;
out_err:
	free(data);
}

const struct test_description tests[] = {
	{
		.array_size = 10,
		.count = 10000,
	},
	{
		.array_size = 100,
		.count = 10000,
	},
	{
		.array_size = 128,
		.count = 10000,
	},
	{
		.array_size = 256,
		.count = 10000,
	},
	{
		.array_size = 512,
		.count = 5000,
	},
	{
		.array_size = 768,
		.count = 5000,
	},
	{
		.array_size = 1024,
		.count = 5000,
	},
	{
		.array_size = 1500,
		.count = 5000,
	},
	{
		.array_size = 2048,
		.count = 5000,
	},
	{
		.array_size = 4096,
		.count = 3000,
	},
	{
		.array_size = 1024 * 16,
		.count = 3000,
	},
	{
		.array_size = 1024 * 512,
		.count = 1000,
	},
	{
		.array_size = 1024 * 1024,
		.count = 1000,
	},
	{
		.array_size = 1024 * 1024 * 10,
		.count = 500,
	},
	{
		.array_size = 1024 * 1024 * 128,
		.count = 100,
	},
	{
		.array_size = 1024 * 1024 * 512,
		.count = 20,
	},
	{
		.array_size = 1024 * 1024 * 1024,
		.count = 20,
	},
};

int main(int argc, char *argv[])
{
	int count = sizeof(tests) / sizeof(tests[0]);
	struct test_result *results_memchr = calloc(sizeof(struct test_result), count);
	struct test_result *results_memcmp = calloc(sizeof(struct test_result), count);
	int i;

	srand(time(NULL));

	for (i = 0; i < count; i++) {
		printf("Running memchr %d/%d: count = %d, array size = %d\n", (i + 1), count, tests[i].count, tests[i].array_size);
		test_memchr(&tests[i], &results_memchr[i]);
		printf("Running memcmp %d/%d: count = %d, array size = %d\n", (i + 1), count, tests[i].count, tests[i].array_size);
		test_memcmp(&tests[i], &results_memcmp[i]);
	}

	printf("memchr\n");
	printf("------\n");
	printf("| test_count | array_size | old_func_cycles | new_func_cycles |     K    |\n");
	printf("--------------------------------------------------------------------------\n");

	for (i = 0; i < count; i++) {
		printf("| %10d | %10d | %15lf | %15lf | %5lf |\n", results_memchr[i].desc->count, results_memchr[i].desc->array_size,
		       results_memchr[i].mean_old, results_memchr[i].mean_new, results_memchr[i].mean_old / results_memchr[i].mean_new);
	}
	printf("memcmp\n");
	printf("------\n");
	printf("| test_count | array_size | old_func_cycles | new_func_cycles |     K    |\n");
	printf("--------------------------------------------------------------------------\n");

	for (i = 0; i < count; i++) {
		printf("| %10d | %10d | %15lf | %15lf | %5lf |\n", results_memcmp[i].desc->count, results_memcmp[i].desc->array_size,
		       results_memcmp[i].mean_old, results_memcmp[i].mean_new, results_memcmp[i].mean_old / results_memcmp[i].mean_new);
	}

	free(results_memchr);
	free(results_memcmp);
	return 0;
}
