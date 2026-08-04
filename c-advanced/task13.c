/*==================================================*
 * @file      task13_Rev_0_1.c
 * @brief     双方向パイプを使用した親子プロセス間通信
 * @version   0.1
 * @date      2026.08.02
 * @author    KAIQUN LUO
 *==================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "type.h"

/*==================================================*
 * 設計方針
 *==================================================*
 * 【エラー発生時の方針】
 * 前提が成立せず処理を続行できない場合（pipeの失敗後のfork等）は打ち切り、
 * 続行可能な場合（送信の失敗後の受信等）は継続する。継続とするのは、
 * 一方のプロセスが先に終了すると相手プロセスのwriteが読み手のいない
 * パイプへの書き込みとなりSIGPIPEで異常終了してしまうため、および
 * 親プロセスが打ち切るとwaitを実行できず子プロセスの終了状態を
 * 取得できなくなるためである。
 *
 * 【文字列の終端】
 * 送信側は終端のヌル文字を含めて送信し、受信側は受信データの最終バイトが
 * ヌル文字であることを確認する。これにより、受信データがバッファサイズを
 * 超えて途中で切れた場合を検出することができる。
 *==================================================*/

/*==================================================*
 * forkシステムコール戻り値
 *==================================================*/
#define FORK_CHILD_PROCESS      (0)     /* 子プロセスに返される値 */

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR       (-1)    /* エラー発生時の戻り値 */

/*==================================================*
 * 送信文字列
 *==================================================*/
#define CHILD_SEND_MESSAGE      ("Hello, I'm your child.")  /* 子プロセスから親プロセスへ送信する文字列 */
#define CHILD_SEND_MESSAGE_BYTE (sizeof(CHILD_SEND_MESSAGE)) /* 上記の送信バイト数（終端のヌル文字を含む） */
#define PARENT_SEND_MESSAGE     ("Hello, I'm your parent.") /* 親プロセスから子プロセスへ送信する文字列 */
#define PARENT_SEND_MESSAGE_BYTE (sizeof(PARENT_SEND_MESSAGE)) /* 上記の送信バイト数（終端のヌル文字を含む） */

/*==================================================*
 * バッファサイズ
 *==================================================*/
#define BUFFER_SIZE             (100)   /* バッファサイズ */

/*==================================================*
 * 関数プロトタイプ宣言
 *==================================================*/
VD vd_ChildProcess(S4 *ps4_childToParentFd, S4 *ps4_parentToChildFd);
S4 s4_ParentProcess(S4 *ps4_childToParentFd, S4 *ps4_parentToChildFd);

/*==================================================*
 * @brief   2本のパイプを生成した後、forkによって子プロセスを生成し、
 *          親プロセス処理または子プロセス処理を呼び出す
 * @param   なし
 * @retval  int  EXIT_SUCCESS  正常に終了した場合
 *               EXIT_FAILURE  pipe、forkまたは親プロセス処理でエラーが発生した場合
 * @note    子プロセスはvd_ChildProcess関数内のexitで終了するため、
 *          本関数の戻り値を返すのは親プロセスのみである
 *==================================================*/
int main(void)
{
    S4 s4_childToParentFd[2];
    S4 s4_parentToChildFd[2];
    S4 s4_pipeResult;
    S4 s4_forkResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    s4_pipeResult = pipe(s4_childToParentFd);

    if (s4_pipeResult == SYSTEM_CALL_ERROR) {
        perror("pipe");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        s4_pipeResult = pipe(s4_parentToChildFd);

        if (s4_pipeResult == SYSTEM_CALL_ERROR) {
            perror("pipe");

            (VD)close(s4_childToParentFd[0]);
            (VD)close(s4_childToParentFd[1]);

            s4_exitStatus = EXIT_FAILURE;
        } else {
            s4_forkResult = fork();

            if (s4_forkResult == SYSTEM_CALL_ERROR) {
                perror("fork");

                (VD)close(s4_childToParentFd[0]);
                (VD)close(s4_childToParentFd[1]);
                (VD)close(s4_parentToChildFd[0]);
                (VD)close(s4_parentToChildFd[1]);

                s4_exitStatus = EXIT_FAILURE;
            } else if (s4_forkResult == FORK_CHILD_PROCESS) {
                /* 子プロセスはvd_ChildProcess関数内で終了するため、本関数へは戻らない。 */
                vd_ChildProcess(s4_childToParentFd, s4_parentToChildFd);
            } else {
                s4_exitStatus = s4_ParentProcess(s4_childToParentFd, s4_parentToChildFd);
            }
        }
    }

    return s4_exitStatus;
}

/*==================================================*
 * @brief   パイプを使用して親プロセスへ文字列を送信した後、
 *          親プロセスから文字列を受信して表示し、子プロセスを終了する
 * @param[in]   S4 *ps4_childToParentFd  子プロセスから親プロセスへのパイプの
 *                                       ファイル記述子配列の先頭アドレス
 * @param[in]   S4 *ps4_parentToChildFd  親プロセスから子プロセスへのパイプの
 *                                       ファイル記述子配列の先頭アドレス
 * @retval  なし
 * @note    受信文字列が終端されていない場合はEXIT_FAILUREを終了ステータスとする。
 *          fork後の子プロセスは親プロセスの処理へ戻ってはならず、戻り値では
 *          プロセスを終了させられないため、本関数の最後にexitライブラリ関数で
 *          終了ステータスを返す。そのため本関数から呼び出し元へは戻らない。
 *==================================================*/
