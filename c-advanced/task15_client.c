/*==================================================*
 * @file      task15_client_Rev_0_2.c
 * @brief     UNIXドメインのストリームソケットを使用したデータ送信および受信（クライアント）
 * @version   0.2
 * @date      2026.08.02
 * @author    KAIQUN LUO
 *==================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "type.h"

/*==================================================*
 * 設計方針
 *==================================================*
 * 【エラー発生時の方針】
 * エラーを検出した場合は、それまでに獲得した資源の後始末（ソケットの
 * クローズ）を行ってからその場でEXIT_FAILUREを返す。
 *
 * 【複数個のreturnについて】
 * 深い分岐のnestを避けるため、main関数内で複数個のreturnを使用する。
 * 規約「複数個のreturnを使用した方が可読性が高まる場合に限り許可する」
 * に基づく。フロー図にも複数個のreturnを明記している。
 *
 * 【文字列の終端】
 * 終端のヌル文字は送信せず、受信側でバッファに付与する。
 *==================================================*/

/*==================================================*
 * ソケット情報
 *==================================================*/
#define SOCKET_PATH         ("./task15_socket") /* UNIXドメインソケットのパス */
#define SOCKET_PROTOCOL     (0)                 /* UNIXが適切なプロトコルを選択する */

/*==================================================*
 * 送信文字列
 *==================================================*/
#define SEND_MESSAGE        ("Hello, I'm the client.")  /* サーバーへ送信する文字列 */
#define SEND_MESSAGE_BYTE   (sizeof(SEND_MESSAGE) - 1)  /* 上記の送信バイト数（終端のヌル文字を含まない） */

/*==================================================*
 * バッファサイズ
 *==================================================*/
#define BUFFER_SIZE         (100)   /* 受信文字列を格納する配列のサイズ */

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR   (-1)    /* エラー発生時の戻り値 */

/*==================================================*
 * @brief   ソケットを作成してサーバーへ接続を要求し、文字列を送信した後、
 *          サーバーから文字列を受信して表示し、ソケットをクローズする
 * @param   なし
 * @retval  int  EXIT_SUCCESS  サーバーとの通信に成功した場合
 *               EXIT_FAILURE  システムコールでエラーが発生した場合
 * @note    サーバープログラムを起動した後に本プログラムを起動すること。
 *          クライアント側はbindによる名前づけを省略することができる。
 *==================================================*/
int main(void)
{
    struct sockaddr_un st_serverAddress;
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_socketDescriptor;
    S4 s4_callResult;

    s4_socketDescriptor = socket(PF_UNIX, SOCK_STREAM, SOCKET_PROTOCOL);

    if (s4_socketDescriptor == SYSTEM_CALL_ERROR) {
        perror("socket");

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    memset(&st_serverAddress, 0, sizeof(st_serverAddress));

    st_serverAddress.sun_family = PF_UNIX;

    strcpy(st_serverAddress.sun_path, SOCKET_PATH);

    s4_callResult = connect(s4_socketDescriptor,
                            (struct sockaddr *)&st_serverAddress,
                            sizeof(st_serverAddress));

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("connect");

        (VD)close(s4_socketDescriptor);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = write(s4_socketDescriptor, SEND_MESSAGE, SEND_MESSAGE_BYTE);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("write");

        (VD)close(s4_socketDescriptor);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = read(s4_socketDescriptor, ch_receiveBuffer, BUFFER_SIZE - 1);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("read");

        (VD)close(s4_socketDescriptor);

        return EXIT_FAILURE;
    } else {
        ch_receiveBuffer[s4_callResult] = '\0';

        printf("クライアントの受信文字列：%s\n", ch_receiveBuffer);
    }

    s4_callResult = close(s4_socketDescriptor);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("close");

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    return EXIT_SUCCESS;
}
