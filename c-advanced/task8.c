/*==================================================*
 * @file      task8_Rev_0_1.c
 * @brief     readシステムコールによるファイル読み出し
 * @version   0.1
 * @date      2026.07.16
 * @author    KAIQUN LUO
 *==================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "type.h"

/*==================================================*
 * ファイル情報
 *==================================================*/
#define TARGET_FILE_PATH   ("./tmp.txt")    /* ファイルのパス */
#define READ_BUFFER_SIZE   (100)           /* 読み出しデータを格納する配列のサイズ */

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR  (-1)             /* システムコールでエラーが発生した場合の戻り値 */

/*==================================================*
 * @brief   ファイルをオープンし、readシステムコールを
 *          使用して文字列を読み出す
 * @param   なし
 * @retval  int  EXIT_SUCCESS 読み出しに成功した場合
 *               EXIT_FAILURE システムコールでエラーが発生した場合
 *==================================================*/
int main(void)
{
    char ch_readBuffer[READ_BUFFER_SIZE];
    S4 s4_fileDescriptor;
    S4 s4_readResult;
    S4 s4_closeResult;

    s4_fileDescriptor = open(TARGET_FILE_PATH, O_RDONLY);

    if (s4_fileDescriptor == SYSTEM_CALL_ERROR) {
        perror("open");
        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_readResult = read(s4_fileDescriptor, ch_readBuffer, sizeof(ch_readBuffer) - 1);

    if (s4_readResult == SYSTEM_CALL_ERROR) {
        perror("read");
        (VD)close(s4_fileDescriptor);
        return EXIT_FAILURE;
    } else if (s4_readResult == 0) {
        printf("ファイルに読み出すデータがありません\n");
    } else {
        ch_readBuffer[s4_readResult] = '\0';
        printf("読み出した文字列：%s\n", ch_readBuffer);
    }

    s4_closeResult = close(s4_fileDescriptor);

    if (s4_closeResult == SYSTEM_CALL_ERROR) {
        perror("close");
        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    return EXIT_SUCCESS;
}