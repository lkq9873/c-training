/*==================================================*
 * @file      task10_Rev_0_1.c
 * @brief     execlによるコマンド実行および終了状態確認
 * @version   0.1
 * @date      2026.07.22
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
 * システムコール戻り値
 *==================================================*/
#define SYSTEM_CALL_ERROR   (-1)      /* システムコールでエラーが発生した場合の戻り値 */

/*==================================================*
 * lsコマンド実行情報
 *==================================================*/
#define LS_COMMAND_PATH        ("/usr/bin/ls")   /* execlで実行するlsコマンドのパス */
#define LS_COMMAND_NAME        ("ls")        /* 実行するコマンド名 */
#define LS_COMMAND_OPTION      ("-l")        /* コマンドのオプション */
#define LS_COMMAND_TARGET      ("/home/")    /* lsコマンドの表示対象のパス */

/*==================================================*
 * 関数プロトタイプ宣言
 *==================================================*/
S4 s4_ChildProcess(VD);
S4 s4_ParentProcess(VD);

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
        s4_exitStatus = s4_ParentProcess();
    }

    exit(s4_exitStatus);
}

/*==================================================*
 * @brief   execlを使用してls -l /home/を実行する
 * @param   なし
 * @retval  S4  EXIT_FAILURE execlに失敗した場合
 *==================================================*/
S4 s4_ChildProcess(void)
{
    execl(
        LS_COMMAND_PATH,
        LS_COMMAND_NAME,
        LS_COMMAND_OPTION,
        LS_COMMAND_TARGET,
        (char *)NULL
    );

    perror("execl");

    return EXIT_FAILURE;
}

/*==================================================*
 * @brief   子プロセスの終了を待ち、子プロセスの終了状態を表示する
 * @param   S4　s4_childProcessId  子プロセスのプロセスID
 * @retval  S4  EXIT_SUCCESS 終了状態を取得した場合
 *              EXIT_FAILURE waitでエラーが発生した場合
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