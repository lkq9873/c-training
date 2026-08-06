/*==================================================*
 * @file      task14_send_Rev_0_2.c
 * @brief     名前つきパイプを使用したデータ送信
 * @version   0.2
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
 * エラーを検出した場合は、それまでに獲得した資源の後始末を行ってから
 * その場でEXIT_FAILUREを返し、以降の処理は行わない。
 *
 * 【複数個のreturnについて】
 * 深い分岐のnestを避けるため、main関数内で複数個のreturnを使用する。
 * 規約「複数個のreturnを使用した方が可読性が高まる場合に限り許可する」
 * に基づく。フロー図にも複数個のreturnを明記している。
 *
 * 【文字列の終端】
 * 終端のヌル文字は送信せず、受信側でバッファに付与する。
 * このため送信バイト数はsizeofから1を減じた値とする。
 *==================================================*/

/*==================================================*
 * 名前つきパイプ情報
 *==================================================*/
#define FIFO_PATH           ("./task14_fifo") /* 名前つきパイプのパス */

/*==================================================*
 * 送信文字列
 *==================================================*/
#define SEND_MESSAGE        ("Hello, I'm the sender.")   /* 名前つきパイプへ書き込む文字列 */
#define SEND_MESSAGE_BYTE   (sizeof(SEND_MESSAGE) - 1)   /* 上記の送信バイト数（終端のヌル文字を含まない） */

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
    S4 s4_callResult;

    s4_fileDescriptor = open(FIFO_PATH, O_WRONLY);

    if (s4_fileDescriptor == SYSTEM_CALL_ERROR) {
        perror("open");

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = write(s4_fileDescriptor, SEND_MESSAGE, SEND_MESSAGE_BYTE);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("write");

        /* オープン済みの名前つきパイプをクローズしてから戻る。 */
        (VD)close(s4_fileDescriptor);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = close(s4_fileDescriptor);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("close");

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    return EXIT_SUCCESS;
}
