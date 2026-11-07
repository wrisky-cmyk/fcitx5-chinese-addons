# 码表输入法 GB2312 字符集过滤

为 fcitx5 码表引擎（五笔/仓颉/二笔等）增加候选字符集过滤：默认只显示
GB2312 字库（6763 字）内的候选，通过快捷键或状态栏开关临时放开，允许输入
GBK/GB18030 及扩展区的生僻字。实现跨越两个仓库：libime（词库级过滤钩子）
与 fcitx5-chinese-addons（table 引擎接线）。

## 功能说明

- **默认开启过滤**：候选、唯一候选自动上屏判定、联想词、码表内拼音反查
  全部按 GB2312 过滤。
- **Ctrl+Alt+G** 切换（可在配置中修改）：放开后词库中的全部字符
  （GBK/GB18030 及扩展区）都可用；再按一次恢复。普通输入与拼音反查
  模式下候选列表立即刷新；其它模式从下一次按键起生效。
- 中文码表（zh_*）的状态栏有开关图标与文字（GB2312/Extended），托盘
  右键菜单里也有开关项；切换时有气泡提示，状态持久化在
  `~/.config/fcitx5/conf/table.conf`。
- 反查编码（reverseLookup）与删词（wordExists）不受过滤影响，生僻字仍可
  通过查编码/删词管理。
- 对该引擎下所有码表（五笔/仓颉/二笔/自然码等）统一生效，开关全局共享。

## 构建

### 依赖

- fcitx5 ≥ 5.1.22 开发包、ECM、Boost（头文件）、cmake
- 本分支的 libime（新增 `TableBasedDictionary::setWordFilter` API，
  fcitx5-chinese-addons 已将依赖提升到 1.1.18）

### 1. 构建并安装 libime（必须先装）

```bash
cd ~/libime
git submodule update --init        # 首次构建需要（kenlm 子模块）
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --install build
sudo ldconfig
```

> - **必须在配置（cmake -B）时指定 `-DCMAKE_INSTALL_PREFIX=/usr`**，
>   之后再安装时不要用 `--prefix` 改来改去。libime 生成的 CMake 导出
>   文件里烤死了配置时的 include 路径：配置时是默认 /usr/local 的话，
>   即使安装时 `--prefix /usr`，下游编译也会报
>   `libime/core/datrie.h: 没有那个文件或目录`。踩到这个坑的解法：
>   重新 `cmake -B build -DCMAKE_INSTALL_PREFIX=/usr` 后重装。
> - 不带 sudo 会报 `Permission denied`。
> - 装到 /usr 会覆盖 pacman 的 libime 包（1.1.17）；SONAME 不变
>   （`libIMETable.so.0`），纯增量 API，系统里旧的
>   fcitx5-chinese-addons 二进制照样兼容。不要装 /usr/local：Arch 的
>   `/etc/ld.so.conf` 不含 `/usr/local/lib`，运行时加载不到库。

### 2. 构建 fcitx5-chinese-addons

libime 已装到 /usr，cmake 会自动找到，无需 `CMAKE_PREFIX_PATH`：

```bash
cd ~/fcitx5-chinese-addons
# 没有 nlohmann-json 时先装：sudo pacman -S nlohmann-json
# 不想装则加 -DENABLE_OPENCC=OFF -DENABLE_CLOUDPINYIN=OFF
rm -rf build     # 换过前缀/换过 libime 后务必清掉旧缓存
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)
sudo cmake --install build
```

如果之前误装过 /usr/local，先清掉残留（XDG 路径里它排在 /usr 前面，
会遮蔽系统配置）：

```bash
sudo rm -rf /usr/local/share/fcitx5 /usr/local/include/Fcitx5 \
            /usr/local/include/LibIME /usr/local/lib/cmake/Fcitx5Module* \
            /usr/local/lib/fcitx5
```

> **中途重新 `cmake -B` 时必须每次都带 `-DCMAKE_INSTALL_PREFIX=/usr`**：
> 不带的话前缀回落为默认 /usr/local，`cmake --install` 会把插件装进
> `/usr/local/lib/fcitx5/`——系统 fcitx5 只从 `/usr/lib/fcitx5/` 加载，
> 看起来就是"装了没生效"。

**为什么安装 chinese-addons 要 `--prefix /usr`**：fcitx5 核心只在它编译时的
插件库目录（`/usr/lib/fcitx5/`）加载插件。装到 /usr/local 的话，插件配置
虽然能被读到（XDG 路径），但库仍然会解析到系统旧版 `libtable.so`，过滤
不会生效。`--prefix /usr` 直接替换系统文件，最省事。

> 注意：这会覆盖 pacman 拥有的文件，下次 pacman 升级这两个包时会提示
> 文件冲突。本地开发自用可接受（升级后重新 install 一次即可）；长期
> 使用建议打补丁包（见下文"打包"）。

