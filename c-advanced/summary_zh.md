# C言語 発展編 研修 — 教材总结 & 课题解题思路（中文）

> 对应教材：`textbook.md`（C言語プログラミング実践編 Rev. 2.0.5）
> 对应规约：`../docs/c_coding_standards.md`、`../docs/structured_programming.md`
> 覆盖课题：課題 3 〜 課題 16

---

## 目录

- [第一部分：教材内容总结](#第一部分教材内容总结)
  - [1. UNIX 的构成](#1-unix-的构成)
  - [2. 系统调用与库函数](#2-系统调用与库函数)
  - [3. 文件](#3-文件)
  - [4. 文件描述符与文件指针](#4-文件描述符与文件指针)
  - [5. 文件输入输出 API](#5-文件输入输出-api)
  - [6. 进程](#6-进程)
  - [7. 进程的执行 — exec 系](#7-进程的执行--exec-系)
  - [8. 进程的生成 — fork / wait / waitpid](#8-进程的生成--fork--wait--waitpid)
  - [9. 信号](#9-信号)
  - [10. 进程间通信 — 管道](#10-进程间通信--管道)
  - [11. 进程间通信 — 套接字](#11-进程间通信--套接字)
- [第二部分：课题解题思路](#第二部分课题解题思路)
- [第三部分：贯穿全部课题的规约要点](#第三部分贯穿全部课题的规约要点)
- [第四部分：讲师 review 指摘与对应](#第四部分讲师-review-指摘与对应)
- [附录：速查表](#附录速查表)

---

# 第一部分：教材内容总结

## 1. UNIX 的构成

UNIX 由五个要素构成：

| 要素 | 说明 |
|---|---|
| **カーネル**（kernel） | 直接控制硬件的 OS 核心。提供文件、进程、进程间通信等基本机能 |
| **シェル**（shell） | 「壳」。解析用户输入并执行程序，也叫 command interpreter |
| **コマンド群** | 内部命令（shell 自己执行，`help` 可列出）/ 外部命令（`/usr/bin/` 下的独立程序） |
| **システムコール** | 调用内核机能的入口 |
| **ライブラリー関数** | 库函数，内部可能调用系统调用，也可能自我完结 |

分层结构（表1）：

```
ソフトウェア ┬ ユーザー層      ← 应用程序、用户自制软件
             ├ ユーティリティ層 ← 编译器、编辑器、mail 命令
             ├ システム層      ← shell、窗口系统、管理命令
             └ カーネル層      ← 内存/文件/IO/进程管理
ハードウェア              ← CPU、内存、硬盘、外设
```

一般把「カーネル層〜ユーティリティ層」称为 OS；狭义只把カーネル層叫 OS。

---

## 2. 系统调用与库函数

### 2.1 怎么区分

**看 man 手册的章节号**：

```bash
man 3 printf   # → PRINTF(3)  ... (3) = ライブラリー関数
man 2 write    # → WRITE(2)   ... (2) = システムコール
```

C 语言里两者都写成函数调用的形式，肉眼分不出来，必须查手册。

### 2.2 关系

标准输入输出库函数（printf/scanf/fopen/fgets/fread/fseek 系等，表2）内部会层层调用更低层的函数，**最低层的就是系统调用**。实现标准输入输出库所用到的系统调用是：

```
open, creat, close, read, write, lseek, unlink
```

例：`printf` 最终会调用 `write`。而 `write` 本身也**不**实现「往屏幕显示 / 往硬盘写 / 往打印机打印 / 往网络发送」这些功能 —— 它只是**调用内核里实现这些功能的软件**，然后把结果返回。

> **系统调用的本质**：システム層以上のソフトウェア 与 カーネル層 之间的信息传递通道。

### 2.3 运行机制

| 概念 | 说明 |
|---|---|
| ユーザーモード / ユーザー空間 | 用户程序代码运行时 |
| カーネルモード / カーネル空間 | 内核程序代码运行时 |

调用流程：

1. 用户程序（直接或经由库函数）调用系统调用
2. 系统调用内部执行**トラップ命令**（trap，软件中断）
3. 切换到内核模式，系统调用把自己的**システムコール番号**通知内核（例：read=3, write=4）
4. 内核按编号执行对应的内核程序
5. 结果返回，切回用户模式

**中断的两种**：

- **ハードウェア割込み**（interrupt）— 定时器、外设发出
- **ソフトウェア割込み**（trap）— trap 命令执行时发出

### 2.4 错误处理 — errno 与 perror

系统调用出错时：

- **返回值 = -1**
- **外部变量 `errno` 被设置为错误编号**（要用 `errno` 需 `#include <errno.h>`）

```c
#include <stdio.h>
void perror(const char *s);
```

`perror` 把「直前のシステムコール」的错误信息输出到**标准错误输出**，格式为：

```
<引数の文字列>: <エラーメッセージ>
```

即先输出参数字符串，再输出冒号+空格 `": "`，最后输出错误消息。
（历史上用外部数组 `sys_errlist`，现已废弃，现在内部调用 `strerror`。）

---

## 3. 文件

### 3.1 文件的种类

UNIX 的文件全部组成**一棵树**，分三种：

| 种类 | 说明 |
|---|---|
| **ディレクトリーファイル** | 管理其正下方全部文件的文件。树的分叉点。最上面的叫ルートディレクトリー |
| **一般ファイル** | 程序、数据、文档。树的叶子 |
| **デバイスファイル**（特殊/スペシャルファイル） | 对应外设、内存等每一个装置 |

デバイスファイル再分两类：

- **キャラクターデバイス** — 按字符单位输入输出（终端、打印机）
- **ブロックデバイス** — 按块单位处理（磁盘装置）

> **UNIX 的一大特征**：设备文件和普通文件可以用同样的输入输出命令（open/close/read/write/ioctl）来处理。

`ls -l` 输出的**第 1 个字符**表示文件种别（表3）：

| 字符 | 意义 |
|---|---|
| `d` | ディレクトリーファイル |
| `l` | 符号链接 |
| `b` | ブロック型デバイスファイル |
| `c` | キャラクター型デバイスファイル |
| `p` | **名前つきパイプ**（← 課題14 会产生） |
| `s` | **ソケット**（← 課題15 会产生） |
| `-` | 一般ファイル |

### 3.2 路径

- **絶対パス名** — 以 `/` 开头，从根目录开始
- **相対パス名** — 不以 `/` 开头，从当前目录开始
- `..` = 上一级、`.` = 当前、`~` = home

### 3.3 保护模式

`ls -l` 第 2〜10 个字符：

```
- rwx rwx rwx
  ↑   ↑   ↑
  所有者 グループ 他の利用者
```

| 位置 | 字符 | 意义（普通文件 / 目录） |
|---|---|---|
| 最左 | `r` | 可读 / 可用 ls 查看目录内容 |
| 中央 | `w` | 可写 / 可在目录内创建·删除文件 |
| 最右 | `x` | 可执行 / 可 cd 进入该目录 |
| — | `-` | 不允许 |

第 4/7/10 个字符还可能出现 `s`/`S`/`t`/`T`（表5）：

| 位置 | 字符 | 意义 |
|---|---|---|
| 第4 | `s` / `S` | セットユーザーID ビット，有/无执行权 |
| 第7 | `s` / `S` | セットグループID ビット，有/无执行权 |
| 第10 | `t` / `T` | sticky ビット，有/无执行权 |

- **セットユーザーIDビット**：执行期间进程的**実効ユーザーID** 变成该文件所有者（例：`su`、`sudo` 就设了这个位）
- **セットグループIDビット**：设在目录上时，目录内新建的文件继承该组
- **sticky ビット**：设在目录上时，目录内的文件**只有所有者能改·能删**（例：`/tmp` 是 `drwxrwxrwt`）

### 3.4 文件系统的结构

磁盘被分成若干**パーティション**，每个分区通常放 1 个文件系统。文件系统由 4 个区域构成（图1）：

```
┌──────────────┐
│ ブートブロック  │ ← 第1块。系统盘时放启动程序（bootstrap / bootloader）
├──────────────┤
│ スーパーブロック │ ← 文件系统大小、空块信息、i节点信息、修改标志
├──────────────┤
│ i ノードリスト  │ ← 多个 i 节点
├──────────────┤
│ データブロック  │ ← 一般文件的数据 + 目录文件的数据
└──────────────┘
```

**スーパーブロック** 的信息为了高速处理会同时存在于内存，内核定期写回磁盘以保持一致性 —— 实现它的是 `sync` 命令和 `sync` 系统调用。

**i ノード** 里存的信息（表7）：i节点号、文件种类、**リンク数**、所有者 UID/GID、访问权限、3 个时间（最终访问/最终变更/状态变更）、文件大小、指向数据块的指针、设备号（メジャー番号+マイナー番号）。

> **重点**：i 节点里**不含文件名，也不含文件数据**。
> 文件名在**ディレクトリーファイル**里，数据在**データブロック**里。
> i 节点与文件实体是 1 对 1。

**ディレクトリーファイル的数据** = 「文件名 + i 节点号」的配对集合。

**从文件名找到文件实体的流程**：

1. 文件名 → i 节点号
2. i 节点号 → i 节点
3. i 节点 → 数据块指针
4. 顺着指针 → 文件实体

> 根目录的 i 节点号固定为 **2**。所以 `/etc/group` 的查找是：i节点2 → 根目录数据 → 找到 "etc" 的i节点号 → etc 的数据 → 找到 "group" 的i节点号 → …

### 3.5 链接

文件名与文件实体的对应关系叫**リンク**。一个文件实体可以有多个文件名。

| | ハードリンク | ソフトリンク（シンボリックリンク） |
|---|---|---|
| 创建 | `ln foo bar` | `ln -s foo bar` |
| i节点号 | **相同** | **不同**（链接本身也有自己的 i 节点，里面存链接信息） |
| 对目录 | **不能**创建 | 可以 |
| 跨文件系统 | **不能** | 可以 |
| 删除行为 | 链接数减 1，减到 0 才释放实体 | 删除原文件后链接失效 |

创建普通文件时 i 节点的链接数为 1。`ls -l` 保护模式右边的数字就是链接数，`ls -i` 显示 i 节点号。

### 3.6 设备文件与デバイススイッチ

- **デバイス番号** = メジャー番号 + マイナー番号
- デバイスファイル ↔ デバイス番号 是 1 对 1
- デバイス番号 ↔ 装置 是 1 对 1
- **メジャー番号 ↔ デバイスドライバー 是 1 对 1**
- マイナー番号作为参数传给驱动的各个函数（同一驱动控制多个装置时用来切换）

`ls -l /dev/` 里，权限字符串右边的两个数字就是「メジャー番号, マイナー番号」。

从文件描述符到驱动的流程：fd → ファイル表 → i节点号 → i节点 → メジャー番号 → **デバイス切り替え表**（表8/表9）→ 驱动函数。

### 3.7 标准输入输出与重定向

| | fd | 默认 |
|---|---|---|
| 標準入力 | 0 | 键盘 |
| 標準出力 | 1 | 显示器 |
| 標準エラー出力 | 2 | 显示器 |

切换是靠 `dup` / `dup2` 系统调用实现的。shell 的**リダイレクション機能**（表10）：

| 写法 | 意义 |
|---|---|
| `prog < ifile` | 输入源 → ifile（`<` 与 `0<` 完全相同） |
| `prog > ofile` | 标准输出 → ofile，**覆盖**（`>` 与 `1>` 完全相同） |
| `prog >> ofile` | 标准输出 → ofile，**追加** |
| `prog 2> efile` | 标准错误输出 → efile，覆盖 |
| `prog 2>> efile` | 标准错误输出 → efile，追加 |
| `prog > ofile 2>&1` | 标准输出 → ofile，标准错误输出 → 与标准输出同一处 |
| `prog0 \| prog1` | **パイプ機能**：prog0 的输出作为 prog1 的输入 |
| `prog0 \| tee outfile` | 标准输出同时也写到 outfile |

> **注意**：`0<`、`1>`、`2>`、`2>&1` 这些记号内部**不能有空格**。

---

## 4. 文件描述符与文件指针

`/usr/include/libio.h` 里的 `struct _IO_FILE` 结构体在**用户空间**管理文件信息，以链表相连（成员 `struct _IO_FILE *_chain`）。这个结构体叫**利用者ファイル記述子表**，每个运行中的程序各有一份。

| | 定义 |
|---|---|
| **ファイル記述子**（fd） | 表示在利用者ファイル記述子表的链表中排第几个的**整数**，值存在成员 `int _fileno` 里 |
| **ファイルポインター**（FILE*） | 分配给该文件的表项的**首地址**，即 `struct _IO_FILE *` |

```c
typedef struct _IO_FILE FILE;   /* /usr/include/bits/types/FILE.h */
FILE *fp;
```

- 标准输入输出的 fd：`0` / `1` / `2`
- 标准输入输出的 FILE*：`stdin` / `stdout` / `stderr`
- **从 FILE* 取 fd**：`fileno(fp)`

---

## 5. 文件输入输出 API

### 5.1 open

```c
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int open(const char *pathname, int flags[, mode_t mode]);
```

- 实际原型是 `int open(const char *__file, int __oflag, ...);` —— **第3参数是可变长参数**
- 第3参数 `mode` **只在新建文件时使用**，其他情况可省略。`mode_t` 当成 `int` 理解即可
- **失败返回 -1** 并设置 errno；**成功返回文件描述符**

`flags` = 表11 三选一 + 表12 任意个的**逻辑或**：

| 表11（必选1个） | 意义 |
|---|---|
| `O_RDONLY` | 只读 |
| `O_WRONLY` | 只写 |
| `O_RDWR` | 读写 |

| 表12（可选） | 意义 |
|---|---|
| `O_APPEND` | 追加模式 |
| `O_CREAT` | 创建文件 |
| `O_TRUNC` | 文件已存在时把大小截为 0 |
| `O_NONBLOCK` / `O_NDELAY` | 非阻塞模式 |
| `O_ASYNC` | 文件数据与状态更新前不返回 |

`mode` 用表13 的宏（`S_IRUSR`=00400、`S_IWUSR`=00200 …），也可以直接写八进制如 `0660`（= `rw-rw----`）。

### 5.2 close / write / read / unlink

```c
#include <unistd.h>
int     close(int fd);                              /* 成功 0，失败 -1 */
ssize_t write(int fd, const void *buf, size_t count); /* 成功=实际写入字节数，失败 -1 */
ssize_t read (int fd, void *buf, size_t count);       /* 成功=实际读出字节数，失败 -1 */
int     unlink(const char *pathname);               /* 成功 0，失败 -1 */
```

---

## 6. 进程

### 6.1 什么是进程

**プログラム** = 把代码和数据收进文件里的可执行文件（多为 ELF 格式），保护模式为可执行。
**プロセス** = 执行程序时所需的**软件资源（含程序本身）+ 硬件资源**的总和。

### 6.2 リング

一个进程有两个局面：**ユーザープロセス**（ユーザーリング）和 **カーネルプロセス**（カーネルリング）。

- ユーザー → カーネル 的切换：系统调用的调用、外部中断
- カーネル → ユーザー 的切换：内核程序返回

这些切换叫**リング切り替え**。

### 6.3 进程的树结构

- `fork` 调用者 = **親プロセス**，新生成的 = **子プロセス**
- fork 生成的子进程**除了进程 ID 以外特征完全一样**
- 让子进程做自己独有的动作 → 用 **exec 系**库函数
- **プロセス 0** 是系统启动时生成的特殊进程，它 fork 出**プロセス 1（init）**后自己成为调度器
- **init 是系统中所有进程的祖先**

### 6.4 进程的状态

| 状态 | 说明 |
|---|---|
| **実行状態** | 正在 CPU 上执行。CPU 占用时间到 → 実行可能状態；进入 IO 等待 → 休眠状態；`_exit` → ゾンビ状態 |
| **実行可能状態** | 等待调度器分配 CPU。被重新调度 → 実行状態 |
| **休眠状態** | 等待 IO 结束等，无法继续。等待原因解除 → 実行可能状態 |
| **生成状態** | fork 刚生成。→ 実行可能状態 |
| **ゾンビ状態** | `_exit` 结束后的状态 |

### 6.5 exit / _exit / _Exit

```c
#include <stdlib.h>
void exit(int status);    /* ライブラリー関数 */
void _Exit(int status);   /* システムコール */

#include <unistd.h>
void _exit(int status);   /* システムコール */
```

**`exit` 与 `_exit` 的区别**：

- `exit`（库函数）会调用 `atexit` / `on_exit` 注册的函数
- `_exit`（系统调用）**不会**调用它们
- `_exit` 与 `_Exit` 等价

```c
int atexit(void (*func)(void));
int on_exit(void (*func)(int, void *), void *arg);
```

- 两者都是注册「进程**正常结束**时被调用的函数」
- **「正常结束」= `exit` 函数，或 main 函数中的 `return` 语句**（← 这条对理解「main 里 return 还是 exit」很关键）
- 注册成功返回 0

### 6.6 进程 ID / 用户 ID / 组 ID

| 概念 | 说明 |
|---|---|
| **プロセスID** | 进程生成时分配的唯一编号 |
| **プロセスグループID** | 相关进程可归为一组；该 ID = 组长进程（プロセスグループリーダー）的进程 ID |
| **実ユーザーID** | 进程所有者（执行该命令的用户）的 UID。**子进程继承父进程的实 UID** |
| **実効ユーザーID** | 用于决定新建文件的所有者、检查访问权限。通常 = 实 UID；但可执行文件设了 set-user-ID 位时，执行期间变成该文件的 UID |
| **実グループID / 実効グループID** | 同理 |

```c
#include <sys/types.h>
#include <unistd.h>
uid_t getuid(void);   gid_t getgid(void);    /* 实 ID，必定成功 */
uid_t geteuid(void);  gid_t getegid(void);   /* 实效 ID，必定成功 */
int setuid(uid_t);    int setgid(gid_t);     /* 成功 0，失败 -1 */
int seteuid(uid_t);   int setegid(gid_t);
```

`uid_t` / `gid_t` 当成 `int` 理解即可。

调度：実行可能状態中**优先度最高**的被选中。优先度分 0〜127 级，**数字越小优先度越高**。

---

## 7. 进程的执行 — exec 系

exec 系库函数接收 3 类参数：

1. 新程序的文件名或路径名
2. 新程序的运行时参数信息（与 main 的 `argv` 同样形式）
3. 需要的话，传给新程序的环境变量信息

**名字里的字母的含义**（表14）：

| 字母 | 意义 |
|---|---|
| `l` | 参数用**逐个指针** `arg0, arg1, ..., argn` 给出，**最后必须是 NULL 指针** |
| `v` | 参数用**指针数组的首地址** `argv` 给出，**数组最后一个元素必须是 NULL 指针** |
| `e` | 给出环境变量信息 |
| `p` | 从环境变量 `PATH` 指定的目录里**搜索**文件名。**不带 `p` 时必须写路径名** |

```c
#include <unistd.h>
int execl (const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg, ..., char const * const envp[]);
int execv (const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execve(const char *file, char *const argv[], char *const envp[]);
```

**要点**：

- 6 个函数内部都是靠 `execve` 系统调用实现的
- **成功时原程序被替换掉，绝不返回**
- **程序被替换但进程 ID 不变**
- **失败时才返回调用元，返回值 -1**，并设置 errno

> 所以调用 exec 之后紧跟的代码 = 「只有失败时才会执行的错误处理」。

---

## 8. 进程的生成 — fork / wait / waitpid

### 8.1 fork

```c
#include <sys/types.h>
#include <unistd.h>
pid_t fork(void);       /* pid_t 与 int 同义 */
```

子进程继承父进程的程序，**只有进程 ID 不同**，父子都从同一个 fork 的返回处继续执行。区分靠**返回值**（表15）：

| 返回值 | 意义 |
|---|---|
| **0** | 返回给**子进程**。这里写子进程要做的事 |
| **1 以上** | 返回给**父进程**，值 = **子进程的进程 ID**。这里写父进程要做的事 |
| **-1** | 返回给父进程。出错，errno 被设置。这里写错误处理 |

教材给的骨架：

```c
if ((pid = fork()) == 0) {
    /* 子プロセスの処理 */
} else if (pid >= 1) {
    /* 親プロセスの処理 */
} else {
    /* エラー処理 */
}
```

### 8.2 wait

```c
#include <sys/types.h>
#include <sys/wait.h>
pid_t wait(int *status);
```

- 子进程**已经结束** → 立刻返回
- 子进程**还没结束** → **阻塞（block）**直到结束
- 成功返回子进程的 PID，失败返回 -1 并设置 errno

子进程的结束有两种：**`_exit` 调用导致的结束** / **信号导致的结束**。

判定宏（表16）：

| 宏 | 意义 |
|---|---|
| `WIFEXITED(status)` | 正常结束（`_exit` 导致）时为真 |
| `WIFSIGNALED(status)` | 信号导致结束时为真 |

**状态信息的构造**（表17）：

| 情况 | 内容 |
|---|---|
| `_exit` 导致结束 | 上位 8 bit = 传给 `_exit` 的参数的**低 8 位**；下位 8 bit = 0。用 **`WEXITSTATUS(status)`** 取出 |
| 信号导致结束 | 结束原因的信号编号。用 **`WTERMSIG(status)`** 取出 |

### 8.3 waitpid

```c
#include <sys/types.h>
#include <sys/wait.h>
pid_t waitpid(pid_t pid, int *status, int options);
```

等待子进程的**状态变化**（结束 / 因信号停止 / 因信号再开）。

**第1参数 pid**：

| 值 | 意义 |
|---|---|
| < -1 | 等待进程组 ID 等于 pid 绝对值的任一子进程 |
| **-1** | 等待任意一个子进程 |
| 0 | 等待与调用者同一进程组的子进程 |
| > 0 | 等待进程 ID 等于 pid 的子进程 |

**第3参数 options**（0 个以上的逻辑或）：

| 宏 | 意义 |
|---|---|
| `WNOHANG` | 没有状态变化的子进程时立即返回（不阻塞） |
| `WUNTRACED` | 子进程停止时也返回 |
| `WCONTINUED` | 停止的子进程因 SIGCONT 再开时也返回 |

> **`wait(&status)` 等价于 `waitpid(-1, &status, 0)`**

追加的判定宏（表18）：`WIFSTOPPED`（因信号停止，需指定 WUNTRACED）、`WIFCONTINUED`（因 SIGCONT 再开）、取停止信号号用 `WSTOPSIG`。
停止时的状态信息：上位 8bit = 停止原因的信号编号，下位 8bit = `0x7f`（表19）。

### 8.4 前台与后台

- **フォアグラウンドプロセス** — 处于接受终端输入状态的进程。能收到键盘 Ctrl-C 发来的 SIGINT
- **バックグラウンドプロセス** — 不是前台的

---

## 9. 信号

信号 = **对进程的软件中断**，是从外部控制运行中进程、或在进程间取得同步的手段。每个信号有编号，宏定义在 `signal.h`。

**信号的分类**：

- 内核发出的
- 进程发给自己的
- 某进程发给另一进程的
- 代替用户发出的（按 Ctrl+C、Ctrl+Z 时发的）

```c
#include <signal.h>
void (*signal(int signum, void (*handler)(int)))(int);
```

**这个原型怎么读**：

```
signal is function
  1st arg: int
  2nd arg: pointer to function (arg: int, returning void)
  returning pointer to function (arg: int, returning void)
```

- 出错返回 `SIG_ERR`（定义如 `#define SIG_ERR (void (*)(int)) -1`）并设置 errno
- 正常返回**之前的 handler 值**（函数指针）
- 第2参数可以指定：`SIG_DFL`（恢复默认）、`SIG_IGN`（忽略）
- **`SIGKILL` 和 `SIGSTOP` 不能指定** —— 否则会产生无法停止的进程

**主要信号**（表20 摘录）：

| 宏 | 号 | 意义 |
|---|---|---|
| `SIGHUP` | 1 | 终端断线（现在多为伪终端关闭时发给该终端启动的进程组） |
| **`SIGINT`** | **2** | **终端按中断键（通常 Ctrl+C）** ← 課題9 用这个 |
| `SIGQUIT` | 3 | 终端的终了键（Ctrl+\） |
| `SIGILL` | 4 | 跳到非指令的内存区域等 |
| `SIGABRT` | 6 | 进程自己执行 abort。可捕获、不可阻塞，从 handler 返回后进程仍被终止 |
| `SIGBUS` | 7 | 地址边界不正、不存在的物理地址等 |
| `SIGFPE` | 8 | 浮点运算的除零/溢出，整数溢出等 |
| **`SIGKILL`** | **9** | **不能捕获也不能忽略**。僵尸进程收到也不会消失 |
| `SIGSEGV` | 11 | 内存访问引起的缺页 |
| `SIGPIPE` | 13 | 向没有读端的管道写入 |
| `SIGALRM` | 14 | alarm 设置的定时器超时 |
| `SIGTERM` | 15 | kill 命令默认发的信号。**可捕获可忽略** |
| `SIGCHLD` | 17 | 子进程状态变化时发生 |
| `SIGCONT` | 18 | 停止中则再开 |
| **`SIGSTOP`** | **19** | **不能捕获也不能忽略**。用 SIGCONT 再开 |
| `SIGTSTP` | 20 | 停止前台进程组（通常 Ctrl+Z） |

**用 kill 命令发信号**：

```bash
kill -<signal> <pid>     # <signal> = 去掉 "SIG" 前缀的宏名
kill <pid>               # 省略时等同 -TERM
```

信号处理函数（シグナルハンドラー）处理结束后，进程从信号发生的位置继续执行。

---

## 10. 进程间通信 — 管道

### 10.0 通信方式的分类

| 分类 | 说明 |
|---|---|
| **ブロッキング通信** | 通信完成前程序执行停止 |
| **ノンブロッキング通信** | 不等通信完成，并行执行其他处理 |

管道的通信范围**限于 1 个 UNIX 系统内**；套接字则可以跨越互联网上不同的 UNIX 系统。

### 10.1 无名管道（無名パイプ）

**パイプ** = 内存中设置的缓冲区域，有 1 个写入口和 1 个读出口。UNIX 把它当成文件处理，用文件描述符访问。

```c
#include <unistd.h>
int pipe(int fd[2]);
```

- 出错返回 -1 并设置 errno；成功返回 0
- **`fd[0]` = 读出模式的文件描述符**
- **`fd[1]` = 写入模式的文件描述符**

> **记忆法**：0 像嘴巴（读进来），1 像笔（写出去）。

**通信双方必须能共享文件描述符** → 最常用的办法是让两个进程成为**父子关系**。fork 时父进程的全部信息（含文件描述符）都会被子进程继承。

**父子间用管道通信的步骤**：

1. 父进程用 `pipe` 打开管道，得到 2 个 fd
2. `fork` 生成子进程，2 个 fd 被父子共享
3. 对共享的 fd 读写，实现通信

教材给的骨架（子 → 父 单向）：

```c
if (pipe(fd) == -1) { perror("pipe"); exit(1); }
if ((pid = fork()) == 0) {          /* 子进程 */
    close(fd[0]);                   /* 读端不用，关掉 */
    /* 用 fd[1] 发送数据 */
    close(fd[1]);
    exit(0);
} else if (pid >= 1) {              /* 父进程 */
    close(fd[1]);                   /* 写端不用，关掉 */
    /* 用 fd[0] 接收数据 */
    close(fd[0]);
    pid = wait(&status);
    ...
} else {                            /* fork 错误 */
    ...
}
```

### 10.2 双向管道（双方向パイプ）

**用 2 条管道**就能双向通信。—— 課題13。

### 10.3 命名管道（名前つきパイプ / FIFO）

| | 無名パイプ | 名前つきパイプ |
|---|---|---|
| 生存期 | 使用的进程结束就消失 | **进程消失后仍然存在** |
| 因此 | — | **不用了必须删除** |
| 通信双方 | 需要父子关系（共享 fd） | 不需要，靠路径名 |

```c
#include <sys/types.h>
#include <sys/stat.h>
int mkfifo(const char *pathname, mode_t mode);
```

- 出错 -1 并设置 errno；成功 0
- 第2参数是保护模式，指定方法与 `open` 的第3参数相同

**接收侧的处理**：

1. `mkfifo` 创建命名管道
2. `open` 打开
3. 从管道读数据
4. `close`
5. **删除命名管道**

**发送侧的处理**：

1. `open` 打开
2. 写数据
3. `close`

---

## 11. 进程间通信 — 套接字

### 11.1 基本概念

**ソケット** = 进行进程间通信的**通信端点**。是 UNIX 上使用 TCP/IP、UDP/IP 的接口。

给套接字起名字的处理叫**バインド**（bind），用 `bind` 系统调用。

**ドメイン** = 套接字名字通用的范围：

| ドメイン | 用途 | 名字用什么 | 结构体 | 头文件 |
|---|---|---|---|---|
| **UNIX ドメイン** | 同一 UNIX 系统内的进程间 | **文件路径名** | `struct sockaddr_un` | `<sys/un.h>` |
| **INET ドメイン** | 网络连接的进程间 | **IP 地址 + 端口号** | `struct sockaddr_in` | `<netinet/in.h>` |

**ソケットの種類**：

| 种类 | 对应的传输层手段 | 特征 | 协议 |
|---|---|---|---|
| **ストリームソケット** | バーチャルサーキット（コネクション型） | 建立连接→收发→解除连接。适合特定进程间持续发送大量数据 | **TCP** |
| **データグラムソケット** | データグラム（コネクションレス型） | 不建连接，逐个把数据发给对方。适合断续发送小块数据 | **UDP** |

**クライアント / サーバー**：发出服务请求的是**クライアントプロセス**，提供服务的是**サーバープロセス**。**最先向对方发起通信的是客户端**。

**服务器形态**：

| 形态 | 说明 |
|---|---|
| **反復サーバー** | 多个请求来时按顺序逐个处理。缺点：耗时的请求连续到来时，后面的请求会一直等 |
| **並行サーバー** | 收到请求时 fork 子进程去处理，服务器本身继续等下一个请求 |

**同期通信 / 非同期通信**：等对方响应再继续 / 不等对方响应就继续。

### 11.2 socket

```c
#include <sys/types.h>
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
```

- 出错 -1 并设置 errno；**成功返回套接字的文件描述符**
- `domain`：`PF_UNIX` / `PF_INET`
- `type`：`SOCK_STREAM` / `SOCK_DGRAM`
- `protocol`：**通常指定 0**，UNIX 会自动选择合适的协议
- **通信双方都要调用**

### 11.3 bind

```c
#include <sys/types.h>
#include <sys/socket.h>
int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
```

- 出错 -1；成功 0
- 第2参数的 `struct sockaddr` 是**汎用型**，实际要用特化的 `sockaddr_un` / `sockaddr_in`，传的时候**强制转换**

**UNIX ドメイン**：

```c
#include <sys/un.h>
struct sockaddr_un my_addr;
bzero((void *)&my_addr, sizeof(my_addr));   /* 教材写法。memset 亦可 */
my_addr.sun_family = PF_UNIX;               /* 或 AF_UNIX */
strcpy(my_addr.sun_path, パス名);            /* 路径名，以 '\0' 终端 */
bind(fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
```

> **重要**：UNIX ドメイン时 bind 的结果会**在文件系统上创建该路径名的文件**。
> **用完必须用 `unlink` 删除。**

**INET ドメイン**：

```c
#include <netinet/in.h>
struct sockaddr_in my_addr;
bzero((void *)&my_addr, sizeof(my_addr));
my_addr.sin_family = PF_INET;                        /* 或 AF_INET */
my_addr.sin_port = htons(<ポート番号>);               /* 转成网络字节序 */
my_addr.sin_addr.s_addr = htonl(INADDR_ANY);         /* 服务器侧：本机任意地址 */
/* 客户端侧：my_addr.sin_addr.s_addr = inet_addr("x.x.x.x"); */
bind(fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
```

**字节序转换函数**（需 `<netinet/in.h>`）：

| 函数 | 方向 | 长度 |
|---|---|---|
| `htonl` | host → network | 4 字节 |
| `htons` | host → network | 2 字节 |
| `ntohl` | network → host | 4 字节 |
| `ntohs` | network → host | 2 字节 |

> 名字最后的 `l` = long（4字节无符号整数），`s` = short（2字节无符号整数）。

`inet_addr` 把 IP 地址的**点分十进制表现**转成网络字节序，需 `<arpa/inet.h>`。

**端口号**（16 bit，0〜65535）：

| 范围 | 用途 |
|---|---|
| **〜1023** | ウェルノウン（Well-known）ポート番号，服务器进程使用 |
| **1024〜49151** | 预留 |
| **49152〜** | **可以自由使用** ← 課題16 要求用这个范围 |

### 11.4 connect / listen / accept

```c
#include <sys/types.h>
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
int listen (int sockfd, int backlog);
int accept (int sockfd, struct sockaddr *client_addr, socklen_t *addrlen);
```

| 系统调用 | 谁调用 | 作用 | 返回值 |
|---|---|---|---|
| **connect** | 客户端 | 发出连接请求。**主要用于流套接字** | 成功 0，失败 -1 |
| **listen** | 服务器 | 通知系统「这个套接字将会收到连接请求」。`backlog` = 接受几个连接请求 | 成功 0，失败 -1 |
| **accept** | 服务器 | 许可连接请求。**必须先 listen** | **成功返回新的文件描述符**，失败 -1 |

**accept 的要点**：

- **阻塞**直到有客户端发来连接请求
- 返回**新的 fd**，用它跟这个客户端通信（write/read/send/recv）
- **socket 返回的旧 fd 继续用来 listen 等待下一个连接** → 这就是**並行サーバー**的实现基础
- 第2、3参数：调用前要把第3参数设成结构体大小，返回时被改成实际收到的大小。**服务器侧不需要客户端地址信息时，第2、3参数都传 NULL 即可**

### 11.5 流套接字的收发顺序（图2）

```
クライアント                          サーバー
socket()                             socket()
bind()（省略可）                       bind()
                                     listen()
connect()  ──── 接続要求 ────→        accept()
        ←──── read()/write() ────→
close()                              close()
```

**要点**：

- **服务器侧必须 bind，客户端侧可以省略 bind**
- 连接建立后，套接字就是文件描述符，所以**可以直接用 `write` / `read` 收发**
- **服务器侧收发要用 accept 返回的 fd**
- 通信结束用 `close` 关闭

**也可以用 send / recv 代替 write / read**：

```c
#include <sys/types.h>
#include <sys/socket.h>
int send(int s, const void *buf, size_t len, int flags);
int recv(int s, const void *buf, size_t len, int flags);
```

- 出错 -1 并设置 errno；成功返回实际发送/接收的字节数
- 第4参数 `flags` **通常指定 0**

---

# 第二部分：课题解题思路

> 全部课题共通的骨架：
> **① 定义宏 → ② 定义变量 → ③ 调用 → ④ 判断返回值 → ⑤ 出错则 perror + 设 EXIT_FAILURE → ⑥ 后处理（close/unlink）→ ⑦ 返回状态**

## 課題 3 — open 失败时的 perror 与 errno

**要求**：用 `open` 打开不存在的文件制造系统调用错误，用 `perror` 输出错误消息，同时显示 errno 的值。

**思路**：

- 用 `#define TARGET_FILE_PATH ("./111/111.c")` 之类**确定不存在**的路径
- `open` 返回 -1 → `perror("open")` + `printf("errno = %d\n", errno)`
- **`errno` 要 `#include <errno.h>`**
- else 分支也要写：万一文件竟然存在，要 `close` 掉并输出说明（体现 if-else 必须有 else 的规约）

**坑**：

- `perror` 输出到**标准错误输出**，`printf` 输出到**标准输出** —— 重定向时两者会分开
- 必须先判断 -1 再读 errno；成功的调用不保证 errno 不变

---

## 課題 4 — fopen 失败时的 perror 与 errno

**要求**：同課題3，但用标准库函数 `fopen`。

**思路**：与課題3 结构完全一样，**只有失败的表示方法不同**：

| | 失败时 |
|---|---|
| `open`（系统调用） | 返回 **-1** |
| `fopen`（库函数） | 返回 **NULL** |

- 变量是 `FILE *pfile_filePointer`（指针类型 → 前缀 `p`）
- 成功时用 `fclose` 关闭

**这题的学习点**：系统调用与库函数在**错误表示方式上不统一**，必须查手册确认。但两者都会设置 errno，所以 `perror` 通用。

---

## 課題 5 — 显示 stdin / stdout / stderr 的文件描述符

**要求**：显示三者的文件描述符。

**思路**：

- `fileno(stdin)` / `fileno(stdout)` / `fileno(stderr)`
- 结果必然是 **0 / 1 / 2**
- 三次调用各自判断返回值（`fileno` 出错返回 -1）

**这题的学习点**：确认「**FILE\* → fd**」的转换，以及标准三件套的 fd 是固定值。

---

## 課題 6 — 打开 tmp.txt 并显示其文件描述符

**要求**：打开 "tmp.txt"，显示文件描述符。

**思路**：

- `open(TARGET_FILE_PATH, O_RDONLY)` → 显示返回值
- 事先要**手动创建 tmp.txt**，否则会失败

**观察点**：结果通常是 **3** —— 因为 0/1/2 已经被标准输入输出占用，新打开的文件从 3 开始分配。这就是「利用者ファイル記述子表的第几个」的实感。

---

## 課題 7 — 用 write 往文件写字符串

**要求**：用 `write` 往文件写字符串。教材指定 `flags` = `O_CREAT | O_WRONLY | O_TRUNC`，`mode` = `0660`。

**思路**：

```c
#define TARGET_FILE_PATH ("./tmp.txt")
#define WRITE_DATA       ("Hello")
#define WRITE_DATA_BYTE  (sizeof(WRITE_DATA) - 1)   /* ← 关键 */
#define FILE_MODE        (0660)                     /* rw-rw---- */

fd = open(TARGET_FILE_PATH, O_CREAT | O_WRONLY | O_TRUNC, FILE_MODE);
write(fd, WRITE_DATA, WRITE_DATA_BYTE);
close(fd);
```

**要点**：

- **`sizeof(WRITE_DATA) - 1`** —— `sizeof` 字符串字面量包含结尾的 `'\0'`，写文件时不需要它，所以减 1。这个 `-1` 在后面的課題12〜16 一直会用到
- `O_CREAT` 时**必须给第3参数 mode**
- `open` / `write` / `close` **三个都要检查返回值**

---

## 課題 8 — 用 read 从文件读字符串

**要求**：用 `read` 从 text 文件读字符串。

**思路**：

```c
char ch_readBuffer[READ_BUFFER_SIZE];
fd = open(TARGET_FILE_PATH, O_RDONLY);
s4_readResult = read(fd, ch_readBuffer, sizeof(ch_readBuffer) - 1);   /* ← 注意 -1 */
if (s4_readResult == -1) { perror("read"); ... }
else if (s4_readResult == 0) { printf("読み出すデータがありません\n"); }
else {
    ch_readBuffer[s4_readResult] = '\0';      /* ← 关键：手动补终端符 */
    printf("読み出した文字列：%s\n", ch_readBuffer);
}
close(fd);
```

**三个要点**：

1. **`read` 不会自动补 `'\0'`** —— 它只是把字节搬过来。要当字符串用就必须自己在 `buf[读到的字节数]` 位置写 `'\0'`
2. 所以读的时候只能读 **`SIZE - 1`** 字节，给 `'\0'` 留位置
3. **`read` 返回 0 表示 EOF**（不是错误）—— 所以判断要分三种：`-1`（错误）/ `0`（无数据）/ `>0`（正常）

> ⚠️ **這個「read → 补 '\0' → printf」的三件套在課題12〜16 已经不再使用了。**
> 讲师 review 時指出「文字列の終端のヌルチェック」，課題12〜16 改成了
> 「发送方连 `'\0'` 一起发 → 接收方**检查**最后一字节是不是 `'\0'`」。
> 详见[第四部分](#2-文字列の終端のヌルチェック字符串终端的空字符检查)。
> 課題8 是**文件**读取（不是通信），没有「消息边界」的问题，所以保持原样。

---

## 課題 9 — fork + 子进程 exit(0xab) + 父进程 wait

**要求**：

- `fork` 生成子进程
- 子进程用 **`exit` 库函数**返回 **0xab** 的状态
- 父进程等子进程结束，显示子进程的状态
- **正常结束和信号结束两种情况都要确认**（信号用 `kill -INT <子进程PID>`）

**思路**：

```c
#define FORK_CHILD_PROCESS  (0)
#define SYSTEM_CALL_ERROR   (-1)
#define CHILD_EXIT_STATUS   (0xAB)
#define CHILD_WAIT_SECOND   (10)     /* 给你时间去敲 kill 命令 */

s4_forkResult = fork();
if (s4_forkResult == SYSTEM_CALL_ERROR)      { perror("fork"); ... }
else if (s4_forkResult == FORK_CHILD_PROCESS){ 子进程处理 }
else                                          { 父进程处理（wait） }
```

父进程侧：

```c
s4_waitResult = wait(&s4_childStatus);
if (s4_waitResult == SYSTEM_CALL_ERROR) { perror("wait"); ... }
else if (WIFEXITED(s4_childStatus))     { printf("終了ステータス：0x%X\n", WEXITSTATUS(s4_childStatus)); }
else if (WIFSIGNALED(s4_childStatus))   { printf("終了シグナル番号：%d\n", WTERMSIG(s4_childStatus)); }
else                                     { printf("未知の要因で終了\n"); }
```

**要点与坑**：

- **子进程要 `sleep` 一会儿**，否则来不及用 `kill -INT` 制造信号结束的情况
- 父进程要**先显示子进程的 PID**，你才知道 kill 谁
- **`0xab` 只有低 8 位有效** —— `WEXITSTATUS` 取的是状态信息的上位 8 bit，正好是 exit 参数的低 8 位。所以用 `%X` 显示会看到 `AB`
- 验证信号结束：程序跑起来后另开一个终端 `kill -INT <PID>`，父进程会走 `WIFSIGNALED` 分支，信号号是 **2**
- `if-else if` 最后**必须有 else**（规约 41-006）

**⚠️ 本次研修最大的坑就在这题**（详见[第三部分](#3-exit-和-return-的取舍-本次研修最大的坑)）：
課題要求「**子プロセスは exit ライブラリー関数により**」，但规约禁止在 main 以外用 exit。最初的做法是「子进程处理函数 return 状态给 main，由 main 的 exit 结束」，但讲师在課題12 review 时指出**应该在子进程处理函数的末尾直接 exit**。

---

## 課題 10 — 子进程用 execl 执行 `ls -l /home/`

**要求**：在課題9 的程序上追加：fork 出子进程，**用 `execl`** 执行 `ls -l /home/`，父进程 wait 并显示状态。

**思路**：

```c
#define LS_COMMAND_PATH   ("/usr/bin/ls")   /* 不带 'p' → 必须写完整路径 */
#define LS_COMMAND_NAME   ("ls")
#define LS_COMMAND_OPTION ("-l")
#define LS_COMMAND_TARGET ("/home/")

execl(LS_COMMAND_PATH, LS_COMMAND_NAME, LS_COMMAND_OPTION, LS_COMMAND_TARGET, NULL);
/* ↑ 成功的话到不了下面这行 */
perror("execl");
return EXIT_FAILURE;
```

**要点**：

- `execl` 的 **`l`** = 参数逐个给，**最后必须是 `NULL`**（忘了会读到垃圾内存）
- 第2参数（`arg0`）习惯上写**命令名本身**（`"ls"`），它会成为新程序的 `argv[0]`
- **`execl` 不带 `p`** → 第1参数必须是**路径名**，不能只写 `"ls"`
- **exec 成功就不返回** → `execl` 后面的代码 = 只有失败才执行的错误处理。这一点要在代码和流程图里注释清楚
- `ls` 的输出会出现在子进程里，父进程的 wait 会看到它正常结束（状态 0）

---

## 課題 11 — 改用 execv

**要求**：把課題10 的 `execl` 改成 **`execv`**。

**思路**：

```c
char *pch_cmdArg[] = {
    LS_COMMAND_NAME,
    LS_COMMAND_OPTION,
    LS_COMMAND_TARGET,
    NULL                 /* ← 数组最后必须是 NULL */
};
execv(LS_COMMAND_PATH, pch_cmdArg);
perror("execv");
return EXIT_FAILURE;
```

**要点**：

- **`v`** = 参数用**指针数组**给，**数组末尾必须是 NULL**
- 与 `execl` 的差别**只在参数的传递方式**，行为完全一样
- 数组是 `char *` 的数组 → 变量名前缀是 `pch_`（指针 + 字符型）
- 数组初始化要用 `{}` 且**数据不能遗漏**（规约 42-007）

**execl vs execv 的选择**：参数个数在编译期固定 → `execl` 直观；参数要动态组装 → `execv` 方便。

---

## 課題 12 — 单向管道：子进程 → 父进程

**要求**：用管道从子进程往父进程发送字符串 `"Hello, I'm your child."`。

**思路**：

```
main:  pipe() → fork() → 分派
子进程: close(fd[0]) → write(fd[1], ...) → close(fd[1]) → 结束
父进程: close(fd[1]) → read(fd[0], ...) → 显示 → close(fd[0]) → wait() → 显示状态
```

**要点与坑**：

1. **`pipe` 必须在 `fork` 之前调用** —— 这样两个 fd 才会被子进程继承，父子才共享
2. **各自关掉不用的那一端**。子进程只写 → 关 `fd[0]`；父进程只读 → 关 `fd[1]`
   - 不关也能跑，但这是坏习惯：**如果写端没有全部关闭，读端的 `read` 就不会返回 0（EOF）**，容易造成永久阻塞
3. `write` 的长度用 `sizeof(SEND_MESSAGE) - 1`
4. `read` 后要**手动补 `'\0'`**（同課題8）
5. fork 失败时，**已经打开的 2 个 fd 要 close 掉**再退出

**变量命名**：`S4 s4_fileDescriptor[2]`，传给函数时是 `S4 *ps4_fileDescriptor`（指针 → 前缀 `p`）。

---

## 課題 13 — 双向管道

**要求**：子进程往父进程发 `"Hello, I'm your child."`，**之后**父进程往子进程发 `"Hello, I'm your parent."`。

**思路**：**开 2 条管道**，一条一个方向。

```c
S4 s4_childToParentFd[2];   /* 子 → 父 */
S4 s4_parentToChildFd[2];   /* 父 → 子 */
pipe(s4_childToParentFd);
pipe(s4_parentToChildFd);
fork();
```

| | 子进程 | 父进程 |
|---|---|---|
| 关掉 | `childToParent[0]`（读端）<br>`parentToChild[1]`（写端） | `childToParent[1]`（写端）<br>`parentToChild[0]`（读端） |
| 然后 | write → close 写端 → read → 显示 → close 读端 | read → 显示 → close 读端 → write → close 写端 → wait |

**最大的坑 —— 死锁**：

顺序必须是「**子写 → 父读 → 父写 → 子读**」。
如果搞成「子先读、父也先读」，两边都阻塞在 read 上，程序永远不动。
本实现里子进程是 write→read，父进程是 read→write，正好错开，不会死锁。

**其他要点**：

- 一共 4 个 fd，fork 失败时全部要 close
- 两条管道的命名要能一眼看出方向（`childToParent` / `parentToChild`），不要用 `fd1` / `fd2`
- 两边都要显示收到的字符串，输出前缀要区分（`子プロセスの受信文字列：` / `親プロセスの受信文字列：`）

> 📌 **本题 review 收到了 2 条 comment**（错误时打ち切り/継続、字符串终端的空字符检查），
> 代码已按指摘修改，详见[第四部分](#第四部分讲师-review-指摘与对应)。

> **注意输出顺序**：用管道重定向捕获输出时，stdio 是全缓冲的，两个进程各自在退出时 flush，所以打印顺序**可能与逻辑顺序不一致**。在终端直接跑（行缓冲）则是正常顺序。这不是 bug。

---

## 課題 14 — 命名管道（要写 2 个程序）

**要求**：用命名管道传输数据。

**思路**：教材明确写了「**データ受信側とデータ送信側それぞれのプログラムを作成すればよい**」，所以是**两个独立的程序**（`task14_receive.c` / `task14_send.c`），不是 fork。

**接收侧**：

```
mkfifo(FIFO_PATH, FIFO_MODE)
  → open(FIFO_PATH, O_RDONLY)      ← 阻塞直到发送侧 open
    → read → 补 '\0' → 显示
    → close
  → unlink(FIFO_PATH)              ← 必须删！
```

**发送侧**：

```
open(FIFO_PATH, O_WRONLY)
  → write
  → close
```

**要点与坑**：

1. **必须先启动接收侧** —— `open(O_RDONLY)` 会**阻塞**直到有写入侧打开同一个 FIFO
2. **`unlink` 的责任在接收侧** —— FIFO 是接收侧用 `mkfifo` 建的，而且「名前つきパイプはプロセスが消滅しても存在し続ける」，不删就会残留
3. **错误处理的嵌套结构**：open 失败也必须 unlink，read 失败也必须 close+unlink
   → 所以**不适合用「出错就早期 return」的写法**（那样 close/unlink 要在每条错误路径重复）
   → 用**单一 `s4_exitStatus` + 嵌套 if-else** 的结构最干净
4. `FIFO_MODE` 用 `0660`，注释写清楚是 `rw-rw----`（同課題7）
5. 建完之后可以用 `ls -l` 确认，文件种别是 **`p`**

---

## 課題 15 — UNIX ドメイン + ストリームソケット（2 个程序）

**要求**：用 UNIX ドメイン的流套接字收发数据。服务器收到客户端的字符串后，再往客户端发送字符串。**收发要用 `write` / `read` 系统调用**。

**思路**（按教材图2）：

| 服务器 | 客户端 |
|---|---|
| `socket(PF_UNIX, SOCK_STREAM, 0)` | `socket(PF_UNIX, SOCK_STREAM, 0)` |
| 地址设定（memset → sun_family → sun_path） | 地址设定（同左） |
| `bind` | *(bind 省略)* |
| `listen` | |
| `accept` ← 阻塞 | `connect` → |
| `read`（用 accept 的 fd） | `write` |
| 显示 | |
| `write`（用 accept 的 fd） | `read` → 显示 |
| `close`(accept fd) → `close`(socket fd) | `close` |
| **`unlink`(SOCKET_PATH)** | |

**要点与坑**：

1. **`accept` 返回的是新 fd** —— 跟客户端的收发必须用**这个新 fd**，不是 socket 返回的那个
2. **服务器侧必须 unlink** —— 教材 3-2-1-2 末尾：「UNIX ドメインの場合、bind システムコールの結果として、与えられたパス名のファイルが作成される。ソケットを使い終わった後必ずファイルを unlink システムコールを使用して消去しておく必要がある」
3. **客户端可以省略 bind**（教材 3-2-2 明说）
4. 地址结构体要**先 memset 清零**再填 —— `sockaddr_un` 有 108 字节的 `sun_path`，不清零会带垃圾
5. `bind` / `connect` 的第2参数要**强制转换成 `struct sockaddr *`**
6. **必须先启动服务器**，否则客户端 `connect` 会失败（`No such file or directory`）
7. **函数拆分**：服务器全写在 main 会嵌套 6 层以上。拆成
   - `main` — 套接字的生命周期（作成 → 名前づけ → 受け入れ準備 → 削除）
   - `s4_CommunicateWithClient` — 一次通信（accept → read → write → close）

   拆完 main 3 层、函数 2 层。客户端够短，全写在 main（3 层）即可。

**关于 `bzero` vs `memset`**：教材用 `bzero`，但规约明确采用 **C90 + `-pedantic-errors`**，而 `bzero` 是 BSD 遗留函数、POSIX.2008 已删除；`<string.h>` 本来就要为 `strcpy` 引入。所以本实现用 `memset`。要跟教材完全一致的话换回 `bzero` + `<strings.h>` 即可。

---

## 課題 16 — INET ドメイン + ストリームソケット（2 个程序）

**要求**：用 INET ドメイン的流套接字收发。**Raspberry Pi 当客户端，Amazon Linux 2 当服务器**。服务器 IP `xxx.xxx.xxx.xxx`（教材に記載），端口用 **49152 以后**（具体号码另行指示）。收发用 `write` / `read`。

**思路**：**结构与課題15 完全一样**，只有下面几处不同：

| | 課題15（UNIX） | 課題16（INET） |
|---|---|---|
| 结构体 | `struct sockaddr_un` | `struct sockaddr_in` |
| 头文件 | `<sys/un.h>` | `<netinet/in.h>` +（客户端）`<arpa/inet.h>` |
| ドメイン | `PF_UNIX` | `PF_INET` |
| 名字设定 | `strcpy(sun_path, パス)` | `sin_port = htons(PORT)`<br>`sin_addr.s_addr = ...` |
| 服务器地址 | — | `htonl(INADDR_ANY)`（本机任意地址） |
| 客户端地址 | — | `inet_addr("xxx.xxx.xxx.xxx")` |
| **服务器 unlink** | **必要** | **不要**（INET 不在文件系统上建文件） |

**要点与坑**：

1. **端口号只是一个宏** —— `#define SERVER_PORT (49152)`，拿到指定号码改一个数字即可（客户端·服务器两边都要改，两边必须一致）
2. **服务器侧不需要 unlink** —— 这是与課題15 最实质的结构差异，main 的判断从 5 个减到 4 个
3. **端口必须放行** —— Amazon Linux 2 那边的**安全组**和 **firewalld** 都要开该端口的 TCP 入站，否则客户端会卡在 `connect` 或报 `Connection timed out`。这是环境配置问题，不是代码问题
4. **`bind: Address already in use`** —— 程序退出后套接字会进 **TIME_WAIT** 约 1 分钟，此时重启服务器会 bind 失败。教材没讲 `setsockopt(SO_REUSEADDR)`，所以没加。撞上就等一会儿，或换端口
5. **本机验证方法**：把客户端的 IP 宏临时换成 `"127.0.0.1"` 做回环测试，就能在一台机器上验证逻辑（交付的源码保持真实 IP）

---

# 第三部分：贯穿全部课题的规约要点

## 1. 命名规约

**类型前缀 + `_` + 名字**：

| 类型 | 略称 | 前缀 | 标准型 |
|---|---|---|---|
| 文字型 | — | `ch` | `char` |
| 符号無整数 1/2/4/8 字节 | `U1`/`U2`/`U4`/`U8` | `u1`/`u2`/`u4`/`u8` | `unsigned char/short/int/long` |
| 符号有整数 1/2/4/8 字节 | `S1`/`S2`/`S4`/`S8` | `s1`/`s2`/`s4`/`s8` | `signed char/short/int/long` |
| 単精度/倍精度浮動小数点 | `FL`/`DL` | `fl`/`dl` | `float`/`double` |
| 型無し | `VD` | `vd` | `void` |
| 構造体/共用体 | typedef 的类型名 | `st`/`un` | `struct`/`union` |

**修饰前缀的顺序**：`s`(static) → `v`(volatile) → `p`(pointer) → 类型前缀

```c
static S4 ss4_ErrorStatus;      /* static 外部变量 */
volatile S4 vs4_LedOn;          /* volatile 外部变量 */
char **ppch_errorCharacter;     /* 二级指针 */
static VD *spvd_MemAlloc(U4);   /* static 指针型函数 */
```

**大小写规则**：

| 对象 | 规则 |
|---|---|
| 外部变量的名字部分 | **UpperCamelCase** |
| **局部变量**的名字部分 | **lowerCamelCase** |
| **函数**的名字部分 | **UpperCamelCase** |
| 宏·常量 | **UPPER_SNAKE_CASE**，且**值要用 `()` 包起来** |

```c
S4 s4_lucentJirou;              /* 局部变量 */
VD vd_InputCompare(void);       /* 函数 */
#define BAT_VOLTAGE_MAX (3)     /* 宏 */
```

> `main` 函数**不需要**类型前缀。

## 2. 风格

- **变量定义按类型归类，尽量从大类型开始**
- **一个声明语句只声明一个变量**（`U1 a, b;` ✗）
- 宏定义**按块对齐各列首位置**
- 运算符两边加半角空格（`++`/`--`/单目/`.`/`->` 除外）
- **文件头注释**：`@file` / `@brief` / `@version` / `@date` / `@author`
- **函数头注释**：`@brief` / `@param[in,out]` / `@retval`
- 注释写「**为什么需要这个处理**」，不是「在做什么」

## 3. 必须遵守的规则（摘录）

| 番号 | 内容 |
|---|---|
| 41-002 | **无参数的函数要写 `void`**（`f()` 是「参数个数类型不明」的旧式声明） |
| 41-006 | **`if-else if` 最后必须有 `else`**。不会发生时写 `/* DO NOTHING */` |
| 41-007/008 | `switch` 最后必须有 `default`；每个 `case`/`default` 必须以 `break` 结束 |
| 42-001 | 不声明·定义用不到的东西 |
| 42-005 | **有意义的常量要定义成宏** |
| 42-006 | **只读的区域要加 `const`** |
| 42-008 | `if`/`while`/`for` 等的本体**必须用 `{}` 块化** |
| 42-011 | **真伪判定式里不写赋值运算符**（`if (p = q)` ✗） |
| 42-014 | 头文件要有 include guard |
| 42-015 | include 顺序：**标准库 → 第三方库 → 项目固有**（依赖弱的在前） |
| 42-017 | 空指针一律用 `NULL`，`NULL` 不作他用 |

## 4. 结构化编程

**基本**：顺次·分歧·反复，**每个部件都是「入口1个、出口1个」**。

**从结构化编程的逸脱**（出口变多）：

| 逸脱 | 判定 |
|---|---|
| **main 以外的 exit** | **原则禁止**（用返回值代替） |
| `longjmp` | **禁止**（可读性剧降） |
| `break` | 需要中止循环时**才**允许，但流程图上要明确画出 |
| `continue` | 需要跳过部分处理时**才**允许，流程图上要明确画出 |
| **同一函数内多个 return** | **只在提高可读性时**允许（避免深层嵌套），流程图上要明确画出 |
| `goto` | 同上，**只在提高可读性时**允许 |

**关于 main**：

> main 中的 exit 与 return（几乎）行为相同，所以**允许在 main 中使用 exit**。
> 但是 —— **main 内用 return 就禁止用 exit，用 exit 就禁止用 return（二选一）**。

## 5. C 语言标准

本研修采用 **C90**。编译时**必须**加：

```bash
gcc -Wall -std=c90 -pedantic-errors -I../include -o <出力> <ソース>
```

| 选项 | 作用 |
|---|---|
| `-Wall` | 启用全部警告 |
| `-std=c90` | 指定 C 语言标准 |
| `-pedantic-errors` | **不符合 ISO C 的代码当成 error** |

**C90 的约束（容易踩的）**：

- **变量声明必须在块的开头**（C99 才能中途声明）
- **不能用 `//` 单行注释**（C99 才有）
- 没有 `_Bool`、`long long`、可变长数组、复合字面量、`snprintf`

## 3'. exit 和 return 的取舍 — 本次研修最大的坑

这是課題9〜16 一路贯穿、并且在課題12 的 review 上被指出的问题，单独记录。

### 事情的经过

課題9 要求「**子プロセスは exit ライブラリー関数により 0xab のステータスを返すこと**」，
但规约写着「**main 関数以外での exit の使用は禁止**」。

最初的应对（課題9〜11）：

```c
S4 s4_ChildProcess(VD) { ...; return CHILD_EXIT_STATUS; }   /* 函数只 return */
int main(void) {
    ...
    else if (子) { s4_exitStatus = s4_ChildProcess(); }
    else         { s4_exitStatus = s4_ParentProcess(); }
    exit(s4_exitStatus);        /* exit 只在 main 出现一次 */
}
```

这个结构本身是自洽的：exit 全程只有 main 里一处，子进程的终了状态是一路 return 上来的。

### 讲师在課題12 指出的两点

1. **子进程处理函数的末尾应该直接 `exit`**
2. **main 里为什么用 `exit` 而不是 `return`？**

**这两点是同一个改造的两半**：

- fork 之后**子进程不能「返回」到父进程的代码路径上去**，它的终点应该画在自己的处理末尾 —— 教材 3-1-1 的示例代码正是 `close(fd[1]); exit(0);`
- 把 exit 挪进子进程函数之后，**main 的末尾就只剩父进程会到达** —— 父进程只是正常结束 main，这时用 `return` 才自然

### 最终的形态

```c
VD vd_ChildProcess(...);      /* 内部 exit，不返回 */
S4 s4_ParentProcess(...);     /* 状态用返回值返回 */

int main(void)
{
    ...
    } else if (s4_forkResult == FORK_CHILD_PROCESS) {
        vd_ChildProcess(...);        /* 子进程在这个函数里结束 */
    } else {
        s4_exitStatus = s4_ParentProcess(...);
    }

    return s4_exitStatus;            /* 到达这里的只有父进程 */
}
```

### 怎么向讲师解释「函数里的 exit」

规约禁止 main 以外用 exit 的**理由**是「関数の返却値を使用することで exit を使用せずに対応できるため」。
**对 fork 出来的子进程这个前提不成立** —— 返回值传不回去，`return` 不会让进程结束。这条规则的前提是「单进程」。

日语说法：

> 通常は規約どおり戻り値で対応しますが、fork 後の子プロセスは親プロセスの処理へ戻ってはならず、戻り値ではプロセスを終了させられません。そのため子プロセス処理関数の最後で exit を使用し、その旨を関数ヘッダの @note に明記しています。main 関数は親プロセスのみが到達するため、return で終了ステータスを返しています。

### ⚠️ 不要写成这样

```c
exit(s4_exitStatus);
return 0;              /* ✗ 三重问题 */
```

1. 违反规约「main 内 exit 与 return 二选一」
2. `return 0` 是**不可达的死代码**
3. **掩盖了真正的返回值** —— 读代码的人会以为返回 0，实际返回的是 `s4_exitStatus`

## 6. 課題14〜16 用的「単一 exitStatus + 嵌套 if-else」模式

課題7/8 用的是「出错就早期 return」（多个 return），課題12 以后统一成这个模式：

```c
S4 s4_exitStatus = EXIT_SUCCESS;      /* ① 先设成成功 */

s4_xxxResult = xxx();
if (s4_xxxResult == SYSTEM_CALL_ERROR) {
    perror("xxx");
    s4_exitStatus = EXIT_FAILURE;     /* ② 出错只「设置」，不 return */
} else {
    s4_yyyResult = yyy();             /* ③ 成功才往下嵌套 */
    if (...) {
        ...
    } else {
        /* DO NOTHING */              /* ④ else 必须有 */
    }
    後始末(close/unlink);              /* ⑤ 后处理写在这一层 */
}

return s4_exitStatus;                 /* ⑥ 唯一的 return */
```

**为什么选这个**：有资源清理（close / unlink / 已打开的 fd）的场合，早期 return 会让清理代码在每条错误路径上重复；嵌套结构天然保证「打开了就一定关掉」。

**代价**：嵌套会深。深到 4 层以上就该**拆函数**了 —— 課題15/16 的服务器就是这么拆的。

---

# 第四部分：讲师 review 指摘与对应

課題13 的 review 收到了 2 条 comment。因为方针问题会波及所有通信题，**課題13〜16 全部统一修改了**（課題12 保持原样）。

## 1. エラーが発生した場合に打ち切るか継続するか（出错时中止还是继续）

### 指摘的内容

> 「エラーが発生した場合に打ち切るか継続するかは**議論の余地がある**」
> 举例：車載系「一出错就先停」、通信系「总之先让它跑」

这不是「你写错了」，而是「**这是个设计决策，你要有意识地做出选择并能说明理由**」。

### 术语（日本工程界的固定说法）

| 方针 | 术语 | 典型领域 | 逻辑 |
|---|---|---|---|
| **打ち切り** | **フェールセーフ**（fail-safe） | 車載、鉄道、医療機器、プラント制御 | 用错误的数据继续跑可能造成**物理伤害**。「動かないほうがマシ」，检测到异常立刻倒向安全侧 |
| **継続** | **フェールソフト** / 可用性優先 | 通信、サーバー | 一次错误不该让整个服务倒掉。记录、重试、继续服务 |

相关词：**フェールオーバー**（切到冗余系）、**フォールトトレラント**（故障了也保持正常动作）。

### 指的是代码里哪些地方

`s4_exitStatus = EXIT_FAILURE;` 之后**要不要往下做**：

| 位置 | 出错的调用 | 之后 |
|---|---|---|
| `vd_ChildProcess` | `write` 失败 | 仍然继续 `read` |
| `s4_ParentProcess` | `read` 失败 | 仍然继续 `write` |
| `s4_ParentProcess` | `write` 失败 | 仍然继续 `wait` |

### 结论：本程序选「継続」，理由是技术性的

做过实验验证。把子进程改成「write 之后立刻打ち切り（不 read 直接结束）」：

```
親プロセスの受信文字列：Hello, I'm your child.
→ 親プロセスの終了コード = 141
```

**141 = 128 + 13 = SIGPIPE，父进程被信号杀死了。**

原因就在教材表20：**`SIGPIPE`（13）= 読み手のいないパイプへの書き込み**。

- 子进程提前退出 → `parentToChild[0]`（读端）全部关闭
- 父进程随后 `write(parentToChild[1])` → 没有读端 → SIGPIPE → 默认动作是终止进程

而且把输出重定向到管道时（全缓冲），父进程连**已经 printf 出来的那行都会丢失** —— 它在 flush 之前就被杀了。

**所以：在双向通信里，单方面提前退出会把错误传染给对方，而且传染的形式是「对方被信号杀死」这种更难查的故障。**

另外，父进程侧如果 `read` 失败就立刻 return，会**跳过 `wait()`** → 拿不到子进程的终了状态，而这正是課題9 一路继承下来的功能要求。

### 采用的方针（已写进各 .c 文件的「設計方針」注释块）

> **依赖关系的有无来区分**：
>
> - **前提不成立、根本做不下去** → **打ち切り**（`open` 失败后的 `read`、`socket` 失败后的 `bind` 等）
>   用**嵌套**表现：出错就不进入下一层。后处理（`close` / `unlink` / `wait`）**必ず実行**
> - **前提成立、能做但要不要做** → **継続**（送信失败后的受信等）
>   这才是「議論の余地」所在

这个区分同时解释了「为什么 課題13 是平铺、課題14〜16 是嵌套」—— 不是前后不一致，是**两种情况本来就不同**。

### ❌ 不要用的理由

> ~~「構造化プログラミングで関数内の return を1個にしているため、途中で打ち切れなかった」~~

**这个说法站不住脚，讲师会当场反驳。** 实测：把 task13 改成「単一 exit / 単一 return 不变，但出错就打ち切り」（把后续处理放进 `else`），

```
コンパイル OK（-Wall -std=c90 -pedantic-errors、警告ゼロ）
exit(s4_exitStatus) の個数: 1
return s4_returnValue の個数: 1
```

只有 2 层嵌套，一点都不深。而且规约提供了 **3 条**合规的打ち切り路径：

| 手法 | 规约的态度 |
|---|---|
| 深い分岐の nest | 基线做法，无条件合规 |
| 複数個の return | 「可読性が高まる場合に限り」許可（要在流程图上画明） |
| goto Fin | 同上 |

**更要命的是**：`structured_programming.md` 里演示这两种手法的示例代码，**本身就是打ち切り的写法**（`if (sts != ERROR) { 次の処理 }` 的嵌套链）。而且你自己的 課題14〜16 已经用了嵌套方式 —— 拿「単一 return」当理由等于自相矛盾。

「単一 return」真正影响的是**代码的形状**，不是**方针的选择**：维持単一 return 打ち切り时，后续处理要放进 `else`，结果**后处理（close）就得挪到最外层** —— 而 課題13 里 `close(childToParentFd[1])` 的位置是有意义的（写完就关，让对端的 read 能拿到 EOF），挪走会改变时序语义。这个论点可以讲。

### 给讲师的说法

> エラー時に打ち切るか継続するかは、**依存関係の有無で分けています**。
>
> 前提が成立せず処理を続行できない場合（`open` 失敗後の `read` 等）は打ち切り、続行可能な場合（送信失敗後の受信等）は継続する方針としました。
>
> 継続とした理由は 2 点です。
> 1. 双方向通信では一方が先に終了すると相手の `write` が読み手のいないパイプ／ソケットへの書き込みとなり、**相手プロセスが SIGPIPE で異常終了**してしまいます。
> 2. 親プロセスが途中で打ち切ると `wait` を実行できず、**子プロセスの終了状態を取得するという課題の要求**が満たせなくなります。
>
> なお、車載などフェールセーフが優先される領域では逆に「検出したら即座に打ち切る」のが適切であることは理解しています。本プログラムは通信処理であり、可用性を優先する側と判断しました。

---

## 2. 文字列の終端のヌルチェック（字符串终端的空字符检查）

### 指摘的内容

> 「文字列の終端のヌルチェック」

修改前的代码做的是「**付ける**」（强制补终端符），不是「**チェックする**」（检查）：

```c
s4_readResult = read(fd, ch_receiveBuffer, BUFFER_SIZE - 1);
...
ch_receiveBuffer[s4_readResult] = '\0';    /* ← 无条件补上 */
printf("...%s\n", ch_receiveBuffer);
```

- 发送方发 `sizeof(MSG) - 1` 字节，**故意不发 `'\0'`**
- 接收方无条件在 `buf[读到的字节数]` 补 `'\0'`
- **结果：不管收到的是完整消息、半截消息还是垃圾，接收方都当成「合法字符串」处理**

### 实验：静默截断

把消息加长到 147 字节（缓冲区只能收 99）：

```
親プロセスの受信文字列：Hello, I am your child. ...(略)... Hello, I am your child. Hel
→ 終了コード = 0   ★エラーは一切報告されない
```

末尾的 `END` 丢了，程序**报告成功**。

### 为什么这在通信程序里特别重要

管道和流套接字都是**字节流**，**没有「消息边界」这个概念**。`read` 只告诉你「读到了 n 字节」，不告诉你「这 n 字节是不是一条完整的消息」。

划分消息边界的三种标准手法：

| 手法 | 说明 |
|---|---|
| **終端文字（デリミタ）方式** | 把 `'\0'` 或 `'\n'` 作为终端符**一起发送**，接收方**检查它是否收到** ← 采用这个 |
| **長さ先行（length-prefix）方式** | 先发 4 字节的长度，再发正文。实务上最正规 |
| **固定長方式** | 消息长度固定 |

修改前的代码哪种都不是 —— 靠的是「发送方和接收方是同一个人写的，消息肯定不超过 99 字节」这个**隐含前提**。**課題14/15/16 的发送方和接收方是两个独立的程序**（課題16 甚至跑在两台机器上），这个约定完全没写在代码里。

而且套接字比管道更严格：TCP 是纯字节流，**`read` 完全可能只返回消息的一部分**。

### 修改后的形态

```c
/* 送信側：sizeof から -1 しない ＝ 終端のヌル文字も送る */
#define SEND_MESSAGE_BYTE (sizeof(SEND_MESSAGE))   /* 終端のヌル文字を含む */
s4_writeResult = write(fd, SEND_MESSAGE, SEND_MESSAGE_BYTE);

/* 受信側：'\0' はデータの一部なので BUFFER_SIZE まで読める */
s4_readResult = read(fd, ch_receiveBuffer, BUFFER_SIZE);

/* s4_readResultが0の場合にch_receiveBuffer[s4_readResult - 1]を参照すると
   範囲外アクセスとなるため、0の判定を終端の判定より先に行う。 */
if (s4_readResult == SYSTEM_CALL_ERROR) {
    perror("read");
    s4_exitStatus = EXIT_FAILURE;
} else if (s4_readResult == 0) {                            /* ← EOF */
    printf("相手がクローズしました\n");
    s4_exitStatus = EXIT_FAILURE;
} else if (ch_receiveBuffer[s4_readResult - 1] != '\0') {   /* ← ヌルチェック */
    printf("受信文字列が終端されていません\n");
    s4_exitStatus = EXIT_FAILURE;
} else {
    printf("受信文字列：%s\n", ch_receiveBuffer);
}
```

### 三个实现细节

1. **`read` 的第3参数从 `BUFFER_SIZE - 1` 改成 `BUFFER_SIZE`** —— `'\0'` 现在是数据的一部分，不需要再给它单独留位置。`buf[BUFFER_SIZE-1]` 是合法下标
2. **`== 0` 的判定必须放在 `!= '\0'` 判定之前** —— 否则 `s4_readResult - 1` 会变成 `-1`，读到 `buf[-1]`。这个顺序是有意的，代码里加了注释说明
3. **宏也要改**：`(sizeof(SEND_MESSAGE) - 1)` → `(sizeof(SEND_MESSAGE))`，注释写清楚「終端のヌル文字を含む」。因为課題7 那个 `-1` 的写法用了好几题，突然不 `-1` 会让人以为写错了

### 验证结果

| 场景 | 修改前 | 修改后 |
|---|---|---|
| 正常系 | 正常 / 退出码 0 | 正常 / 退出码 0 |
| 147 字节（超过缓冲区） | 静默截断 / **退出码 0** | `受信文字列が終端されていません` / **退出码 1** |
| 对端不发数据就关闭 | 打印空字符串 / 察觉不到 | `〜がクローズしました` / **退出码 1** |

顺带这个改法**一并解决了 `read` 返回 0（EOF）没处理的问题** —— 課題8 处理了 `== 0`，但課題12〜16 原本都没处理。

### 给讲师的说法

> ご指摘のとおり、修正前は受信側で無条件に `'\0'` を付与しており、**終端されているかどうかのチェックはしていませんでした**。そのため受信データがバッファサイズを超えて途中で切れていても検出できず、正常終了として扱われてしまう問題がありました。
>
> 修正版では、**送信側が終端のヌル文字を含めて送信し、受信側で最終バイトがヌル文字であることを確認する**方式に変更しました。あわせて `read` の戻り値が 0（相手がクローズした場合）の判定も追加しています。
>
> なお、ストリーム型通信にはメッセージの区切りが存在しないため、区切りを与える方式としては他に「長さ先行方式」「固定長方式」がありますが、本課題では扱う文字列が短く固定であるため、終端文字方式を採用しました。

---

## 3. 残った課題（自分で見つけた点）

EOF 検出のテスト中に見つかった、**2つの指摘が交差するところ**：

サーバー側で「クライアントが接続だけして送信せずにクローズ」を試すと、

```
クライアントがソケットをクローズしました
→ server exit=141   ← SIGPIPE
```

EOF を検出できてはいるが、その後**継続方針に従って `write` してしまう**ため、既に切断した相手への書き込みとなり SIGPIPE で死ぬ。

**継続方針の前提は「相手がまだ存在していること」。`read` が 0 を返した時点で相手は既にいないので、この場合に限っては打ち切るのが筋。**

- 修正前も同じ挙動（141）だったので**デグレではない**。修正によって原因が表示されるようになった分、むしろ改善している
- 課題の正常系では発生しない
- 直すには `write` を「EOF でない」分岐の中に入れる必要があり、フロー図の構造がかなり複雑になる

→ **今回は直さず、この文書に記録するに留めた。** 讲师に聞かれたら上記のとおり説明する。

---

## 4. 修改范围一览

| 文件 | 打ち切り/継続の明記 | ヌルチェック |
|---|:---:|:---:|
| task13.c | ✓ | ✓（双方向 2 箇所） |
| task14_send.c | ✓ | ✓（送信のみ） |
| task14_receive.c | ✓ | ✓ |
| task15_client.c / task16_client.c | ✓ | ✓ |
| task15_server.c / task16_server.c | ✓ | ✓ |
| task12.c | — | — （据说明保持原样） |

流程图侧：各 `read` 之后的判定从 **2 分支变成 4 分支**（エラー / EOF / 未終端 / 正常），`'\0'格納` 框删除，送信框的字节数表记改成 `〜_BYTE`。

方针写在各 `.c` 文件开头的「設計方針」注释块里（和 `送信文字列` 那些宏块同样的格式），review 时可以直接指着它说明。

---

# 附录：速查表

## A. 系统调用的返回值

| 系统调用 | 成功 | 失败 |
|---|---|---|
| `open` | **文件描述符** | -1 |
| `close` / `unlink` / `pipe` | 0 | -1 |
| `read` / `write` | **实际字节数**（read 返回 **0 = EOF**、課題12〜16 ではエラー扱い） | -1 |
| `fork` | 子进程 **0** / 父进程 **子的PID** | -1 |
| `wait` / `waitpid` | **子进程的 PID** | -1 |
| `mkfifo` | 0 | -1 |
| `socket` | **套接字的文件描述符** | -1 |
| `bind` / `listen` / `connect` | 0 | -1 |
| **`accept`** | **新的文件描述符** | -1 |
| `send` / `recv` | 实际字节数 | -1 |
| `getuid` / `geteuid` / `getgid` / `getegid` | ID（**必定成功**） | — |
| `fopen`（库函数） | `FILE *` | **`NULL`** |
| `fileno` | 文件描述符 | -1 |

> 除了 `fopen` 返回 NULL 以外，其他基本都是 **-1 = 错误**。所以定义一个 `#define SYSTEM_CALL_ERROR (-1)` 用到底。

## B. 子进程状态判定宏

```c
WIFEXITED(status)    → 真：正常结束      → WEXITSTATUS(status) 取终了状态
WIFSIGNALED(status)  → 真：信号结束      → WTERMSIG(status)    取信号号
WIFSTOPPED(status)   → 真：因信号停止    → WSTOPSIG(status)    取信号号（需 WUNTRACED）
WIFCONTINUED(status) → 真：因 SIGCONT 再开
```

## C. 头文件对照

| 要用的东西 | 头文件 |
|---|---|
| `printf` / `perror` / `FILE` / `fileno` | `<stdio.h>` |
| `exit` / `EXIT_SUCCESS` / `EXIT_FAILURE` / `atexit` | `<stdlib.h>` |
| `errno` | `<errno.h>` |
| `memset` / `strcpy` | `<string.h>` |
| `bzero` | `<strings.h>` |
| `open` / `O_RDONLY` 等 | `<fcntl.h>` |
| `read` / `write` / `close` / `unlink` / `fork` / `pipe` / `_exit` | `<unistd.h>` |
| `mkfifo` / `mode_t` | `<sys/types.h>` + `<sys/stat.h>` |
| `wait` / `waitpid` / `WIFEXITED` 等 | `<sys/types.h>` + `<sys/wait.h>` |
| `exec` 系 | `<unistd.h>` |
| `signal` / `SIGINT` 等 | `<signal.h>` |
| `socket` / `bind` / `listen` / `accept` / `connect` | `<sys/types.h>` + `<sys/socket.h>` |
| `struct sockaddr_un` | `<sys/un.h>` |
| `struct sockaddr_in` / `htons` / `htonl` / `INADDR_ANY` | `<netinet/in.h>` |
| `inet_addr` | `<arpa/inet.h>` |
| `S4` / `VD` 等 | `"type.h"` |

## D. 课题一览

| 課題 | 主题 | 关键 API | 程序数 |
|---|---|---|---|
| 3 | 系统调用的错误 | `open` + `perror` + `errno` | 1 |
| 4 | 库函数的错误 | `fopen`（失败返回 **NULL**） | 1 |
| 5 | 标准三件套的 fd | `fileno` → 0/1/2 | 1 |
| 6 | 文件的 fd | `open` → 通常是 3 | 1 |
| 7 | 写文件 | `write` + `O_CREAT\|O_WRONLY\|O_TRUNC` + `0660` | 1 |
| 8 | 读文件 | `read` + 手动补 `'\0'` + EOF 判定 | 1 |
| 9 | 进程生成与终了状态 | `fork` / `exit(0xab)` / `wait` / `WIFEXITED` / `WIFSIGNALED` | 1 |
| 10 | 执行别的程序 | `execl`（参数逐个 + NULL） | 1 |
| 11 | 同上 | `execv`（指针数组 + NULL） | 1 |
| 12 | 单向管道 | `pipe` ×1 | 1 |
| 13 | 双向管道 | `pipe` ×2（注意死锁）＋ヌルチェック | 1 |
| 14 | 命名管道 | `mkfifo` + `unlink` ＋ヌルチェック | **2** |
| 15 | UNIX ドメイン套接字 | `socket(PF_UNIX)` + `bind`/`listen`/`accept`/`connect` + **`unlink`** | **2** |
| 16 | INET ドメイン套接字 | `socket(PF_INET)` + `htons`/`htonl`/`inet_addr`（**不需要 unlink**） | **2** |

## E. 常用验证命令

```bash
# 编译（全部课题共通）
gcc -Wall -std=c90 -pedantic-errors -I../include -o task13 task13.c

# 課題9：信号结束的验证
./task9 &          # 记下显示的子进程 PID
kill -INT <PID>    # 父进程会走 WIFSIGNALED 分支，信号号 = 2

# 課題14：先接收后发送
./task14_receive & sleep 0.5; ./task14_send
ls -l task14_fifo  # 应该已被 unlink 掉

# 課題15：先服务器后客户端
./task15_server & sleep 0.5; ./task15_client
ls -l task15_socket  # 应该已被 unlink 掉

# 課題16：本机回环验证（把客户端的 IP 宏临时改成 127.0.0.1）
./task16_server & sleep 0.5; ./task16_client

# 确认退出码
echo $?

# 确认文件种别（p = 命名管道, s = 套接字）
ls -l
```
