/*
 * cal.c - 指定した年のカレンダーを表示する
 *
 * usage: ./cal 2001
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DAYS_IN_WEEK    7
#define WEEKS_IN_MONTH  6       /* 1ヶ月は最大6週にまたがる */
#define MONTHS_PER_ROW  3       /* 横に並べる月数 */
#define CELL_WIDTH      3       /* "dd " の幅 */
#define MONTH_WIDTH     (DAYS_IN_WEEK * CELL_WIDTH)      /* 21 */
#define MONTH_GAP       2
#define YEAR_MIN        1583    /* グレゴリオ暦が有効な範囲 */
#define YEAR_MAX        9999

/* 各月の日数（2月は閏年で補正） */
static const int days_tbl[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int is_leap(int year)
{
    return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

static int last_day(int year, int month)
{
    if (month == 2 && is_leap(year))
        return 29;
    return days_tbl[month - 1];
}

/*
 * 指定日の曜日を返す（0=日曜 ... 6=土曜）
 * 1月・2月は前年の13月・14月として扱う
 */
static int day_of_week(int year, int month, int day)
{
    if (month < 3) {
        year--;
        month += 12;
    }
    return (year + year / 4 - year / 100 + year / 400
            + (13 * month + 8) / 5 + day) % 7;
}

/* n 個のスペースを出力する */
static void pad(int n)
{
    printf("%*s", n, "");
}

/*
 * カレンダー配列を作る
 * cal[月][週][曜日] = 日付、空きは 0
 */
static void build(int year, int cal[12][WEEKS_IN_MONTH][DAYS_IN_WEEK])
{
    int month, day, row, col, last;

    memset(cal, 0, sizeof(int) * 12 * WEEKS_IN_MONTH * DAYS_IN_WEEK);

    for (month = 1; month <= 12; month++) {
        col  = day_of_week(year, month, 1);
        last = last_day(year, month);
        row  = 0;

        for (day = 1; day <= last; day++) {
            cal[month - 1][row][col] = day;
            if (++col >= DAYS_IN_WEEK) {
                col = 0;
                row++;
            }
        }
    }
}

/* 3ヶ月ずつ横に並べて出力する */
static void print_cal(int year, int cal[12][WEEKS_IN_MONTH][DAYS_IN_WEEK])
{
    static const char *wday[] = { "日", "月", "火", "水", "木", "金", "土" };
    int total_width = MONTH_WIDTH * MONTHS_PER_ROW
                    + MONTH_GAP * (MONTHS_PER_ROW - 1);
    int band, row, i, col, month, day;

    /* 年の見出しを中央に */
    pad((total_width - 6) / 2);
    printf("%d年\n\n", year);

    for (band = 0; band < 12 / MONTHS_PER_ROW; band++) {

        /* 月の見出し（"12月" は表示幅4） */
        for (i = 0; i < MONTHS_PER_ROW; i++) {
            month = band * MONTHS_PER_ROW + i + 1;
            pad((MONTH_WIDTH - 4) / 2);
            printf("%2d月", month);
            pad(MONTH_WIDTH - 4 - (MONTH_WIDTH - 4) / 2);
            if (i < MONTHS_PER_ROW - 1)
                pad(MONTH_GAP);
        }
        putchar('\n');

        /* 曜日の見出し */
        for (i = 0; i < MONTHS_PER_ROW; i++) {
            for (col = 0; col < DAYS_IN_WEEK; col++)
                printf("%s ", wday[col]);
            if (i < MONTHS_PER_ROW - 1)
                pad(MONTH_GAP);
        }
        putchar('\n');

        /* 日付を週単位で3ヶ月分ずつ */
        for (row = 0; row < WEEKS_IN_MONTH; row++) {
            for (i = 0; i < MONTHS_PER_ROW; i++) {
                month = band * MONTHS_PER_ROW + i + 1;
                for (col = 0; col < DAYS_IN_WEEK; col++) {
                    day = cal[month - 1][row][col];
                    if (day == 0)
                        pad(CELL_WIDTH);
                    else
                        printf("%2d ", day);
                }
                if (i < MONTHS_PER_ROW - 1)
                    pad(MONTH_GAP);
            }
            putchar('\n');
        }
        putchar('\n');
    }
}

int main(int argc, char *argv[])
{
    int cal[12][WEEKS_IN_MONTH][DAYS_IN_WEEK];
    char *end;
    long year;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <year>\n", argv[0]);
        return 1;
    }

    year = strtol(argv[1], &end, 10);
    if (*end != '\0' || end == argv[1]) {
        fprintf(stderr, "error: '%s' is not a number\n", argv[1]);
        return 1;
    }
    if (year < YEAR_MIN || year > YEAR_MAX) {
        fprintf(stderr, "error: year must be %d - %d\n", YEAR_MIN, YEAR_MAX);
        return 1;
    }

    build((int)year, cal);
    print_cal((int)year, cal);

    return 0;
}
