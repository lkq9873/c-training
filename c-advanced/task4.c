/*==================================================*
 * @file      task5_Rev_0_2.c
 * @brief     fopen関数のエラー確認
 * @version   0.2
 * @date      2026.07.14
 * @author    KAIQUN LUO
 *==================================================*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "type.h"

/*==================================================*
 * ファイル情報
 *==================================================*/
#define TARGET_FILE_PATH  ("./111/111.c")   /* 存在しないファイルのパス */
#define FILE_OPEN_MODE    ("r")             /* ファイルを読み出しでオープンするモード */

/*==================================================*
 * @brief   存在しないファイルをfopenし、エラー情報を出力する
 * @param   なし
 * @retval  int  0 fopenエラーを正常に確認した場合
 *               1 fopenエラーを発生させられなかった場合
 *==================================================*/
int main(void)
{
    FILE *pfile_filePointer;

    pfile_filePointer = fopen(TARGET_FILE_PATH, FILE_OPEN_MODE);

    if (pfile_filePointer == NULL) {
        perror("fopen");
        printf("errno = %d\n", errno);
        return EXIT_FAILURE;
    } else {
        (VD)fclose(pfile_filePointer);
        printf("ファイルが存在するため、fopenエラーを発生させることができませんでした。\n");
    }

    return EXIT_SUCCESS;
}