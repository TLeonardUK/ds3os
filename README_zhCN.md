![Dark Souls 3 - Open Server](./Resources/banner.png?raw=true)

![GitHub license](https://img.shields.io/github/license/TLeonardUK/ds3os)
![GitHub release](https://img.shields.io/github/release/TLeonardUK/ds3os)
![GitHub downloads](https://img.shields.io/github/downloads/TLeonardUK/ds3os/total)
[![Discord](https://img.shields.io/discord/937318023495303188?label=Discord)](https://discord.gg/pBmquc9Jkj)

<div align="center">

[English](./README.md) | 简体中文

</div>

**注**：中文文档直接翻译自[英文版](./README.md)，因译者水平有限，如果您认为翻译得不通顺请配合英文原版查看，中文文档可能不会随英文文档修改而更新。

# 这是什么？
这是一个开源的《黑暗之魂Ⅱ：原罪学者》和《黑暗之魂3》的游戏服务器实现。

该项目旨在提供一种替代方案，让玩家可以在不冒被封号风险的情况下使用MOD进行在线游戏，或者只是想私下玩游戏而不想处理作弊者/入侵等问题。

如果你遇到任何问题，请加入Discord: https://discord.gg/pBmquc9Jkj 来获取技术支持

# 我可以将它用作盗版游戏吗？
不，服务器会验证Steam凭据。不要询问是否支持盗版、Steam模拟器或类似的东西，我们不会支持这些。

FROM SOFTWARE值得您的支持，是他们开发了如此精妙的游戏，请尽可能支持正版游戏。

# 我该去哪里下载它？
您可以在GitHub的发布页面下载： https://github.com/TLeonardUK/ds3os/releases

# 如何使用？

构建完成后，你应该会得到一个名为Bin的文件夹，其中有两个相关子文件夹。Loader和Server。

运行Loader，你将可以选择创建服务器或加入现有服务器。

如果你想自己创建专用服务器，你应该在Server文件夹中运行Server.exe，这将在你的计算机上启动实际的自定义服务器。

服务器首次运行时将生成Saved\default\config.json文件，其中包含各种可以调整的匹配参数（重新启动服务器即可应用配置文件修改）以自定义服务器。

如果需要，服务器也可以通过在Saved\default\config.json中设置密码来设置密码保护，尝试进入受密码保护的服务器时Loader会提示您输入密码。

**注意**：运行**Server.exe**时必须安装**steam**客户端（无需登录）。否则，**Server.exe**将无法初始化。

# 目前有什么功能？
现在大部分的游戏核心功能都可以使用，但是与官方游戏服务器可能有一些差异。我们目前正在寻求更接近官方服务器的行为，并为非官方服务器的运行做出一些普遍改进。

:bangbang: 对《黑暗之魂2》的支持目前处于试验阶段，服务器有很大的可能不会正常运行。

| 功能                    | 黑暗之魂III            | 黑暗之魂2：原罪学者         |
|-----------------------|--------------------|--------------------|
| 稳定到可以使用               | :heavy_check_mark: | 实验性的               |
| 网络传输                  | :heavy_check_mark: | :heavy_check_mark: |
| 公告消息                  | :heavy_check_mark: | :heavy_check_mark: |
| 个人资料管理                | :heavy_check_mark: | :heavy_check_mark: |
| 血迹信息                  | :heavy_check_mark: | :heavy_check_mark: |
| 血迹                    | :heavy_check_mark: | :heavy_check_mark: |
| 灵体                    | :heavy_check_mark: | :heavy_check_mark: |
| 召唤                    | :heavy_check_mark: | :heavy_check_mark: |
| 入侵                    | :heavy_check_mark: | :heavy_check_mark: |
| 自动召唤（契约）              | :heavy_check_mark: | :heavy_check_mark: |
| 镜之骑士                  | n/a                | :heavy_check_mark: |
| 匹配                    | :heavy_check_mark: | :heavy_check_mark: |
| 排行榜                   | :heavy_check_mark: | :heavy_check_mark: |
| 敲钟                    | :heavy_check_mark: | n/a                |
| 快速比赛（竞技场）             | :heavy_check_mark: | :heavy_check_mark: |
| 遥测/杂项 Telemetry/Misc  | :heavy_check_mark: | :heavy_check_mark: |
| 凭据认证                  | :heavy_check_mark: | :heavy_check_mark: |
| 主服务器支持                | :heavy_check_mark: | :heavy_check_mark: |
| Loader支持              | :heavy_check_mark: | :heavy_check_mark: |
| Web管理面板               | :heavy_check_mark: | :heavy_check_mark: |
| 分片支持 Sharding Support | :heavy_check_mark: | :heavy_check_mark: |
| Discord Activity Feed | :heavy_check_mark: |                    |

未来路线图：

- 支持每个服务器的各种MOD设置（例如，允许服务器删除召唤限制）
- 更好的反作弊（我们可能会进行比万代更严格的检查）

# 这会导致在官方服务器上被封号吗？
DSOS使用自己的保存文件，只要你不将ds3os保存文件复制回你的官方服务器保存文件，应该就没问题。

# 常见问题解答
# 如何使服务器在黑暗之魂2和黑暗之魂3之间切换？
首次运行服务器后，您会看到Saved/default/config.json。修改这个配置文件，设置GameType参数为DarkSouls2和DarkSouls3之间您希望的那一个，以更改服务器托管的游戏。

## 我的存档为什么消失了？
DSOS使用自己的存档文件来避免与官方服务器保存文件的任何问题。如果你想将将官方服务器的存档文件转移到DSOS，请点击Loader底部的设置（齿轮）图标，然后单击复制原版存档按钮。

我们不提供将ds3os存档复制回原版存档的自动化选项，以保证安全。如果你~真的~想这样做，你可以找到保存文件存储的文件夹，并将.ds3os文件重命名为.sl2。

## 我可以通过docker运行服务器吗
是的，目前为DSOS发布了两个docker容器，每次发布新版本时都会自动更新：

timleonarduk/ds3os - 这是游戏服务器，大部分情况下你需要的是这个。
timleonarduk/ds3os-master - 这是主服务器，除非你正在fork ds3os，否则你可能不需要这个。

如果你想快速运行服务器，可以使用这个单行命令。注意，它将Saved文件夹挂载到主机文件系统上的/opt/ds3os/Saved，使修改配置文件更容易。您可以访问/opt/ds3os/Saved查看和修改配置文件。

`sudo mkdir -p /opt/ds3os/Saved && sudo chown 1000:1000 /opt/ds3os/Saved && sudo docker run -d -m 2G --restart always --net host --mount type=bind,source=/opt/ds3os/Saved,target=/opt/ds3os/Saved timleonarduk/ds3os:latest`

## 我启动了游戏，但无法连接到服务器？
这种情况有很多不同的原因，最简单的一个是确保你以管理员身份运行，启动器需要修补游戏的内存，以便它连接到新服务器，这需要管理员权限。

如果服务器由你自己托管，上述方法没有解决你的问题，请尝试以下步骤：

1. 确保在你的路由器上转发这些端口，并同时允许tcp和udp协议通过：50000, 50010, 50050, 50020

2. 确保你已经允许服务器通过Windows Defender防火墙，你可以在这里设置规则：开始菜单 -> Windows管理工具 -> Windows Defender防火墙高级安全性 -> 入站/出站规则

3. 可能你没有正确设置服务器的配置。首次运行服务器后，请确保打开配置文件（Saved/config.json），并确保它设置正确（程序会尝试自动配置，但如果你有多个网络适配器，可能会得到错误的值）。最需要正确设置的关键设置是ServerHostname和ServerPrivateHostname，这些应该分别设置为你的WAN IP（从像https://whatismyip.com这样的网站获得的IP）和你的LAN IP（通过运行ipconfig获得的IP）。如果你正在使用虚拟局域网软件（例如hamachi），你需要将这些设置为适当的hamachi IP。

## 配置文件中的所有属性都是什么意思？
这些设置都在这个文件的源代码中有文档记录，将来我会编写更详细的文档。

https://github.com/TLeonardUK/ds3os/blob/main/Source/Server/Config/RuntimeConfig.h

# 如何构建？
目前，该项目使用Visual Studio 2022和C++17进行编译。

我们使用cmake生成项目文件。你可以使用cmake前端生成项目文件，或者你可以使用Tools文件夹中的generate_* shell脚本之一。

生成项目文件后，它们存储在intermediate文件夹中，此时你可以打开并构建项目。

## 使用 nix

```sh
# 打包构建
nix build github:TLeonardUK/ds3os
# 直接运行
nix run github:TLeonardUK/ds3os
# 运行主服务器
nix run github:TLonardUK/ds3os#master-server
```

nix版本将配置存储在 `${XDG_CONFIG_HOME:-$HOME/.config}/ds3os`

# 仓库里都有些什么？
```
/
├── Protobuf/              包含服务器网络流量使用的protobuf定义。通过Tools/中的bat文件进行编译。
├── Resources/             用于构建和打包的一般资源例如图标/自述文件等。
├── Source/                项目的所有源代码。
│   ├── Injector/          这是注入到游戏中以提供DS3OS功能的DLL。
│   ├── Loader/            简单的winforms应用程序，加载DS3以便连接到自定义服务器。
│   ├── MasterServer/      NodeJS源代码，用于列出所有公开活动服务器的简单API服务器。
│   ├── Server/            主服务器的源代码。
│   ├── Server.DarkSouls3/ 专为黑暗之魂3提供支持的源代码。
│   ├── Server.DarkSouls2/ 专为黑暗之魂2提供支持的源代码。
│   ├── Shared/            服务器和注入器项目之间共享的源代码。
│   └── ThirdParty/        使用的任何第三方库的源代码。
│   └── WebUI/             包含用于管理服务器的网页后台的静态资源。
├── Tools/                 用于分析的各种ce表、bat文件等。
```

# 我可以做些什么？
查看我们的 issues 页面，或者给我发消息，我会建议您做一些有用的事情。

目前，有一些服务器调用我们要么舍弃了它们，要么返回虚假的信息。正确实现它们，或者找出它们需要返回的数据格式会对我们有所帮助。

还有很多protobuf字段仍然未知，处理它们的时候服务器只会发送恒定值，确定它们代表什么将是一个很好的改进。

# 贡献者
这个实现所需的许多信息是由社区提供的。
尤其是 ?ServerName? 黑暗之魂系列mod开发discord频道成员.

下面的3个仓库提供了许多在这个实现中使用的信息：

https://github.com/garyttierney/ds3-open-re

https://github.com/Jellybaby34/DkS3-Server-Emulator-Rust-Edition

https://github.com/AmirBohd/ModEngine2

同样的，下面的贡献值为我们提供了软件图标和插图：

篝火图标由 www.flaticon.com 的 ultimatearm 制作

从 http://www.famfamfam.com/lab/icons/silk/ 获取的 Mark James 制作的多种UI图标
