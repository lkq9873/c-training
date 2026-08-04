/*==================================================*
 * @file      task5_Rev_0_1.c
 * @brief     標準入出力のファイル記述子表示
 * @version   0.1
 * @date      2026.07.15
 * @author    KAIQUN LUO
 *==================================================*/
#include <stdio.h>
#include <stdlib.h>
#include "type.h"

/*==================================================*
 * fileno関数戻り値
 *==================================================*/
#define FILENO_ERROR (-1)   /* fileno関数でエラーが発生した場合の戻り値 */

/*==================================================*
 * @brief   stdin、stdout、stderrのファイル記述子を表示する
 * @param   なし
 * @retval  int  EXIT_SUCCESS  ファイル記述子を表示し、処理を終了した場合
 *               EXIT_FAILURE  fileno関数でエラーが発生した場合
 *==================================================*/
int main(void)
{
    S4 s4_stdinFileDescriptor;
    S4 s4_stdoutFileDescriptor;
    S4 s4_stderrFileDescriptor;

    s4_stdinFileDescriptor = fileno(stdin);

    if (s4_stdinFileDescriptor == FILENO_ERROR) {
        perror("fileno(stdin)");
        return EXIT_FAILURE;
    } else {
        printf("stdinのファイル記述子は%dである。\n", s4_stdinFileDescriptor);
    }

    s4_stdoutFileDescriptor = fileno(stdout);

    if (s4_stdoutFileDescriptor == FILENO_ERROR) {
        perror("fileno(stdout)");
        return EXIT_FAILURE;
    } else {
        printf("stdoutのファイル記述子は%dである。\n", s4_stdoutFileDescriptor);
    }

    s4_stderrFileDescriptor = fileno(stderr);

    if (s4_stderrFileDescriptor == FILENO_ERROR) {
        perror("fileno(stderr)");
        return EXIT_FAILURE;
    } else {
        printf("stderrのファイル記述子は%dである。\n", s4_stderrFileDescriptor);
    }

    return EXIT_SUCCESS;
}