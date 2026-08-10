/*==================================================*
 * @file      task19_client.c
 * @brief     UNIXドメインソケットを使用した定期データ送信
 * @version   0.1
 * @date      2026.08.10
 * @author    KAIQUN LUO
 *==================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "type.h"

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR  (-1)

/*==================================================*
 * UNIXドメインソケット情報
 *==================================================*/
#define SOCKET_PATH        ("/tmp/task19_socket")

/*==================================================*
 * 送信バッファサイズ
 *==================================================*/
#define SEND_BUFFER_SIZE   (100)

/*==================================================*
 * @brief   現在の西暦年月日時分秒および曜日を取得し、
 *          UNIXドメインソケットを使用してサーバーへ送信する
 * @param   なし
 * @retval  int  EXIT_SUCCESS 処理に成功した場合
 *               EXIT_FAILURE 処理に失敗した場合
 *==================================================*/
int main(void)
{
    struct sockaddr_un st_serverAddress;
    struct tm *pst_localTime;
    time_t t_currentTime;
    char ch_sendBuffer[SEND_BUFFER_SIZE];
    S4 s4_socket;
    S4 s4_connectResult;
    S4 s4_sendDataLength;
    S4 s4_writeResult;

    t_currentTime = time(NULL);

    if (t_currentTime == (time_t)-1) {
        perror("time");

        return EXIT_FAILURE;
    }

    pst_localTime = localtime(
        &t_currentTime
    );

    if (pst_localTime == NULL) {
        perror("localtime");

        return EXIT_FAILURE;
    }

    s4_sendDataLength = strftime(
        ch_sendBuffer,
        SEND_BUFFER_SIZE,
        "%Y/%m/%d %H:%M:%S %A\n",
        pst_localTime
    );

    if (s4_sendDataLength == 0) {
        printf(
            "送信データの作成に失敗しました。\n"
        );

        return EXIT_FAILURE;
    }

    s4_socket = socket(
        PF_UNIX,
        SOCK_STREAM,
        0
    );

    if (s4_socket == SYSTEM_CALL_ERROR) {
        perror("socket");

        return EXIT_FAILURE;
    }

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

    s4_connectResult = connect(
        s4_socket,
        (struct sockaddr *)&st_serverAddress,
        sizeof(st_serverAddress)
    );

    if (s4_connectResult == SYSTEM_CALL_ERROR) {
        perror("connect");

        (VD)close(s4_socket);

        return EXIT_FAILURE;
    }

    s4_writeResult = write(
        s4_socket,
        ch_sendBuffer,
        s4_sendDataLength
    );

    if (s4_writeResult == SYSTEM_CALL_ERROR) {
        perror("write");

        (VD)close(s4_socket);

        return EXIT_FAILURE;
    }

    (VD)close(s4_socket);

    return EXIT_SUCCESS;
}