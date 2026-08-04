/*==================================================*
 * @file      task3_Rev_0_1.c
 * @brief     openシステムコールのエラー確認
 * @version   0.1
 * @date      2026.07.14
 * @author    KAIQUN LUO
 *==================================================*/
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "type.h"

/*==================================================*
 * ファイル情報
 *==================================================*/
#define TARGET_FILE_PATH  ("./111/111.c")   /* 存在しないファイルのパス */
 
/*==================================================*
 * システムコール戻り値 
 *==================================================*/
#define OPEN_ERROR        (-1)              /* openシステムコール失敗時の戻り値 */

/*==================================================*
 * @brief   存在しないファイルをopenし、エラー情報を出力する
 * @param   なし
 * @retval  int  0 openエラーを正常に確認した場合
 *               1 openエラーを発生させられなかった場合
 *==================================================*/
int main(void)
{
    S4 s4_fileDescriptor;

    s4_fileDescriptor = open(TARGET_FILE_PATH, O_RDONLY);

    if (s4_fileDescriptor == OPEN_ERROR) {
        perror("open");
        printf("errno = %d\n", errno);
        return EXIT_FAILURE;
    } else {
        (VD)close(s4_fileDescriptor);
        printf("ファイルが存在するため、openエラーを発生させることができませんでした。\n");
    }
    return EXIT_SUCCESS;
}