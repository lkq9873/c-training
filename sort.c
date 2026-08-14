/*
 * sort_demo.c -- 经典排序算法合集（纯 C11，无外部依赖）
 *
 * 编译: gcc -std=c11 -Wall -Wextra -O2 -o sort_demo sort_demo.c
 * 运行: ./sort_demo
 *
 * 所有排序函数统一签名 void f(int *a, size_t n)，对 a[0..n-1] 原地升序排序。
 * 需要额外内存的算法（归并 / 计数 / 基数）在分配失败时回退到堆排序。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

static void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

/* ------------------------------------------------------------------ *
 * 1. 冒泡排序  O(n^2) / 稳定 / 原地
 *    带 swapped 标志，已有序时可提前退出（最好情况 O(n)）
 * ------------------------------------------------------------------ */
void bubble_sort(int *a, size_t n)
{
    for (size_t i = 0; i + 1 < n; i++) {
        int swapped = 0;
        for (size_t j = 0; j + 1 < n - i; j++) {
            if (a[j] > a[j + 1]) {
                swap(&a[j], &a[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}

/* ------------------------------------------------------------------ *
 * 2. 选择排序  O(n^2) / 不稳定 / 原地
 *    交换次数最少（至多 n-1 次），比较次数恒为 n(n-1)/2
 * ------------------------------------------------------------------ */
void selection_sort(int *a, size_t n)
{
    for (size_t i = 0; i + 1 < n; i++) {
        size_t min = i;
        for (size_t j = i + 1; j < n; j++)
            if (a[j] < a[min])
                min = j;
        if (min != i)
            swap(&a[i], &a[min]);
    }
}

/* ------------------------------------------------------------------ *
 * 3. 插入排序  O(n^2) / 稳定 / 原地
 *    小规模和"基本有序"数据上非常快，常被用作其他算法的收尾
 * ------------------------------------------------------------------ */
void insertion_sort(int *a, size_t n)
{
    for (size_t i = 1; i < n; i++) {
        int key = a[i];
        size_t j = i;
        while (j > 0 && a[j - 1] > key) {
            a[j] = a[j - 1];
            j--;
        }
        a[j] = key;
    }
}

/* ------------------------------------------------------------------ *
 * 4. 希尔排序  约 O(n^1.3) / 不稳定 / 原地
 *    按 gap 分组做插入排序，gap 逐步缩小到 1
 * ------------------------------------------------------------------ */
void shell_sort(int *a, size_t n)
{
    for (size_t gap = n / 2; gap > 0; gap /= 2) {
        for (size_t i = gap; i < n; i++) {
            int key = a[i];
            size_t j = i;
            while (j >= gap && a[j - gap] > key) {
                a[j] = a[j - gap];
                j -= gap;
            }
            a[j] = key;
        }
    }
}

/* ------------------------------------------------------------------ *
 * 5. 归并排序  O(n log n) / 稳定 / 需要 O(n) 辅助空间
 *    区间约定为半开 [lo, hi)
 * ------------------------------------------------------------------ */
static void merge_run(int *a, int *buf, size_t lo, size_t mid, size_t hi)
{
    size_t i = lo, j = mid, k = lo;

    while (i < mid && j < hi)
        buf[k++] = (a[j] < a[i]) ? a[j++] : a[i++];   /* < 而非 <=，保证稳定 */
    while (i < mid)
        buf[k++] = a[i++];
    while (j < hi)
        buf[k++] = a[j++];

    memcpy(a + lo, buf + lo, (hi - lo) * sizeof *a);
}

static void merge_rec(int *a, int *buf, size_t lo, size_t hi)
{
    if (hi - lo < 2)
        return;

    size_t mid = lo + (hi - lo) / 2;
    merge_rec(a, buf, lo, mid);
    merge_rec(a, buf, mid, hi);

    if (a[mid - 1] <= a[mid])       /* 两段已天然衔接，省掉一次归并 */
        return;
    merge_run(a, buf, lo, mid, hi);
}

void heap_sort(int *a, size_t n);   /* 前置声明，供分配失败时回退 */

void merge_sort(int *a, size_t n)
{
    if (n < 2)
        return;

    int *buf = malloc(n * sizeof *buf);
    if (buf == NULL) {
        heap_sort(a, n);            /* 内存不够就退化成原地排序 */
        return;
    }
    merge_rec(a, buf, 0, n);
    free(buf);
}

/* ------------------------------------------------------------------ *
 * 6. 快速排序  平均 O(n log n) / 不稳定 / 原地
 *    三数取中选 pivot + Hoare 双向划分 + 小区间插入排序
 *    只对较小的一侧递归，另一侧尾迭代 => 栈深度 O(log n)
 * ------------------------------------------------------------------ */
#define QSORT_CUTOFF 16

static void quick_rec(int *a, long lo, long hi)     /* 闭区间 [lo, hi] */
{
    while (lo < hi) {
        if (hi - lo < QSORT_CUTOFF) {
            insertion_sort(a + lo, (size_t)(hi - lo + 1));
            return;
        }

        long mid = lo + (hi - lo) / 2;
        if (a[mid] < a[lo])  swap(&a[mid], &a[lo]);
        if (a[hi]  < a[lo])  swap(&a[hi],  &a[lo]);
        if (a[hi]  < a[mid]) swap(&a[hi],  &a[mid]);
        swap(&a[mid], &a[lo]);              /* 中位数放到 lo 作为 pivot */

        int pivot = a[lo];
        long i = lo - 1, j = hi + 1;
        for (;;) {
            do { i++; } while (a[i] < pivot);
            do { j--; } while (a[j] > pivot);
            if (i >= j)
                break;
            swap(&a[i], &a[j]);
        }

        if (j - lo < hi - j) {              /* 递归短的一边 */
            quick_rec(a, lo, j);
            lo = j + 1;
        } else {
            quick_rec(a, j + 1, hi);
            hi = j;
        }
    }
}

void quick_sort(int *a, size_t n)
{
    if (n > 1)
        quick_rec(a, 0, (long)n - 1);
}

/* ------------------------------------------------------------------ *
 * 7. 堆排序  O(n log n) / 不稳定 / 原地
 *    先建大顶堆，再反复把堆顶换到末尾并下沉
 * ------------------------------------------------------------------ */
static void sift_down(int *a, size_t root, size_t n)
{
    for (;;) {
        size_t child = 2 * root + 1;
        if (child >= n)
            break;
        if (child + 1 < n && a[child] < a[child + 1])
            child++;
        if (a[root] >= a[child])
            break;
        swap(&a[root], &a[child]);
        root = child;
    }
}

void heap_sort(int *a, size_t n)
{
    if (n < 2)
        return;

    for (size_t i = n / 2; i-- > 0; )       /* i 从 n/2-1 递减到 0 */
        sift_down(a, i, n);

    for (size_t end = n - 1; end > 0; end--) {
        swap(&a[0], &a[end]);
        sift_down(a, 0, end);
    }
}

/* ------------------------------------------------------------------ *
 * 8. 计数排序  O(n + k) / 稳定 / 非比较排序
 *    仅适合取值范围 k 不大的整数；范围过大或分配失败则回退
 * ------------------------------------------------------------------ */
#define COUNT_MAX_RANGE (1u << 22)          /* 约 400 万个桶的上限 */

void counting_sort(int *a, size_t n)
{
    if (n < 2)
        return;

    int min = a[0], max = a[0];
    for (size_t i = 1; i < n; i++) {
        if (a[i] < min) min = a[i];
        if (a[i] > max) max = a[i];
    }

    unsigned long range = (unsigned long)(max - min) + 1;
    if (range > COUNT_MAX_RANGE) {
        quick_sort(a, n);
        return;
    }

    size_t *cnt = calloc(range, sizeof *cnt);
    if (cnt == NULL) {
        heap_sort(a, n);
        return;
    }

    for (size_t i = 0; i < n; i++)
        cnt[a[i] - min]++;

    size_t k = 0;
    for (unsigned long v = 0; v < range; v++)
        while (cnt[v]-- > 0)
            a[k++] = (int)v + min;

    free(cnt);
}

/* ------------------------------------------------------------------ *
 * 9. 基数排序（LSD，每轮 8 位，共 4 轮）O(4n) / 稳定 / 非比较排序
 *    有符号数先异或最高位映射成无符号数，排完再映射回去
 * ------------------------------------------------------------------ */
void radix_sort(int *a, size_t n)
{
    if (n < 2)
        return;

    unsigned *src = malloc(n * sizeof *src);
    unsigned *dst = malloc(n * sizeof *dst);
    if (src == NULL || dst == NULL) {
        free(src);
        free(dst);
        heap_sort(a, n);
        return;
    }

    for (size_t i = 0; i < n; i++)
        src[i] = (unsigned)a[i] ^ 0x80000000u;      /* 翻转符号位 */

    for (int shift = 0; shift < 32; shift += 8) {
        size_t cnt[256] = {0};
        for (size_t i = 0; i < n; i++)
            cnt[(src[i] >> shift) & 0xFF]++;

        size_t sum = 0;
        for (int v = 0; v < 256; v++) {             /* 前缀和 -> 起始下标 */
            size_t c = cnt[v];
            cnt[v] = sum;
            sum += c;
        }
        for (size_t i = 0; i < n; i++)
            dst[cnt[(src[i] >> shift) & 0xFF]++] = src[i];

        unsigned *tmp = src;
        src = dst;
        dst = tmp;
    }

    for (size_t i = 0; i < n; i++)
        a[i] = (int)(src[i] ^ 0x80000000u);

    free(src);
    free(dst);
}

/* ================================================================== *
 *                            测试代码
 * ================================================================== */

typedef void (*sort_fn)(int *, size_t);

struct sort_entry {
    const char *name;
    sort_fn     fn;
    int         quadratic;      /* 1 表示 O(n^2)，大数据量时跳过 */
};

static const struct sort_entry SORTS[] = {
    { "bubble",    bubble_sort,    1 },
    { "selection", selection_sort, 1 },
    { "insertion", insertion_sort, 1 },
    { "shell",     shell_sort,     0 },
    { "merge",     merge_sort,     0 },
    { "quick",     quick_sort,     0 },
    { "heap",      heap_sort,      0 },
    { "counting",  counting_sort,  0 },
    { "radix",     radix_sort,     0 },
};
#define NSORTS (sizeof SORTS / sizeof SORTS[0])

static int cmp_int(const void *x, const void *y)
{
    int a = *(const int *)x, b = *(const int *)y;
    return (a > b) - (a < b);
}

static int is_sorted(const int *a, size_t n)
{
    for (size_t i = 1; i < n; i++)
        if (a[i - 1] > a[i])
            return 0;
    return 1;
}

static void fill_random(int *a, size_t n)
{
    for (size_t i = 0; i < n; i++)
        a[i] = rand() % 20000 - 10000;      /* 含负数，顺便验证基数排序 */
}

static void print_array(const char *tag, const int *a, size_t n)
{
    printf("%-10s", tag);
    for (size_t i = 0; i < n; i++)
        printf(" %d", a[i]);
    putchar('\n');
}

int main(void)
{
    srand((unsigned)time(NULL));

    /* ---- 小数组：直观看一下排序效果 ---- */
    int demo[] = { 5, -3, 42, 0, 17, 8, -25, 8, 1, 99, 4, -7 };
    size_t dn = sizeof demo / sizeof demo[0];
    int work[sizeof demo / sizeof demo[0]];

    puts("=== 小数组演示 ===");
    print_array("original", demo, dn);
    for (size_t s = 0; s < NSORTS; s++) {
        memcpy(work, demo, sizeof demo);
        SORTS[s].fn(work, dn);
        print_array(SORTS[s].name, work, dn);
    }

    /* ---- 大数组：正确性 + 耗时 ---- */
    const size_t N = 200000;
    int *base = malloc(N * sizeof *base);
    int *ref  = malloc(N * sizeof *ref);
    int *buf  = malloc(N * sizeof *buf);
    if (base == NULL || ref == NULL || buf == NULL) {
        fputs("内存分配失败\n", stderr);
        free(base); free(ref); free(buf);
        return EXIT_FAILURE;
    }

    fill_random(base, N);
    memcpy(ref, base, N * sizeof *ref);
    qsort(ref, N, sizeof *ref, cmp_int);    /* 标准库结果作为对照 */

    printf("\n=== 随机数组 n = %zu ===\n", N);
    printf("%-10s %10s  %s\n", "算法", "耗时(ms)", "结果");
    for (size_t s = 0; s < NSORTS; s++) {
        size_t n = SORTS[s].quadratic ? 5000 : N;   /* O(n^2) 只跑小规模 */

        memcpy(buf, base, n * sizeof *buf);
        clock_t t0 = clock();
        SORTS[s].fn(buf, n);
        double ms = (double)(clock() - t0) * 1000.0 / CLOCKS_PER_SEC;

        int ok = is_sorted(buf, n);
        if (ok && n == N)
            ok = memcmp(buf, ref, N * sizeof *buf) == 0;

        printf("%-10s %10.2f  %s%s\n", SORTS[s].name, ms,
               ok ? "OK" : "FAILED",
               SORTS[s].quadratic ? "  (n=5000)" : "");
    }

    free(base);
    free(ref);
    free(buf);
    return EXIT_SUCCESS;
}
