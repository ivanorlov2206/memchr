#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

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

void test_memcmp(const struct test_description *desc, struct test_result *result, bool aligned)
{
	unsigned char *buffer1 = calloc(desc->array_size, 1);
	unsigned char *buffer2 = calloc(desc->array_size, 1);

	clock_t start, end, start2, end2;
	int i, res1, res2;
	double mean_new = 0, mean_old = 0;

	result->desc = desc;

	for (i = 0; i < desc->count; i++) {
		int num = desc->array_size - 1;
		buffer1[num] = rand() % 2;
		buffer2[num] = rand() % 2;
		int st = i % 8;
		int st2 = (i + 1) % 8;
		if (aligned)
			st2 = i % 8;
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

void test_memchr(const struct test_description *desc, struct test_result *result, bool aligned)
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
	};

void print_test_results(struct test_result *results, int count, bool aligned, char *test_title)
{
	int i;

	printf("%s (%s)\n", test_title, aligned ? "aligned" : "misaligned");
	printf("---------------\n");
	printf("| test_count | array_size | old_func_cycles | new_func_cycles |     K    |\n");
	printf("--------------------------------------------------------------------------\n");
	for (i = 0; i < count; i++) {
		printf("| %10d | %10d | %15lf | %15lf | %5lf |\n", results[i].desc->count, results[i].desc->array_size,
		       results[i].mean_old, results[i].mean_new, results[i].mean_old / results[i].mean_new);
	}
}

int main(int argc, char *argv[])
{
	int count = sizeof(tests) / sizeof(tests[0]);
	struct test_result *results = calloc(sizeof(struct test_result), count);
	int i;

	srand(time(NULL));

	for (i = 0; i < count; i++) {
		printf("Running memchr(misaligned) %d/%d: count = %d, array size = %d\n", (i + 1), count, tests[i].count, tests[i].array_size);
		test_memchr(&tests[i], &results[i], 0);
	}

	print_test_results(results, count, 0, "memchr");

	for (i = 0; i < count; i++) {
		printf("Running memcmp(misaligned) %d/%d: count = %d, array size = %d\n", (i + 1), count, tests[i].count, tests[i].array_size);
		test_memcmp(&tests[i], &results[i], 0);
	}
	print_test_results(results, count, 0, "memcmp");

	for (i = 0; i < count; i++) {
		printf("Running memcmp(aligned) %d/%d: count = %d, array size = %d\n", (i + 1), count, tests[i].count, tests[i].array_size);
		test_memcmp(&tests[i], &results[i], 1);
	}
	print_test_results(results, count, 1, "memcmp");
	free(results);

	return 0;
}
