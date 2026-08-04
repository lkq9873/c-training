/*==================================================*
 * @file      task16_client_Rev_0_1.c
 * @brief     INETドメインのストリームソケットを使用したデータ送信および受信（クライアント）
 * @version   0.1
 * @date      2026.08.02
 * @author    KAIQUN LUO
 *==================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "type.h"

/*==================================================*
 * 設計方針
 *==================================================*
 * 【エラー発生時の方針】
 * 前提が成立せず処理を続行できない場合（socketの失敗後のconnect等）は打ち切り、
 * 続行可能な場合（送信の失敗後の受信）は継続する。継続とするのは、
 * 一方が先に終了すると相手のwriteが読み手のいないソケットへの書き込みと
 * なりSIGPIPEで異常終了してしまうためである。
 * 後処理（close）は必ず実行する。
 *
 * 【文字列の終端】
 * 送信側は終端のヌル文字を含めて送信し、受信側は受信データの最終バイトが
 * ヌル文字であることを確認する。これにより、受信データがバッファサイズを
 * 超えて途中で切れた場合を検出することができる。
 *==================================================*/

/*==================================================*
 * ソケット情報
 *==================================================*/
#define SERVER_IP_ADDRESS   ("xxx.xxx.xxx.xxx") /* サーバーのIPアドレス（教材に記載のアドレスに変更すること） */
#define SERVER_PORT         (49152) /* サーバーのポート番号（別途指示される番号に変更すること） */
#define SOCKET_PROTOCOL     (0)     /* UNIXが適切なプロトコルを選択する */

/*==================================================*
 * 送信文字列
 *==================================================*/
#define SEND_MESSAGE        ("Hello, I'm the client.")  /* サーバーへ送信する文字列 */
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
 * @brief   ソケットを作成してサーバーへ接続を要求し、文字列を送信した後、
 *          サーバーから文字列を受信して表示し、ソケットをクローズする
 * @param   なし
 * @retval  int  EXIT_SUCCESS  サーバーとの通信に成功した場合
 *               EXIT_FAILURE  システムコールでエラーが発生した場合
 * @note    本プログラムはRaspberry Pi上で実行する。
 *          サーバープログラムを起動した後に本プログラムを起動すること。
 *          クライアント側はbindによる名前づけを省略することができる。
 *==================================================*/
int main(void)
{
    struct sockaddr_in st_serverAddress;
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_socketDescriptor;
    S4 s4_connectResult;
    S4 s4_writeResult;
    S4 s4_readResult;
    S4 s4_closeResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    s4_socketDescriptor = socket(PF_INET, SOCK_STREAM, SOCKET_PROTOCOL);

    if (s4_socketDescriptor == SYSTEM_CALL_ERROR) {
        perror("socket");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        memset(&st_serverAddress, 0, sizeof(st_serverAddress));

        st_serverAddress.sin_family = PF_INET;

        st_serverAddress.sin_port = htons(SERVER_PORT);

        /* inet_addr関数はIPアドレスのドットつき10進表現を
           ネットワークバイトオーダーに変換する。 */
        st_serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);

        s4_connectResult = connect(s4_socketDescriptor,
                                   (struct sockaddr *)&st_serverAddress,
                                   sizeof(st_serverAddress));

        if (s4_connectResult == SYSTEM_CALL_ERROR) {
            perror("connect");

            s4_exitStatus = EXIT_FAILURE;
        } else {
            s4_writeResult = write(s4_socketDescriptor, SEND_MESSAGE, SEND_MESSAGE_BYTE);

            if (s4_writeResult == SYSTEM_CALL_ERROR) {
                perror("write");

                s4_exitStatus = EXIT_FAILURE;
            } else {
                /* DO NOTHING */
            }

            s4_readResult = read(s4_socketDescriptor, ch_receiveBuffer, BUFFER_SIZE);

            /* s4_readResultが0の場合にch_receiveBuffer[s4_readResult - 1]を参照すると
               範囲外アクセスとなるため、0の判定を終端の判定より先に行う。 */
            if (s4_readResult == SYSTEM_CALL_ERROR) {
                perror("read");

                s4_exitStatus = EXIT_FAILURE;
            } else if (s4_readResult == 0) {
                printf("サーバーがソケットをクローズしました\n");

                s4_exitStatus = EXIT_FAILURE;
            } else if (ch_receiveBuffer[s4_readResult - 1] != '\0') {
                printf("受信文字列が終端されていません\n");

                s4_exitStatus = EXIT_FAILURE;
            } else {
                printf("クライアントの受信文字列：%s\n", ch_receiveBuffer);
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
