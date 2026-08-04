/*==================================================*
 * @file      task15_server_Rev_0_1.c
 * @brief     UNIXドメインのストリームソケットを使用したデータ受信および送信（サーバー）
 * @version   0.1
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
 * 前提が成立せず処理を続行できない場合（socketの失敗後のbind等）は打ち切り、
 * 続行可能な場合（受信の失敗後の送信）は継続する。継続とするのは、
 * 一方が先に終了すると相手のwriteが読み手のいないソケットへの書き込みと
 * なりSIGPIPEで異常終了してしまうためである。
 * 後処理（close、unlink）は必ず実行する。
 *
 * 【文字列の終端】
 * 送信側は終端のヌル文字を含めて送信し、受信側は受信データの最終バイトが
 * ヌル文字であることを確認する。これにより、受信データがバッファサイズを
 * 超えて途中で切れた場合を検出することができる。
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
#define SEND_MESSAGE_BYTE   (sizeof(SEND_MESSAGE))  /* 上記の送信バイト数（終端のヌル文字を含む） */

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
 *               EXIT_FAILURE  システムコールでエラーが発生した場合
 * @note    本プログラムを起動した後にクライアントプログラムを起動すること。
 *==================================================*/
int main(void)
{
    struct sockaddr_un st_serverAddress;
    S4 s4_socketDescriptor;
    S4 s4_bindResult;
    S4 s4_listenResult;
    S4 s4_unlinkResult;
    S4 s4_closeResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    s4_socketDescriptor = socket(PF_UNIX, SOCK_STREAM, SOCKET_PROTOCOL);

    if (s4_socketDescriptor == SYSTEM_CALL_ERROR) {
        perror("socket");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        memset(&st_serverAddress, 0, sizeof(st_serverAddress));

        st_serverAddress.sun_family = PF_UNIX;

        strcpy(st_serverAddress.sun_path, SOCKET_PATH);

        s4_bindResult = bind(s4_socketDescriptor,
                             (struct sockaddr *)&st_serverAddress,
                             sizeof(st_serverAddress));

        if (s4_bindResult == SYSTEM_CALL_ERROR) {
            perror("bind");

            s4_exitStatus = EXIT_FAILURE;
        } else {
            s4_listenResult = listen(s4_socketDescriptor, LISTEN_BACKLOG);

            if (s4_listenResult == SYSTEM_CALL_ERROR) {
                perror("listen");

                s4_exitStatus = EXIT_FAILURE;
            } else {
                s4_exitStatus = s4_CommunicateWithClient(s4_socketDescriptor);
            }

            /* bindによって作成されたソケットのファイルはプロセスが終了しても
               存在し続けるため、使用後に削除する。 */
            s4_unlinkResult = unlink(SOCKET_PATH);

            if (s4_unlinkResult == SYSTEM_CALL_ERROR) {
                perror("unlink");

                s4_exitStatus = EXIT_FAILURE;
            } else {
                /* DO NOTHING */
            }
        }

        s4_closeResult = close(s4_socketDescriptor);

        if (s4_closeResult == SYSTEM_CALL_ERROR) {
            perror("close");

            s4_exitStatus = EXIT_FAILURE;
        } else {
            /* DO NOTHING */
        }
    }

    return s4_exitStatus;
}

/*==================================================*
 * @brief   クライアントからの接続要求を受け入れ、文字列を受信して表示した後、
 *          クライアントへ文字列を送信し、接続用のソケットをクローズする
 * @param[in]   S4 s4_socketDescriptor  接続要求を待っているソケットのファイル記述子
 * @retval  S4  EXIT_SUCCESS  文字列の受信および送信に成功した場合
 *              EXIT_FAILURE  システムコールでエラーが発生した場合
 *==================================================*/
S4 s4_CommunicateWithClient(S4 s4_socketDescriptor)
{
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_acceptDescriptor;
    S4 s4_readResult;
    S4 s4_writeResult;
    S4 s4_closeResult;
    S4 s4_returnValue = EXIT_SUCCESS;

    /* クライアントのアドレス情報は使用しないため、第2引数と第3引数にはNULLを指定する。 */
    s4_acceptDescriptor = accept(s4_socketDescriptor, NULL, NULL);

    if (s4_acceptDescriptor == SYSTEM_CALL_ERROR) {
        perror("accept");

        s4_returnValue = EXIT_FAILURE;
    } else {
        s4_readResult = read(s4_acceptDescriptor, ch_receiveBuffer, BUFFER_SIZE);

        /* s4_readResultが0の場合にch_receiveBuffer[s4_readResult - 1]を参照すると
           範囲外アクセスとなるため、0の判定を終端の判定より先に行う。 */
        if (s4_readResult == SYSTEM_CALL_ERROR) {
            perror("read");

            s4_returnValue = EXIT_FAILURE;
        } else if (s4_readResult == 0) {
            printf("クライアントがソケットをクローズしました\n");

            s4_returnValue = EXIT_FAILURE;
        } else if (ch_receiveBuffer[s4_readResult - 1] != '\0') {
            printf("受信文字列が終端されていません\n");

            s4_returnValue = EXIT_FAILURE;
        } else {
            printf("サーバーの受信文字列：%s\n", ch_receiveBuffer);
        }

        s4_writeResult = write(s4_acceptDescriptor, SEND_MESSAGE, SEND_MESSAGE_BYTE);

        if (s4_writeResult == SYSTEM_CALL_ERROR) {
            perror("write");

            s4_returnValue = EXIT_FAILURE;
        } else {
            /* DO NOTHING */
        }

        s4_closeResult = close(s4_acceptDescriptor);

        if (s4_closeResult == SYSTEM_CALL_ERROR) {
            perror("close");

            s4_returnValue = EXIT_FAILURE;
        } else {
            /* DO NOTHING */
        }
    }

    return s4_returnValue;
}