VD vd_ChildProcess(S4 *ps4_childToParentFd, S4 *ps4_parentToChildFd)
{
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_writeResult;
    S4 s4_readResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    /* ps4_childToParentFd[0]は子プロセスから親プロセスへのパイプの読み出しのファイル記述子を示す。
       子プロセスでは当該パイプへの書き込みのみを行い、読み出しは使用しないためクローズする。 */
    (VD)close(ps4_childToParentFd[0]);

    /* ps4_parentToChildFd[1]は親プロセスから子プロセスへのパイプの書き込みのファイル記述子を示す。
       子プロセスでは当該パイプからの読み出しのみを行い、書き込みは使用しないためクローズする。 */
    (VD)close(ps4_parentToChildFd[1]);

    s4_writeResult = write(ps4_childToParentFd[1], CHILD_SEND_MESSAGE, CHILD_SEND_MESSAGE_BYTE);

    if (s4_writeResult == SYSTEM_CALL_ERROR) {
        perror("write");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    (VD)close(ps4_childToParentFd[1]);

    s4_readResult = read(ps4_parentToChildFd[0], ch_receiveBuffer, BUFFER_SIZE);

    /* s4_readResultが0の場合にch_receiveBuffer[s4_readResult - 1]を参照すると
       範囲外アクセスとなるため、0の判定を終端の判定より先に行う。 */
    if (s4_readResult == SYSTEM_CALL_ERROR) {
        perror("read");

        s4_exitStatus = EXIT_FAILURE;
    } else if (s4_readResult == 0) {
        printf("親プロセスがパイプをクローズしました\n");

        s4_exitStatus = EXIT_FAILURE;
    } else if (ch_receiveBuffer[s4_readResult - 1] != '\0') {
        printf("受信文字列が終端されていません\n");

        s4_exitStatus = EXIT_FAILURE;
    } else {
        printf("子プロセスの受信文字列：%s\n", ch_receiveBuffer);
    }

    (VD)close(ps4_parentToChildFd[0]);

    exit(s4_exitStatus);
}

/*==================================================*
 * @brief   子プロセスから文字列を受信して表示した後、子プロセスへ文字列を送信し、
 *          子プロセスの終了を待ち、終了状態を表示する
 * @param[in]   S4 *ps4_childToParentFd  子プロセスから親プロセスへのパイプの
 *                                       ファイル記述子配列の先頭アドレス
 * @param[in]   S4 *ps4_parentToChildFd  親プロセスから子プロセスへのパイプの
 *                                       ファイル記述子配列の先頭アドレス
 * @retval  S4  EXIT_SUCCESS  文字列の受信および送信、子プロセスの状態取得に成功した場合
 *              EXIT_FAILURE  read、writeまたはwaitでエラーが発生した場合、
 *                            または未知の要因で子プロセスが終了した場合
 *==================================================*/
S4 s4_ParentProcess(S4 *ps4_childToParentFd, S4 *ps4_parentToChildFd)
{
    char ch_receiveBuffer[BUFFER_SIZE];
    S4 s4_childStatus;
    S4 s4_readResult;
    S4 s4_writeResult;
    S4 s4_waitResult;
    S4 s4_returnValue = EXIT_SUCCESS;

    /* ps4_childToParentFd[1]は子プロセスから親プロセスへのパイプの書き込みのファイル記述子を示す。
       親プロセスでは当該パイプからの読み出しのみを行い、書き込みは使用しないためクローズする。 */
    (VD)close(ps4_childToParentFd[1]);

    /* ps4_parentToChildFd[0]は親プロセスから子プロセスへのパイプの読み出しのファイル記述子を示す。
       親プロセスでは当該パイプへの書き込みのみを行い、読み出しは使用しないためクローズする。 */
    (VD)close(ps4_parentToChildFd[0]);

    s4_readResult = read(ps4_childToParentFd[0], ch_receiveBuffer, BUFFER_SIZE);

    /* s4_readResultが0の場合にch_receiveBuffer[s4_readResult - 1]を参照すると
       範囲外アクセスとなるため、0の判定を終端の判定より先に行う。 */
    if (s4_readResult == SYSTEM_CALL_ERROR) {
        perror("read");

        s4_returnValue = EXIT_FAILURE;
    } else if (s4_readResult == 0) {
        printf("子プロセスがパイプをクローズしました\n");

        s4_returnValue = EXIT_FAILURE;
    } else if (ch_receiveBuffer[s4_readResult - 1] != '\0') {
        printf("受信文字列が終端されていません\n");

        s4_returnValue = EXIT_FAILURE;
    } else {
        printf("親プロセスの受信文字列：%s\n", ch_receiveBuffer);
    }

    (VD)close(ps4_childToParentFd[0]);

    s4_writeResult = write(ps4_parentToChildFd[1], PARENT_SEND_MESSAGE, PARENT_SEND_MESSAGE_BYTE);

    if (s4_writeResult == SYSTEM_CALL_ERROR) {
        perror("write");

        s4_returnValue = EXIT_FAILURE;
    } else {
        /* DO NOTHING */
    }

    (VD)close(ps4_parentToChildFd[1]);

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
            printf("未知の要因で終了\n");

            s4_returnValue = EXIT_FAILURE;
        }
    }

    return s4_returnValue;
}
