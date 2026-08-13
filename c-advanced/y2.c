/******************************************************************************
 * ファイル名 : calendar.c
 * 概要       : コマンドライン引数で指定された西暦年のカレンダーを画面に表示する
 * 使用方法   : $ ./calendar 2001
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * 型定義（研修共通のtypedefヘッダがある場合は、そちらに差し替えること）
 * ------------------------------------------------------------------------ */
typedef unsigned char   u1;
typedef unsigned short  u2;
typedef unsigned int    u4;
typedef char            s1;
typedef signed int      s4;

#define VD                  (void)

/* --- 戻り値 ------------------------------------------------------------- */
#define RET_OK              (0)
#define RET_NG              (1)
#define CHK_OK              (0)
#define CHK_NG              (1)

/* --- 引数関連 ----------------------------------------------------------- */
#define ARGC_VALID          (2)                     /* プログラム名 + 年     */
#define ARGV_IDX_YEAR       (1)                     /* 年の格納位置          */
#define YEAR_DIGIT_MIN      (1)                     /* 年の最小桁数          */
#define YEAR_DIGIT_MAX      (4)                     /* 年の最大桁数          */
#define DECIMAL_BASE        (10)                    /* 10進数の基数          */
#define CHAR_NUM_MIN        ('0')
#define CHAR_NUM_MAX        ('9')

/* --- 年月日の範囲 ------------------------------------------------------- */
#define YEAR_MIN            (1)
#define YEAR_MAX            (9999)
#define MONTH_MIN           (1)
#define MONTH_MAX           (12)
#define MONTH_MAR           (3)                     /* 3月（曜日算出の基点） */
#define DAY_FIRST           (1)                     /* 各月の1日             */
#define DAY_EMPTY           (0)                     /* 日付が無いセル        */

/* --- 閏年判定 ----------------------------------------------------------- */
#define LEAP_CYCLE_4        (4)
#define LEAP_CYCLE_100      (100)
#define LEAP_CYCLE_400      (400)
#define FEB_DAY_NORMAL      (28)
#define FEB_DAY_LEAP        (FEB_DAY_NORMAL + 1)    /* 閏年の2月は29日       */

/* --- 曜日算出（ツェラー系の公式）用係数 --------------------------------- */
#define ZELLER_MUL_MONTH    (13)
#define ZELLER_ADD_MONTH    (8)
#define ZELLER_DIV_MONTH    (5)

/* --- カレンダーの表示レイアウト ----------------------------------------- */
#define WEEK_DAY_NUM        (7)                                     /* 1週の日数        */
#define WEEK_ROW_MAX        (6)                                     /* 1ヶ月の最大週数  */
#define MONTH_PER_LINE      (3)                                     /* 横に並べる月数   */
#define LINE_MAX            (MONTH_MAX / MONTH_PER_LINE)            /* 段数 = 12 / 3    */
#define CELL_WIDTH          (3)                                     /* "dd " の幅       */
#define MONTH_WIDTH         (WEEK_DAY_NUM * CELL_WIDTH)             /* 1ヶ月分の幅      */
#define MONTH_GAP           (2)                                     /* 月と月の間隔     */
#define CAL_WIDTH           ((MONTH_WIDTH * MONTH_PER_LINE) + (MONTH_GAP * (MONTH_PER_LINE - 1)))

#define MONTH_LABEL_WIDTH   (4)                                     /* "12月" の表示幅  */
#define MONTH_PAD_LEFT      ((MONTH_WIDTH - MONTH_LABEL_WIDTH) / 2)
#define MONTH_PAD_RIGHT     (MONTH_WIDTH - MONTH_LABEL_WIDTH - MONTH_PAD_LEFT)

#define YEAR_LABEL_WIDTH    (YEAR_DIGIT_MAX + 2)                    /* "2001年" の表示幅*/
#define YEAR_PAD_LEFT       ((CAL_WIDTH - YEAR_LABEL_WIDTH) / 2)

