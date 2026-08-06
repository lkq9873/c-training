/*==================================================*
 * @file      task15_server_Rev_0_2.c
 * @brief     UNIXドメインのストリームソケットを使用したデータ受信および送信（サーバー）
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
 * クローズおよび削除）を行ってからその場でEXIT_FAILUREを返す。
 *
 * 【複数個のreturnについて】
 * main関数は深い分岐のnestを避けるため複数個のreturnを使用する。
 * 自作関数は入口1個・出口1個とするため、戻り値を変数に保持して
 * 関数末尾で1個のreturnにより返す。
 *
 * 【文字列の終端】
 * 終端のヌル文字は送信せず、受信側でバッファに付与する。
 *==================================================*/

/*==================================================*
 * ソケット情報
 *==================================================*/
#define SOCKET_PATH         ("./task15_socket") /* UNIXドメインソケットのパス */
#define SOCKET_PROTOCOL     (0)                 /* UNIXが適切なプロトコルを選択する */
#define LISTEN_BACKLOG      (5)                 /* 受け入れる接続要求の数 */

/*==================================================*
 * 送信文字列
 *==================================================*/
#define SEND_MESSAGE        ("Hello, I'm the server.")  /* クライアントへ送信する文字列 */
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
 * 関数プロトタイプ宣言
 *==================================================*/
S4 s4_CommunicateWithClient(S4 s4_socketDescriptor);

/*==================================================*
 * @brief   ソケットを作成して名前をつけ、接続要求の受け入れ準備を行った後、
 *          クライアントとの通信処理を呼び出し、ソケットを削除する
 * @param   なし
 * @retval  int  EXIT_SUCCESS  クライアントとの通信に成功した場合
 *               EXIT_FAILURE  システムコールまたは通信処理でエラーが発生した場合
 * @note    本プログラムを起動した後にクライアントプログラムを起動すること。
 *==================================================*/
int main(void)
{
    struct sockaddr_un st_serverAddress;
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

    s4_callResult = bind(s4_socketDescriptor,
                         (struct sockaddr *)&st_serverAddress,
                         sizeof(st_serverAddress));

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("bind");

        /* bindに失敗した場合はソケットのファイルが作成されないため、
           クローズのみを行う。 */
        (VD)close(s4_socketDescriptor);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = listen(s4_socketDescriptor, LISTEN_BACKLOG);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("listen");

        (VD)close(s4_socketDescriptor);
        (VD)unlink(SOCKET_PATH);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    /* 本関数内でperrorを実行済みのため、ここではエラーメッセージを出力しない。 */
    s4_callResult = s4_CommunicateWithClient(s4_socketDescriptor);

    if (s4_callResult == EXIT_FAILURE) {
        (VD)close(s4_socketDescriptor);
        (VD)unlink(SOCKET_PATH);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = close(s4_socketDescriptor);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("close");

        (VD)unlink(SOCKET_PATH);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    /* bindによって作成されたソケットのファイルはプロセスが終了しても
       存在し続けるため、使用後に削除する。 */
    s4_callResult = unlink(SOCKET_PATH);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("unlink");

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    return EXIT_SUCCESS;
}

/*==================================================*
 * @brief   クライアントからの接続要求を受け入れ、文字列を受信して表示した後、
 *          クライアントへ文字列を送信し、接続用のソケットをクローズする
 * @param[in]   S4 s4_socketDescriptor  接続要求を待っているソケットのファイル記述子
 * @retval  S4  EXIT_SUCCESS  文字列の受信および送信に成功した場合
 *              EXIT_FAILURE  システムコールでエラーが発生した場合
 * @note    入口1個・出口1個とするため、戻り値をs4_returnValueに保持し、
 *          関数末尾で1個のreturnにより返す。
 *==================================================*/
S4 s4_CommunicateWithClient(S4 s4_socketDescriptor)
{
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_acceptDescriptor;
    S4 s4_callResult;
    S4 s4_returnValue = EXIT_SUCCESS;

    /* クライアントのアドレス情報は使用しないため、第2引数と第3引数にはNULLを指定する。 */
    s4_acceptDescriptor = accept(s4_socketDescriptor, NULL, NULL);

    if (s4_acceptDescriptor == SYSTEM_CALL_ERROR) {
        perror("accept");

        s4_returnValue = EXIT_FAILURE;
    } else {
        s4_callResult = read(s4_acceptDescriptor, ch_receiveBuffer, BUFFER_SIZE - 1);

        if (s4_callResult == SYSTEM_CALL_ERROR) {
            perror("read");

            s4_returnValue = EXIT_FAILURE;
        } else {
            ch_receiveBuffer[s4_callResult] = '\0';

            printf("サーバーの受信文字列：%s\n", ch_receiveBuffer);

            s4_callResult = write(s4_acceptDescriptor, SEND_MESSAGE, SEND_MESSAGE_BYTE);

            if (s4_callResult == SYSTEM_CALL_ERROR) {
                perror("write");

                s4_returnValue = EXIT_FAILURE;
            } else {
                /* DO NOTHING */
            }
        }

        /* 受信または送信に失敗した場合も、接続用のソケットは必ずクローズする。 */
        s4_callResult = close(s4_acceptDescriptor);

        if (s4_callResult == SYSTEM_CALL_ERROR) {
            perror("close");

            s4_returnValue = EXIT_FAILURE;
        } else {
            /* DO NOTHING */
        }
    }

    return s4_returnValue;
}
