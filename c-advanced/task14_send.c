/*==================================================*
 * @file      task14_send_Rev_0_1.c
 * @brief     名前つきパイプを使用したデータ送信
 * @version   0.1
 * @date      2026.08.02
 * @author    KAIQUN LUO
 *==================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "type.h"

/*==================================================*
 * 設計方針
 *==================================================*
 * 【エラー発生時の方針】
 * 前提が成立せず処理を続行できない場合（openの失敗後のwrite）は打ち切り、
 * 後処理（close）は必ず実行する。
 *
 * 【文字列の終端】
 * 受信側が終端を確認できるよう、終端のヌル文字を含めて送信する。
 *==================================================*/

/*==================================================*
 * 名前つきパイプ情報
 *==================================================*/
#define FIFO_PATH           ("./task14_fifo") /* 名前つきパイプのパス */

/*==================================================*
 * 送信文字列
 *==================================================*/
#define SEND_MESSAGE        ("Hello, I'm the sender.")   /* 名前つきパイプへ書き込む文字列 */
#define SEND_MESSAGE_BYTE   (sizeof(SEND_MESSAGE))   /* 上記の送信バイト数（終端のヌル文字を含む） */

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR   (-1)    /* エラー発生時の戻り値 */

/*==================================================*
 * @brief   名前つきパイプをオープンし、文字列を送信した後、
 *          名前つきパイプをクローズする
 * @param   なし
 * @retval  int  EXIT_SUCCESS  文字列の送信に成功した場合
 *               EXIT_FAILURE  システムコールでエラーが発生した場合
 * @note    データ受信側のプログラムを起動した後に本プログラムを起動すること。
 *          名前つきパイプはデータ受信側のプログラムが作成する。
 *==================================================*/
int main(void)
{
    S4 s4_fileDescriptor;
    S4 s4_writeResult;
    S4 s4_closeResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    s4_fileDescriptor = open(FIFO_PATH, O_WRONLY);

    if (s4_fileDescriptor == SYSTEM_CALL_ERROR) {
        perror("open");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        s4_writeResult = write(s4_fileDescriptor, SEND_MESSAGE, SEND_MESSAGE_BYTE);

        if (s4_writeResult == SYSTEM_CALL_ERROR) {
            perror("write");

            s4_exitStatus = EXIT_FAILURE;
        } else {
            /* DO NOTHING */
        }

        s4_closeResult = close(s4_fileDescriptor);

        if (s4_closeResult == SYSTEM_CALL_ERROR) {
            perror("close");

            s4_exitStatus = EXIT_FAILURE;
        } else {
            /* DO NOTHING */
        }
    }

    return s4_exitStatus;
}