/* --- 各月の日数（2月は閏年で補正する） ---------------------------------- */
static const u1 gu1LastDayTbl[MONTH_MAX] = {
    31, FEB_DAY_NORMAL, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* --- プロトタイプ宣言 --------------------------------------------------- */
static u1 U1ChkYearStr(const s1 *ps1Str, u2 *pu2Year);
static u1 U1IsLeapYear(u2 u2Year);
static u1 U1GetLastDay(u2 u2Year, u1 u1Month);
static u1 U1CalcDayOfWeek(u2 u2Year, u1 u1Month, u1 u1Day);
static void VDMakeCalendar(u2 u2Year, u1 u1Cal[MONTH_MAX][WEEK_ROW_MAX][WEEK_DAY_NUM]);
static void VDPrintCalendar(u2 u2Year, const u1 u1Cal[MONTH_MAX][WEEK_ROW_MAX][WEEK_DAY_NUM]);

/******************************************************************************
 * 関数名 : main
 * 概要   : コマンドライン引数の年を検査し、カレンダーを生成して画面に表示する
 * 引数   : s4Argc   引数の個数
 *          ps1Argv  引数の文字列配列
 * 戻り値 : RET_OK / RET_NG
 ******************************************************************************/
s4 main(s4 s4Argc, s1 *ps1Argv[])
{
    u1 u1Cal[MONTH_MAX][WEEK_ROW_MAX][WEEK_DAY_NUM];
    u2 u2Year;
    s4 s4Ret;

    s4Ret   = RET_NG;
    u2Year  = 0U;

    /* ※1 引数の個数が ARGC_VALID 以外の場合は使用方法を表示して終了する    */
    if (s4Argc != ARGC_VALID) {
        VD printf("usage : %s <year(%d-%d)>\n", ps1Argv[0], YEAR_MIN, YEAR_MAX);
        /* 戻り値のエラーチェック不要。理由は設計を参照 */
    }
    /* ※2 年の文字列が数字のみ、かつ YEAR_MIN〜YEAR_MAX の範囲かを検査する  */
    else if (U1ChkYearStr((const s1 *)ps1Argv[ARGV_IDX_YEAR], &u2Year) != CHK_OK) {
        VD printf("error : year must be %d - %d\n", YEAR_MIN, YEAR_MAX);
        /* 戻り値のエラーチェック不要。理由は設計を参照 */
    }
    else {
        /* ここに到達した時点で u2Year は YEAR_MIN〜YEAR_MAX に収まっている  */
        VDMakeCalendar(u2Year, u1Cal);
        VDPrintCalendar(u2Year, (const u1 (*)[WEEK_ROW_MAX][WEEK_DAY_NUM])u1Cal);
        s4Ret = RET_OK;
    }

    return s4Ret;
}

/******************************************************************************
 * 関数名 : U1ChkYearStr
 * 概要   : 年の文字列が数字のみで構成され、かつ有効範囲内かを検査し、
 *          正常な場合のみ数値へ変換して pu2Year に格納する
 * 引数   : ps1Str   年を表す文字列（NULL終端）
 *          pu2Year  変換後の年の格納先
 * 戻り値 : CHK_OK / CHK_NG
 * 備考   : 引数のNULLチェックは呼び出し元(main)で確認済みのため実施しない
 ******************************************************************************/
static u1 U1ChkYearStr(const s1 *ps1Str, u2 *pu2Year)
{
    u4 u4Len;
    u4 u4Idx;
    u4 u4Val;
    u1 u1Ret;

    u1Ret = CHK_NG;
    u4Val = 0U;
    u4Len = (u4)strlen(ps1Str);

    if ((u4Len >= (u4)YEAR_DIGIT_MIN) && (u4Len <= (u4)YEAR_DIGIT_MAX)) {
        u1Ret = CHK_OK;
        for (u4Idx = 0U; u4Idx < u4Len; u4Idx++) {
            if ((ps1Str[u4Idx] < CHAR_NUM_MIN) || (ps1Str[u4Idx] > CHAR_NUM_MAX)) {
                u1Ret = CHK_NG;
                break;
            }
            u4Val = (u4Val * (u4)DECIMAL_BASE) + (u4)(ps1Str[u4Idx] - CHAR_NUM_MIN);
        }
    }

    if (u1Ret == CHK_OK) {
        if ((u4Val < (u4)YEAR_MIN) || (u4Val > (u4)YEAR_MAX)) {
            u1Ret = CHK_NG;
        }
        else {
            *pu2Year = (u2)u4Val;
        }
    }

    return u1Ret;
}

/******************************************************************************
 * 関数名 : U1IsLeapYear
 * 概要   : 指定された年が閏年かどうかを判定する
 * 引数   : u2Year  西暦年
 * 戻り値 : 1:閏年 / 0:平年
 * 備考   : 引数の範囲チェックは呼び出し元で確認済みのため実施しない
 ******************************************************************************/
static u1 U1IsLeapYear(u2 u2Year)
{
    u1 u1Ret;

    u1Ret = 0U;

    if (((u2Year % LEAP_CYCLE_4) == 0U) && ((u2Year % LEAP_CYCLE_100) != 0U)) {
        u1Ret = 1U;
    }
    else if ((u2Year % LEAP_CYCLE_400) == 0U) {
        u1Ret = 1U;
    }
    else {
        /* 平年のため何もしない */
    }

    return u1Ret;
}

/******************************************************************************
 * 関数名 : U1GetLastDay
 * 概要   : 指定された年月の末日を、月別日数テーブルと閏年判定から求める
 * 引数   : u2Year   西暦年
 *          u1Month  月(1-12)
 * 戻り値 : 末日(28-31)
 * 備考   : 引数の範囲チェックは呼び出し元で確認済みのため実施しない
 ******************************************************************************/
static u1 U1GetLastDay(u2 u2Year, u1 u1Month)
{
    u1 u1LastDay;

    u1LastDay = gu1LastDayTbl[u1Month - MONTH_MIN];

    if ((u1Month == 2U) && (U1IsLeapYear(u2Year) != 0U)) {
        u1LastDay = FEB_DAY_LEAP;
    }

    return u1LastDay;
}

/******************************************************************************
 * 関数名 : U1CalcDayOfWeek
 * 概要   : 指定された年月日の曜日を算出する（"2-1 曜日算出"で作成した処理）
 *          1月・2月は前年の13月・14月として扱い、公式に代入する
 * 引数   : u2Year   西暦年
 *          u1Month  月(1-12)
 *          u1Day    日(1-31)
 * 戻り値 : 0:日 1:月 2:火 3:水 4:木 5:金 6:土
 * 備考   : 引数の範囲チェックは呼び出し元で確認済みのため実施しない
 ******************************************************************************/
static u1 U1CalcDayOfWeek(u2 u2Year, u1 u1Month, u1 u1Day)
{
    u4 u4CalcYear;
    u4 u4CalcMonth;
    u1 u1Week;

    u4CalcYear  = (u4)u2Year;
    u4CalcMonth = (u4)u1Month;

    if (u4CalcMonth < (u4)MONTH_MAR) {
        u4CalcYear--;
        u4CalcMonth += (u4)MONTH_MAX;
    }

    u1Week = (u1)((u4CalcYear
                 + (u4CalcYear / LEAP_CYCLE_4)
                 - (u4CalcYear / LEAP_CYCLE_100)
                 + (u4CalcYear / LEAP_CYCLE_400)
                 + (((ZELLER_MUL_MONTH * u4CalcMonth) + ZELLER_ADD_MONTH) / ZELLER_DIV_MONTH)
                 + (u4)u1Day) % (u4)WEEK_DAY_NUM);

    return u1Week;
}

/******************************************************************************
 * 関数名 : VDMakeCalendar
 * 概要   : 1年分のカレンダー配列を作成する。各月1日の曜日を開始位置として
 *          末日まで日付を格納し、日付が無いセルには DAY_EMPTY を設定する
 * 引数   : u2Year  西暦年
 *          u1Cal   カレンダー格納配列 [月][週][曜日]
 * 戻り値 : なし
 * 備考   : 引数の範囲チェックは呼び出し元で確認済みのため実施しない
 ******************************************************************************/
static void VDMakeCalendar(u2 u2Year, u1 u1Cal[MONTH_MAX][WEEK_ROW_MAX][WEEK_DAY_NUM])
{
    u1 u1Month;
    u1 u1Day;
    u1 u1LastDay;
    u1 u1Row;
    u1 u1Col;

    VD memset(u1Cal, DAY_EMPTY, (size_t)(MONTH_MAX * WEEK_ROW_MAX * WEEK_DAY_NUM));
    /* 戻り値のエラーチェック不要。理由は設計を参照 */

    for (u1Month = MONTH_MIN; u1Month <= MONTH_MAX; u1Month++) {
        u1Col     = U1CalcDayOfWeek(u2Year, u1Month, DAY_FIRST);
        u1LastDay = U1GetLastDay(u2Year, u1Month);
        u1Row     = 0U;

        for (u1Day = DAY_FIRST; u1Day <= u1LastDay; u1Day++) {
            u1Cal[u1Month - MONTH_MIN][u1Row][u1Col] = u1Day;
            u1Col++;
            if (u1Col >= (u1)WEEK_DAY_NUM) {
                u1Col = 0U;
                u1Row++;
            }
        }
    }
}

/******************************************************************************
 * 関数名 : VDPrintCalendar
 * 概要   : 年の見出しを中央に表示し、1段あたり MONTH_PER_LINE ヶ月を
 *          横に並べて1年分のカレンダーを画面に表示する
 * 引数   : u2Year  西暦年
 *          u1Cal   カレンダー格納配列 [月][週][曜日]
 * 戻り値 : なし
 * 備考   : 引数の範囲チェックは呼び出し元で確認済みのため実施しない
 ******************************************************************************/
static void VDPrintCalendar(u2 u2Year, const u1 u1Cal[MONTH_MAX][WEEK_ROW_MAX][WEEK_DAY_NUM])
{
    u1 u1Line;
    u1 u1Row;
    u1 u1Pos;
    u1 u1Col;
    u1 u1Month;
    u1 u1Day;

    /* 以降のprintfは表示のみのため、戻り値のエラーチェック不要。理由は設計を参照 */

    /* 年の見出し（カレンダー全体幅の中央に表示する） */
    VD printf("%*s%*u年\n\n", YEAR_PAD_LEFT, "", YEAR_DIGIT_MAX, u2Year);

    for (u1Line = 0U; u1Line < (u1)LINE_MAX; u1Line++) {

        /* 月の見出し */
        for (u1Pos = 0U; u1Pos < (u1)MONTH_PER_LINE; u1Pos++) {
            u1Month = (u1)((u1Line * MONTH_PER_LINE) + u1Pos + MONTH_MIN);
            VD printf("%*s%2u月%*s", MONTH_PAD_LEFT, "", u1Month, MONTH_PAD_RIGHT, "");
            if (u1Pos < (u1)(MONTH_PER_LINE - 1)) {
                VD printf("%*s", MONTH_GAP, "");
            }
        }
        VD printf("\n");

        /* 日付（週単位で3ヶ月分を横に並べる） */
        for (u1Row = 0U; u1Row < (u1)WEEK_ROW_MAX; u1Row++) {
            for (u1Pos = 0U; u1Pos < (u1)MONTH_PER_LINE; u1Pos++) {
                u1Month = (u1)((u1Line * MONTH_PER_LINE) + u1Pos + MONTH_MIN);

                for (u1Col = 0U; u1Col < (u1)WEEK_DAY_NUM; u1Col++) {
                    u1Day = u1Cal[u1Month - MONTH_MIN][u1Row][u1Col];
                    if (u1Day == (u1)DAY_EMPTY) {
                        VD printf("%*s", CELL_WIDTH, "");
                    }
                    else {
                        VD printf("%2u ", u1Day);
                    }
                }
                if (u1Pos < (u1)(MONTH_PER_LINE - 1)) {
                    VD printf("%*s", MONTH_GAP, "");
                }
            }
            VD printf("\n");
        }
        VD printf("\n");
    }
}
