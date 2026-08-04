/*==================================================*
 * @file      task12_Rev_0_1.c
 * @brief     パイプを使用した親子プロセス間通信
 * @version   0.2
 * @date      2026.07.29
 * @author    KAIQUN LUO
 *==================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "type.h"

/*==================================================*
 * forkシステムコール戻り値
 *==================================================*/
#define FORK_CHILD_PROCESS  (0)     /* 子プロセスに返される値 */

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR   (-1)    /* エラー発生時の戻り値 */

/*==================================================*
 * 送信文字列
 *==================================================*/
#define SEND_MESSAGE        ("Hello, I'm your child.") /* 子プロセスから親プロセスへ送信する文字列 */

/*==================================================*
 * バッファサイズ
 *==================================================*/
#define BUFFER_SIZE (100)   /* バッファサイズ */

/*==================================================*
 * 関数プロトタイプ定義
 *==================================================*/
VD vd_ChildProcess(S4 *ps4_fileDescriptor);
S4 s4_ParentProcess(S4 *ps4_fileDescriptor);

/*==================================================*
 * @brief   パイプを生成した後、forkによって子プロセスを生成し、親プロセス処理または子プロセス処理を呼び出す
 * @param   なし
 * @retval  int  0
 *==================================================*/
int main(void)
{
    S4 s4_fileDescriptor[2];
    S4 s4_pipeResult;
    S4 s4_forkResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    s4_pipeResult = pipe(s4_fileDescriptor);

    if (s4_pipeResult == SYSTEM_CALL_ERROR) {
        perror("pipe");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        s4_forkResult = fork();

        if (s4_forkResult == SYSTEM_CALL_ERROR) {
            perror("fork");

            (VD)close(s4_fileDescriptor[0]);
            (VD)close(s4_fileDescriptor[1]);

            s4_exitStatus = EXIT_FAILURE;
        } else if (s4_forkResult == FORK_CHILD_PROCESS) {
            vd_ChildProcess(s4_fileDescriptor);
        } else {
            s4_exitStatus = s4_ParentProcess(s4_fileDescriptor);
        }
    }

    exit(s4_exitStatus);

    return 0;
}

/*==================================================*
 * @brief   パイプを使用して親プロセスへ文字列を送信する
 * @param   S4 *ps4_fileDescriptor  パイプのファイル記述子配列の先頭アドレス
 * @retval  なし
 *==================================================*/
VD vd_ChildProcess(S4 *ps4_fileDescriptor)
{
    S4 s4_writeResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    /* ps4_fileDescriptor[0]はパイプの読み出しのファイル記述子を示す。
     * 子プロセスではパイプへの書き込みのみを行い、読み出しは使用しないためクローズする。 */
    (VD)close(ps4_fileDescriptor[0]);

    s4_writeResult = write(ps4_fileDescriptor[1], SEND_MESSAGE, sizeof(SEND_MESSAGE) - 1);

    if (s4_writeResult == SYSTEM_CALL_ERROR) {
        perror("write");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    (VD)close(ps4_fileDescriptor[1]);

    exit(s4_exitStatus);
}

/*==================================================*
 * @brief   子プロセスから文字列を受信し、子プロセスの終了を待ち、終了状態を表示する
 * @param   S4 *ps4_fileDescriptor  ファイル記述子配列の先頭アドレス
 * @retval  S4  EXIT_SUCCESS  処理に成功した場合
 *              EXIT_FAILURE  処理に失敗した場合
 *==================================================*/
S4 s4_ParentProcess(S4 *ps4_fileDescriptor)
{
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_childStatus;
    S4 s4_waitResult;
    S4 s4_readResult;
    S4 s4_returnValue = EXIT_SUCCESS;


    /* ps4_fileDescriptor[1]はパイプの書き込みのファイル記述子を示す。
       親プロセスではパイプからの読み出しのみを行い、書き込みは使用しないためクローズする。 */
    (VD)close(ps4_fileDescriptor[1]);

    s4_readResult = read(ps4_fileDescriptor[0], ch_receiveBuffer, BUFFER_SIZE - 1);

    if (s4_readResult == SYSTEM_CALL_ERROR) {
        perror("read");

        s4_returnValue = EXIT_FAILURE;
    } else {
        ch_receiveBuffer[s4_readResult] = '\0';

        printf("受信文字列：%s\n", ch_receiveBuffer);
    }

    (VD)close(ps4_fileDescriptor[0]);

    s4_waitResult = wait(&s4_childStatus);

    if (s4_waitResult == SYSTEM_CALL_ERROR) {
        perror("wait");

        s4_returnValue = EXIT_FAILURE;
    } else {
        if (WIFEXITED(s4_childStatus)) {
            printf(
                "子プロセスは正常に終了\n"
                "終了ステータス：%d\n",
                WEXITSTATUS(s4_childStatus)
            );
        } else if (WIFSIGNALED(s4_childStatus)) {
            printf(
                "子プロセスはシグナルによって終了\n"
                "終了シグナル番号：%d\n",
                WTERMSIG(s4_childStatus)
            );
        } else {
            printf("未知の要因で終了。\n");

            s4_returnValue = EXIT_FAILURE;
        }
    }

    return s4_returnValue;
}