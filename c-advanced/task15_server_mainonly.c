/*==================================================*
 * @file      task15_server_mainonly_Rev_0_1.c
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
 * エラーを検出した場合は、それまでに獲得した資源の後始末（ソケットの
 * クローズおよび削除）を行ってからその場でEXIT_FAILUREを返す。
 *
 * 【複数個のreturnについて】
 * 深い分岐のnestを避けるため、main関数内で複数個のreturnを使用する。
 * 規約「複数個のreturnを使用した方が可読性が高まる場合に限り許可する」
 * に基づく。フロー図にも複数個のreturnを明記している。
 *
 * 【本ファイルについて】
 * 本ファイルは処理をすべてmain関数に記述した版である。
 * 通信処理を自作関数に分割した版は task15_server.c を参照のこと。
 * 両者の動作は同一である。
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
 * @brief   ソケットを作成して名前をつけ、接続要求を受け入れて文字列を受信し、
 *          クライアントへ文字列を送信した後、ソケットをクローズして削除する
 * @param   なし
 * @retval  int  EXIT_SUCCESS  クライアントとの通信に成功した場合
 *               EXIT_FAILURE  システムコールでエラーが発生した場合
 * @note    本プログラムを起動した後にクライアントプログラムを起動すること。
 *          処理を自作関数に分割した版は task15_server.c を参照のこと。
 *==================================================*/
int main(void)
{
    struct sockaddr_un st_serverAddress;
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_socketDescriptor;
    S4 s4_acceptDescriptor;
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

    /* クライアントのアドレス情報は使用しないため、第2引数と第3引数にはNULLを指定する。 */
    s4_acceptDescriptor = accept(s4_socketDescriptor, NULL, NULL);

    if (s4_acceptDescriptor == SYSTEM_CALL_ERROR) {
        perror("accept");

        (VD)close(s4_socketDescriptor);
        (VD)unlink(SOCKET_PATH);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = read(s4_acceptDescriptor, ch_receiveBuffer, BUFFER_SIZE - 1);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("read");

        (VD)close(s4_acceptDescriptor);
        (VD)close(s4_socketDescriptor);
        (VD)unlink(SOCKET_PATH);

        return EXIT_FAILURE;
    } else {
        ch_receiveBuffer[s4_callResult] = '\0';

        printf("サーバーの受信文字列：%s\n", ch_receiveBuffer);
    }

    s4_callResult = write(s4_acceptDescriptor, SEND_MESSAGE, SEND_MESSAGE_BYTE);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("write");

        (VD)close(s4_acceptDescriptor);
        (VD)close(s4_socketDescriptor);
        (VD)unlink(SOCKET_PATH);

        return EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    s4_callResult = close(s4_acceptDescriptor);

    if (s4_callResult == SYSTEM_CALL_ERROR) {
        perror("close");

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
