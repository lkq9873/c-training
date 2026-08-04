/*==================================================*
 * @file      task6_Rev_0_1.c
 * @brief     tmp.txtのファイル記述子表示
 * @version   0.1
 * @date      2026.07.15
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
#define TARGET_FILE_PATH  ("./tmp.txt")  /* オープンするファイルのパス */

/*==================================================*
 * openシステムコール戻り値
 *==================================================*/
#define OPEN_ERROR        (-1)  /* openシステムコール失敗時の戻り値 */

/*==================================================*
 * @brief   tmp.txtをオープンし、ファイル記述子を表示する
 * @param   なし
 * @retval  EXIT_SUCCESS ファイル記述子を表示した場合
 * @retval  EXIT_FAILURE openシステムコールでエラーが発生した場合
 *==================================================*/
int main(void)
{
    S4 s4_fileDescriptor;

    s4_fileDescriptor = open(TARGET_FILE_PATH, O_RDONLY);

    if (s4_fileDescriptor == OPEN_ERROR) {
        perror("open");
        return EXIT_FAILURE;
    } else {
        printf("tmp.txtのファイル記述子は%dである。\n", s4_fileDescriptor);
        (VD)close(s4_fileDescriptor);
    }

    return EXIT_SUCCESS;
}