/*==================================================*
 * @file      task11_Rev_0_1.c
 * @brief     execvによるコマンド実行および終了状態確認
 * @version   0.1
 * @date      2026.07.23
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
#define FORK_CHILD_PROCESS  (0)         /* 子プロセスに返される値 */

/*==================================================*
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR   (-1)        /* エラー発生時の戻り値 */

/*==================================================*
 * lsコマンド実行情報
 *==================================================*/
#define LS_COMMAND_PATH     ("/usr/bin/ls") /* lsコマンドのパス */
#define LS_COMMAND_NAME     ("ls")      /* コマンド名 */
#define LS_COMMAND_OPTION   ("-l")      /* コマンドオプション */
#define LS_COMMAND_TARGET   ("/home/")  /* 表示対象のパス */

/*==================================================*
 * 関数プロトタイプ宣言
 *==================================================*/
S4 s4_ChildProcess(VD);
S4 s4_ParentProcess(VD);

/*==================================================*
 * @brief   子プロセスを生成し、親プロセス処理または子プロセス処理を呼び出す
 * @param   なし
 * @retval  なし
 * @note    exit関数によってプロセスを終了する
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
        s4_exitStatus = s4_ParentProcess();
    }

    exit(s4_exitStatus);
}

/*==================================================*
 * @brief   execvを使用してls -l /home/を実行する
 * @param   なし
 * @retval  S4  EXIT_FAILURE execvに失敗した場合
 *==================================================*/
S4 s4_ChildProcess(VD)
{
    char *pch_cmdArg[] = {
        LS_COMMAND_NAME,
        LS_COMMAND_OPTION,
        LS_COMMAND_TARGET,
        NULL
    };

    execv(LS_COMMAND_PATH, pch_cmdArg);

    /* execvが成功した場合は子プロセスがlsに置き換わり、本関数には戻らないため、以降は失敗時のみ実行される。*/
    perror("execv");

    return EXIT_FAILURE;
}

/*==================================================*
 * @brief   子プロセスの終了を待ち、
 *          子プロセスの終了状態を表示する
 * @param   なし
 * @retval  S4  EXIT_SUCCESS 終了状態を判定した場合
 *              EXIT_FAILURE waitに失敗した場合、または
 *                           終了状態を判定できなかった場合
 *==================================================*/
S4 s4_ParentProcess(VD)
{
    S4 s4_childStatus;
    S4 s4_waitResult;
    S4 s4_returnValue = EXIT_SUCCESS;

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