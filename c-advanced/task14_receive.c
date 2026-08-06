/*==================================================*
 * @file      task14_receive_Rev_0_2.c
 * @brief     名前つきパイプを使用したデータ受信
 * @version   0.2
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
 * エラーを検出した場合は、それまでに獲得した資源の後始末（名前つきパイプの
 * クローズおよび削除）を行ってからその場でEXIT_FAILUREを返し、
 * 以降の処理は行わない。
 *
 * 【複数個のreturnについて】
 * 深い分岐のnestを避けるため、main関数内で複数個のreturnを使用する。
 * 規約「複数個のreturnを使用した方が可読性が高まる場合に限り許可する」
 * に基づく。フロー図にも複数個のreturnを明記している。
 *
 * 【文字列の終端】
 * 送信側は終端のヌル文字を送信しないため、受信側でバッファに付与する。
 * このためreadで読み出すバイト数はBUFFER_SIZEから1を減じた値とし、
 * 終端のヌル文字を格納する領域を確保する。
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
    S4 s4_fileDescriptor;
    S4 s4_callResult;

    s4_callResult = mkfifo(FIFO_PATH, FIFO_MODE);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("mkfifo");

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_fileDescriptor = open(FIFO_PATH, O_RDONLY);

    if (s4_fileDescriptor == SYSTEM_CALL_ERROR) {
        perror("open");

        /* 作成済みの名前つきパイプを削除してから戻る。 */
        (VD)unlink(FIFO_PATH);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = read(s4_fileDescriptor, ch_receiveBuffer, BUFFER_SIZE - 1);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("read");

        /* オープン済みの名前つきパイプをクローズし、削除してから戻る。 */
        (VD)close(s4_fileDescriptor);
        (VD)unlink(FIFO_PATH);

        return EXIT_FAILURE;
    } else {
        ch_receiveBuffer[s4_callResult] = '\0';

        printf("受信文字列：%s\n", ch_receiveBuffer);
    }

    s4_callResult = close(s4_fileDescriptor);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("close");

        /* 作成済みの名前つきパイプを削除してから戻る。 */
        (VD)unlink(FIFO_PATH);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    /* 名前つきパイプはプロセスが終了しても存在し続けるため、使用後に削除する。 */
    s4_callResult = unlink(FIFO_PATH);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("unlink");

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    return EXIT_SUCCESS;
}
