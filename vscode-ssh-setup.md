# VS Code から Amazon Linux サーバーへ SSH 接続する手順（中文メモ）

研修で配布された `.pem` ファイルを使い、Tera Term の代わりに **VS Code** からサーバーへ接続してリモート開発する手順。

> Tera Term では「ユーザー名を入力 → 秘密鍵(pem)を選択 → パスワードなしで接続」という操作。
> VS Code でも同じことができる（**Remote - SSH** 拡張機能を使う）。

---

## 0. 先说一个可能踩的坑

**Amazon Linux 2 的 glibc 是 2.26，而 VS Code 1.86 以后的 remote server 要求 glibc ≥ 2.28。**

如果连上去之后报这个：

```
/lib64/libc.so.6: version `GLIBC_2.28' not found
```

就是撞上了。解决办法：

| 办法 | 说明 |
|---|---|
| **VS Code 降级到 1.85.2** | 最省事。降级后要**关掉自动更新**（设置里搜 `update.mode` → 改成 `none`），否则又会升回去 |
| 确认服务器是 **Amazon Linux 2023** | glibc 2.34，没这个问题 |

> SSH 连接本身不受影响 —— 命令行 `ssh` 照样能连。这个坑只影响 VS Code 的远程开发功能。
> 所以**先按下面做，遇到再处理**。

---

## 方法A：Windows 的 VS Code（推荐）

Tera Term 是 Windows 应用，`.pem` 大概率也在 Windows 侧，所以这条路最直接。

### 步骤 1：装扩展

VS Code 里 `Ctrl+Shift+X` → 搜 **`Remote - SSH`** → 装微软官方那个（发布者 `Microsoft`，ID `ms-vscode-remote.remote-ssh`）。

### 步骤 2：把 pem 放到固定位置

放到 `%USERPROFILE%\.ssh\` 下面（就是 `C:\Users\<你的用户名>\.ssh\`）。

目录不存在的话先建：

```powershell
mkdir "$env:USERPROFILE\.ssh"
```

下面假设文件名是 `training.pem`，换成你实际的。

### 步骤 3：改权限 ⚠️ 最容易卡住的一步

Windows 的 OpenSSH 会**拒绝「别人也能读」的私钥**。开 **PowerShell** 跑：

```powershell
icacls "$env:USERPROFILE\.ssh\training.pem" /inheritance:r
icacls "$env:USERPROFILE\.ssh\training.pem" /grant:r "$env:USERNAME:R"
```

- 第一条：切断从上级目录继承来的权限
- 第二条：只给你自己「读」权限

不做这步会报：

```
WARNING: UNPROTECTED PRIVATE KEY FILE!
Permissions for 'training.pem' are too open.
```

### 步骤 4：写 SSH 配置

新建（或编辑）`%USERPROFILE%\.ssh\config` —— **没有扩展名**，不是 `config.txt`。

```
Host aws-training
    HostName <サーバーのIPアドレス>
    User ec2-user
    IdentityFile ~/.ssh/training.pem
    IdentitiesOnly yes
    ServerAliveInterval 60
```

| 项目 | 填什么 |
|---|---|
| `Host` | 随便起个名字，之后就用这个名字连 |
| `HostName` | **Tera Term 里输的那个地址**（教材记载のアドレス） |
| `User` | **Tera Term 里输的用户名**（Amazon Linux 通常是 `ec2-user`） |
| `IdentityFile` | pem 的路径 |
| `IdentitiesOnly yes` | 只用这个 key。不加的话 ssh 会先试 `.ssh` 里已有的其他 key，试几次可能被服务器拒绝 |
| `ServerAliveInterval 60` | 每 60 秒发个心跳，防止长时间不操作被断开 |

### 步骤 5：先用命令行测一下

**别直接上 VS Code** —— 命令行的报错信息清楚得多，出问题好定位。

PowerShell 里：

```powershell
ssh aws-training
```

看到服务器的 shell 提示符（比如 `[ec2-user@ip-xxx ~]$`）就成功了。

第一次连会问：

```
The authenticity of host ... can't be established.
ED25519 key fingerprint is SHA256:...
Are you sure you want to continue connecting (yes/no/[fingerprint])?
```

输 `yes` 回车。这是把服务器的指纹记到 `known_hosts` 里，只问一次。

退出用 `exit`。

### 步骤 6：VS Code 连接

1. 按 `F1`（或 `Ctrl+Shift+P`）打开命令面板
2. 输入 `Remote-SSH: Connect to Host...`
3. 列表里选 **`aws-training`**
4. 如果问平台 → 选 **Linux**
5. 等它在远程安装 VS Code Server（第一次要 1〜2 分钟）

连上后**左下角会显示 `SSH: aws-training`**。

然后：

- `Ctrl+Shift+E` → **Open Folder** → 选服务器上的目录（比如 `/home/ec2-user/`）
- `` Ctrl+` `` 打开的终端**就是服务器上的终端**
- 编辑、保存、编译全都直接在服务器上进行，不用来回传文件

