/*==================================================*
 * @file      task9_Rev_0_1.c
 * @brief     子プロセスの生成および終了状態確認
 * @version   0.1
 * @date      2026.07.21
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
#define FORK_CHILD_PROCESS  (0)       /* 子プロセスに返される値 */

/*==================================================*
 * 子プロセス情報
 *==================================================*/
#define CHILD_EXIT_STATUS   (0xAB)    /* 子プロセスが終了時に返す終了ステータス*/
#define CHILD_WAIT_SECOND   (10)     /* 子プロセス待機時間（秒） */

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR   (-1)      /* システムコールでエラーが発生した場合の戻り値 */

/*==================================================*
 * 関数プロトタイプ宣言
 *==================================================*/
S4 s4_ChildProcess(VD);
S4 s4_ParentProcess(S4 s4_childProcessId);

/*==================================================*
 * @brief   子プロセスを生成し、親プロセス処理または子プロセス処理を呼び出す
 * @param   なし
 * @retval  なし
 *==================================================*/
int main(void)
{
    S4 s4_forkResult;
    S4 s4_exitStatus = EXIT_SUCCESS;

    s4_forkResult = fork();

    if (s4_forkResult == SYSTEM_CALL_ERROR) {
        perror("fork");
        s4_exitStatus = EXIT_FAILURE;
    } else if (s4_forkResult == FORK_CHILD_PROCESS) {
        s4_exitStatus = s4_ChildProcess();
    } else {
        s4_exitStatus = s4_ParentProcess(s4_forkResult);
    }

    exit(s4_exitStatus);
}

/*==================================================*
 * @brief   子プロセスをCHILD_WAIT_SECOND秒待機させ、CHILD_EXIT_STATUSを返す
 * @param   なし
 * @retval  S4  CHILD_EXIT_STATUS   正常終了ステータス
 *==================================================*/
S4 s4_ChildProcess(VD)
{
    printf("子プロセス：%u秒間待機\n", CHILD_WAIT_SECOND);
    
    sleep(CHILD_WAIT_SECOND);

    return CHILD_EXIT_STATUS;
}

/*==================================================*
 * @brief   子プロセスの終了を待ち、子プロセスの終了状態を表示する
 * @param   S4　s4_childProcessId  子プロセスのプロセスID
 * @retval  S4  EXIT_SUCCESS 終了状態を取得した場合
 *              EXIT_FAILURE waitでエラーが発生した場合
 *==================================================*/
S4 s4_ParentProcess(S4 s4_childProcessId)
{
    S4 s4_childStatus;
    S4 s4_waitResult;
    S4 s4_returnValue = EXIT_SUCCESS;

    printf("親プロセス：子プロセスIDは%d\n", s4_childProcessId);

    s4_waitResult = wait(&s4_childStatus);

    if (s4_waitResult == SYSTEM_CALL_ERROR) {
        perror("wait");
        s4_returnValue = EXIT_FAILURE;
    } else {
        if (WIFEXITED(s4_childStatus)) {

            printf("子プロセスは正常に終了\n"
                "終了ステータス：0x%X\n",
                WEXITSTATUS(s4_childStatus));

        } else if (WIFSIGNALED(s4_childStatus)) {

            printf("子プロセスはシグナルによって終了\n"
                "終了シグナル番号：%d\n",
                WTERMSIG(s4_childStatus));
            
        } else {
            printf("未知の要因で終了\n");
        }
    }

    return s4_returnValue;
}