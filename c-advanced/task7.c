/*==================================================*
 * @file      task7_Rev_0_1.c
 * @brief     writeシステムコールによるファイル書き込み
 * @version   0.1
 * @date      2026.07.16
 * @author    KAIQUN LUO
 *==================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "type.h"

/*==================================================*
 * ファイル情報
 *==================================================*/
#define TARGET_FILE_PATH ("./tmp.txt")                  /* ファイルのパス */
#define WRITE_DATA ("Hello")                            /* ファイルへ書き込む文字列 */
#define WRITE_DATA_BYTE (sizeof(WRITE_DATA) - 1)        /* ファイルへ書き込むデータのバイト数 */
#define FILE_MODE (0660)                                /* ファイルの保護モードをrw-rw----に指定する */
/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR  (-1)                         /* システムコールでエラーが発生した場合の戻り値 */

/*==================================================*
 * @brief   ファイルを作成またはオープンし、writeシステムコールを使用して文字列を書き込む
 * @param   なし
 * @retval  int  EXIT_SUCCESS 書き込みに成功した場合
 *               EXIT_FAILURE システムコールでエラーが発生した場合
 *==================================================*/
int main(void)
{
    S4 s4_fileDescriptor;
    S4 s4_writeResult;
    S4 s4_closeResult;

    s4_fileDescriptor = open(TARGET_FILE_PATH, O_CREAT | O_WRONLY | O_TRUNC, FILE_MODE);

    if (s4_fileDescriptor == SYSTEM_CALL_ERROR) {
        perror("open");
        return EXIT_FAILURE;
    } else {
        /*DO NOTHING*/
    }

    s4_writeResult = write(s4_fileDescriptor, WRITE_DATA, WRITE_DATA_BYTE);

    if (s4_writeResult == SYSTEM_CALL_ERROR) {
        perror("write");
        (VD)close(s4_fileDescriptor);
        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_closeResult = close(s4_fileDescriptor);

    if (s4_closeResult == SYSTEM_CALL_ERROR) {
        perror("close");
        return EXIT_FAILURE;
    } else {
        (VD)printf("文字列を書き込みました。\n");
    }
    return EXIT_SUCCESS;
}