---

## 方法B：从 WSL 连接

如果你想在 WSL 的环境里用 `ssh` / VS Code (WSL) 连服务器。

### ⚠️ 多一个坑：`/mnt/c/` 下的文件设不了 Unix 权限

`chmod 600` 对 `/mnt/c/...` 下的文件**无效**，ssh 还是会报 UNPROTECTED。
所以必须**先复制到 WSL 自己的文件系统里**：

```bash
cp "/mnt/c/Users/<你的用户名>/Downloads/training.pem" ~/.ssh/training.pem
chmod 600 ~/.ssh/training.pem
```

### 配置

编辑 `~/.ssh/config`：

```
Host aws-training
    HostName <サーバーのIPアドレス>
    User ec2-user
    IdentityFile ~/.ssh/training.pem
    IdentitiesOnly yes
    ServerAliveInterval 60
```

### 测试

```bash
ssh aws-training
```

---

## 常见报错对照表

| 报错 | 原因 | 解决 |
|---|---|---|
| `UNPROTECTED PRIVATE KEY FILE` | pem 权限太开放 | Windows 跑 `icacls`；WSL 复制到 `~/.ssh/` 再 `chmod 600` |
| `Permission denied (publickey)` | 用户名不对，或 pem 不是这台服务器的 | 用户名试 `ec2-user`；确认 pem 和 Tera Term 里用的是同一个 |
| `Connection timed out` | 安全组没放行你的 IP，或地址写错 | 确认 IP；让管理员在安全组开 22 端口的入站 |
| `Too many authentication failures` | ssh 试了太多把 key | 配置里加 `IdentitiesOnly yes` |
| `GLIBC_2.28 not found` | VS Code 版本与 Amazon Linux 2 不兼容 | VS Code 降到 1.85.2 并关掉自动更新（见开头第 0 节） |
| `Could not establish connection` | 上面几种的综合症状 | 先用命令行 `ssh` 测，报错更具体 |

---

## 补充：和課題16 的关系

課題16 の構成は「Raspberry Pi = クライアント、**Amazon Linux 2 = サーバー**」。
配布された pem で接続するサーバーが課題16 のサーバーと同じであれば、
**VS Code で直接サーバー上で `task16_server` を編集・コンパイル・実行できる**ので、
ファイルを行き来させる必要がなくなる。

サーバー側でのビルド：

```bash
gcc -Wall -std=c90 -pedantic-errors -I../include -o task16_server task16_server.c
./task16_server
```

> このリポジトリの `task16_client.c` の `SERVER_IP_ADDRESS` は
> プレースホルダー（`xxx.xxx.xxx.xxx`）にしてある。
> 実行前に教材記載のアドレスへ書き換えること。

---

## 快速参考

```powershell
# Windows：权限修正
icacls "$env:USERPROFILE\.ssh\training.pem" /inheritance:r
icacls "$env:USERPROFILE\.ssh\training.pem" /grant:r "$env:USERNAME:R"

# 接続テスト
ssh aws-training

# VS Code
F1 → Remote-SSH: Connect to Host... → aws-training
```

```bash
# WSL：pem を WSL 側へコピーして権限修正
cp "/mnt/c/Users/<user>/Downloads/training.pem" ~/.ssh/training.pem
chmod 600 ~/.ssh/training.pem
ssh aws-training
```