不想动系统文件的话，也可以构建后手动只覆盖插件库：

```bash
sudo cp build/lib/fcitx5/libtable.so /usr/lib/fcitx5/libtable.so
```

### 3. 生效与验证

```bash
fcitx5 -r          # 重启 fcitx5（或注销重登）
```

- 五笔 86 输入 `yyff`：默认只出"文坛"（重码的"堃"是 GBK 字，被过滤）；
  按 Ctrl+Alt+G 后出"堃 文坛"。
- 输入 `qpwk`：默认无候选（鎔/镕 都是超集字）；切换后出"鎔 镕"。
- 状态栏出现 GB2312/GBK 开关（中文码表下）；重启 fcitx5 后状态保持。
- 配置入口：fcitx5 配置 → 附加组件 → 码表 → 全局选项，可改快捷键、
  关闭默认过滤。

## 实现摘要

| 仓库 | 位置 | 内容 |
|---|---|---|
| libime | `src/libime/table/tablebaseddictionary.{h,cpp,_p.h}` | `TableWordFilter` + `setWordFilter()`；`matchWordsInternal` 与组词规则路径（`matchPrefixImpl` 的 ConstructPhrase 节点）统一过滤，覆盖候选/唯一候选判定/自动上屏/hasMatchingWords；不影响 reverseLookup/wordExists |
| libime | `test/testtable.cpp` | `testWordFilter` 单测 |
| chinese-addons | `im/table/gbcharset.{h,cpp}`、`gb2312table.h` | GB2312 码点判定（CJK 表意区查 6763 字位图，其余字符放行） |
| chinese-addons | `im/table/gen-gb2312table.py` | 位图生成脚本（Python `gb2312` codec，输出可复现） |
| chinese-addons | `im/table/engine.{h,cpp}` | 配置项 `GB2312Only`/`GB2312Hotkey`、状态栏动作（图标+文字）、切换逻辑与提示 |
| chinese-addons | `icon/scalable/apps/fcitx-gb2312-*.svg` | 开关图标两态（GB2312=过滤中 / Extended=放开）及 symbolic 变体 |
| chinese-addons | `im/table/ime.{h,cpp}` | `TableIME::setWordFilter` 向所有已加载词库下发 |
| chinese-addons | `im/table/state.cpp` | 快捷键拦截、联想/候选显示过滤（切换立即生效）、拼音反查过滤 |

设计要点：过滤放在 **libime 词典匹配层**而不是引擎展示层，保证最佳句子
与自动上屏判定一致（码表有唯一候选自动上屏，纯展示过滤会漏网）。

## 测试

```bash
# libime 侧
ctest --test-dir ~/libime/build -R testtable
# chinese-addons 侧（注意：上游 testtable 在部分环境下因系统 wbx 表数据
# 与测试期望不匹配而失败，属预先存在的问题，与本功能无关）
ctest --test-dir ~/fcitx5-chinese-addons/build -R testtable
```

## 上游更新维护

改动以两个仓库各一个独立 commit 维护（fork：wrisky-cmyk/libime、
wrisky-cmyk/fcitx5-chinese-addons）。

```bash
git remote add upstream https://github.com/fcitx/libime.git       # 另一仓库同理
git fetch upstream
git rebase upstream/master        # 或 upstream/main
```

冲突热点：

- libime `tablebaseddictionary.cpp` 的 `matchWordsInternal`（上游重构匹配
  路径时）；其余为纯增量，低风险。
- chinese-addons：`engine.h` 的 `TableGlobalConfig`、`state.cpp` 的
  `keyEvent`/`updateUI` 热区、顶层 `CMakeLists.txt` 版本行。
- `po/zh_CN.po` 每次必冲突：rebase 时可
  `git checkout --theirs po/zh_CN.po` 后重新追加新增的 8 条，或直接放弃
  po 改动交由上游翻译流程（功能不受影响，仅界面暂显英文）。

Rebase 后回归：

```bash
ctest --test-dir ~/libime/build -R testtable
cmake --build ~/fcitx5-chinese-addons/build --target table testtable
```

`gb2312table.h` 为固定数据（GB2312 字表不会变），无需随上游更新。

### 建议推上游

本功能适合上游合并（带单测、按上游惯例实现、约 200 行）。推到 fork 后
向上游 fcitx5-chinese-addons 与 libime 各开 PR。上游可能反馈的点：
默认值（可能要求默认关闭或首迁提示）、是否做成 gb2312/gbk 枚举分级。

## 打包

导出补丁供 PKGBUILD 使用（libime 与 chinese-addons 需成对升级，
chinese-addons 依赖 libime 1.1.18 的新符号）：

```bash
cd ~/libime && git format-patch -1 -o ~/patches/
cd ~/fcitx5-chinese-addons && git format-patch -1 -o ~/patches/
```
