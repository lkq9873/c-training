/*==================================================*
 * @file      task14_receive_Rev_0_1.c
 * @brief     名前つきパイプを使用したデータ受信
 * @version   0.1
 * @date      2026.08.02
 * @author    KAIQUN LUO
 *==================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "type.h"

/*==================================================*
 * 設計方針
 *==================================================*
 * 【エラー発生時の方針】
 * 前提が成立せず処理を続行できない場合（mkfifoの失敗後のopen等）は打ち切り、
 * 後処理（close、unlink）は必ず実行する。
 *
 * 【文字列の終端】
 * 送信側は終端のヌル文字を含めて送信し、受信側は受信データの最終バイトが
 * ヌル文字であることを確認する。これにより、受信データがバッファサイズを
 * 超えて途中で切れた場合を検出することができる。
 *==================================================*/

/*==================================================*
 * 名前つきパイプ情報
 *==================================================*/
#define FIFO_PATH           ("./task14_fifo") /* 名前つきパイプのパス */
#define FIFO_MODE           (0660)            /* 名前つきパイプの保護モードをrw-rw----に指定する */

/*==================================================*
 * バッファサイズ
 *==================================================*/
#define BUFFER_SIZE         (100)   /* 受信文字列を格納する配列のサイズ */

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR   (-1)    /* エラー発生時の戻り値 */

/*==================================================*
 * @brief   名前つきパイプを作成してオープンし、文字列を受信して表示した後、
 *          名前つきパイプをクローズして削除する
 * @param   なし
 * @retval  int  EXIT_SUCCESS  文字列の受信に成功した場合
 *               EXIT_FAILURE  システムコールでエラーが発生した場合
 * @note    本プログラムを起動した後にデータ送信側のプログラムを起動すること。
 *          読み出し専用でオープンした名前つきパイプは、データ送信側が
 *          オープンするまでブロックされる。
 *==================================================*/
int main(void)
{
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_mkfifoResult;
    S4 s4_fileDescriptor;
    S4 s4_readResult;
    S4 s4_closeResult;
    S4 s4_unlinkResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    s4_mkfifoResult = mkfifo(FIFO_PATH, FIFO_MODE);

    if (s4_mkfifoResult == SYSTEM_CALL_ERROR) {
        perror("mkfifo");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        s4_fileDescriptor = open(FIFO_PATH, O_RDONLY);

        if (s4_fileDescriptor == SYSTEM_CALL_ERROR) {
            perror("open");

            s4_exitStatus = EXIT_FAILURE;
        } else {
            s4_readResult = read(s4_fileDescriptor, ch_receiveBuffer, BUFFER_SIZE);

            /* s4_readResultが0の場合にch_receiveBuffer[s4_readResult - 1]を参照すると
               範囲外アクセスとなるため、0の判定を終端の判定より先に行う。 */
            if (s4_readResult == SYSTEM_CALL_ERROR) {
                perror("read");

                s4_exitStatus = EXIT_FAILURE;
            } else if (s4_readResult == 0) {
                printf("送信側が名前つきパイプをクローズしました\n");

                s4_exitStatus = EXIT_FAILURE;
            } else if (ch_receiveBuffer[s4_readResult - 1] != '\0') {
                printf("受信文字列が終端されていません\n");

                s4_exitStatus = EXIT_FAILURE;
            } else {
                printf("受信文字列：%s\n", ch_receiveBuffer);
            }

            s4_closeResult = close(s4_fileDescriptor);

            if (s4_closeResult == SYSTEM_CALL_ERROR) {
                perror("close");

                s4_exitStatus = EXIT_FAILURE;
            } else {
                /* DO NOTHING */
            }
        }

        /* 名前つきパイプはプロセスが終了しても存在し続けるため、使用後に削除する。 */
        s4_unlinkResult = unlink(FIFO_PATH);

        if (s4_unlinkResult == SYSTEM_CALL_ERROR) {
            perror("unlink");

            s4_exitStatus = EXIT_FAILURE;
        } else {
            /* DO NOTHING */
        }
    }

    return s4_exitStatus;
}
