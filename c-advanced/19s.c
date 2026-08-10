/*==================================================*
 * @file      task19_server.c
 * @brief     UNIXドメインソケットを使用した定期データ受信
 * @version   0.1
 * @date      2026.08.10
 * @author    KAIQUN LUO
 *==================================================*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "type.h"

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR    (-1)

/*==================================================*
 * UNIXドメインソケット情報
 *==================================================*/
#define SOCKET_PATH          ("/tmp/task19_socket")

/*==================================================*
 * 受信データ保存ファイル情報
 *==================================================*/
#define LOG_FILE_PATH        ("./task19.log")
#define LOG_FILE_MODE        (0660)

/*==================================================*
 * 受信バッファサイズ
 *==================================================*/
#define RECEIVE_BUFFER_SIZE  (100)

/*==================================================*
 * @brief   UNIXドメインソケットでクライアントからデータを受信し、
 *          受信したデータを1つのファイルへ蓄積する
 * @param   なし
 * @retval  int  EXIT_SUCCESS 処理に成功した場合
 *               EXIT_FAILURE 処理に失敗した場合
 *==================================================*/
int main(void)
{
    struct sockaddr_un st_serverAddress;
    char ch_receiveBuffer[RECEIVE_BUFFER_SIZE];
    S4 s4_serverSocket;
    S4 s4_clientSocket;
    S4 s4_bindResult;
    S4 s4_listenResult;
    S4 s4_readResult;
    S4 s4_logFileDescriptor;
    S4 s4_writeResult;

    s4_serverSocket = socket(
        PF_UNIX,
        SOCK_STREAM,
        0
    );

    if (s4_serverSocket == SYSTEM_CALL_ERROR) {
        perror("socket");

        return EXIT_FAILURE;
    }

    /*
     * 前回の実行でソケットファイルが残っている可能性があるため、
     * bindを実行する前に既存のソケットファイルを削除する。
     */
    (VD)unlink(SOCKET_PATH);

    memset(
        &st_serverAddress,
        0,
        sizeof(st_serverAddress)
    );

    st_serverAddress.sun_family = PF_UNIX;

    strcpy(
        st_serverAddress.sun_path,
        SOCKET_PATH
    );

    s4_bindResult = bind(
        s4_serverSocket,
        (struct sockaddr *)&st_serverAddress,
        sizeof(st_serverAddress)
    );

    if (s4_bindResult == SYSTEM_CALL_ERROR) {
        perror("bind");

        (VD)close(s4_serverSocket);
        (VD)unlink(SOCKET_PATH);

        return EXIT_FAILURE;
    }

    s4_listenResult = listen(
        s4_serverSocket,
        5
    );

    if (s4_listenResult == SYSTEM_CALL_ERROR) {
        perror("listen");

        (VD)close(s4_serverSocket);
        (VD)unlink(SOCKET_PATH);

        return EXIT_FAILURE;
    }

    while (1) {
        s4_clientSocket = accept(
            s4_serverSocket,
            NULL,
            NULL
        );

        if (s4_clientSocket == SYSTEM_CALL_ERROR) {
            perror("accept");

            (VD)close(s4_serverSocket);
            (VD)unlink(SOCKET_PATH);

            return EXIT_FAILURE;
        }

        s4_readResult = read(
            s4_clientSocket,
            ch_receiveBuffer,
            RECEIVE_BUFFER_SIZE - 1
        );

        if (s4_readResult == SYSTEM_CALL_ERROR) {
            perror("read");

            (VD)close(s4_clientSocket);
            (VD)close(s4_serverSocket);
            (VD)unlink(SOCKET_PATH);

            return EXIT_FAILURE;
        }

        ch_receiveBuffer[s4_readResult] = '\0';

        printf(
            "受信データ：%s",
            ch_receiveBuffer
        );

        s4_logFileDescriptor = open(
            LOG_FILE_PATH,
            O_CREAT | O_WRONLY | O_APPEND,
            LOG_FILE_MODE
        );

        if (s4_logFileDescriptor == SYSTEM_CALL_ERROR) {
            perror("open");

            (VD)close(s4_clientSocket);
            (VD)close(s4_serverSocket);
            (VD)unlink(SOCKET_PATH);

            return EXIT_FAILURE;
        }

        s4_writeResult = write(
            s4_logFileDescriptor,
            ch_receiveBuffer,
            s4_readResult
        );

        if (s4_writeResult == SYSTEM_CALL_ERROR) {
            perror("write");

            (VD)close(s4_logFileDescriptor);
            (VD)close(s4_clientSocket);
            (VD)close(s4_serverSocket);
            (VD)unlink(SOCKET_PATH);

            return EXIT_FAILURE;
        }

        (VD)close(s4_logFileDescriptor);
        (VD)close(s4_clientSocket);
    }

    return EXIT_SUCCESS;
}