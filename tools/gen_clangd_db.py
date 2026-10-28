#!/usr/bin/env python3
"""从 build-idf/compile_commands.json 生成 clangd 专用数据库。

背景
----
ESP-IDF 把 16 个 .ino 拼进 main.cpp 当作一个编译单元，所以
build-idf/compile_commands.json 里只有 main/main.cpp 一条命令。
clangd 单独打开 cli.ino 时找不到对应条目，会退化成空命令，于是：

  * "expected exactly one compiler job in ''"   （命令为空）
  * 'Arduino.h' file not found                  （没有 -I 路径）
  * pid.h 里 'template' / 'class' 未知类型      （被当成 C 解析）

做法
----
取 main.cpp 那条命令，去掉 -c / -o / 输入文件，展开 @响应文件，
然后为每个 .ino 各生成一条，并补上：

  -xc++                          按 C++ 解析
  -DCLANGD_PART=n                告诉 ino_prefix.h 当前是第几段
  -include clangd/ino_prefix.h   把"前面"的片段先包含进来

用法
----
  python tools/gen_clangd_db.py

然后在 VS Code 命令面板执行 "clangd: Restart language server"。

注意：只影响编辑器提示，不参与固件构建。
"""

import json
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DB = os.path.join(ROOT, "build-idf", "compile_commands.json")
OUT_DB = os.path.join(ROOT, "clangd", "compile_commands.json")

# 必须与 main/main.cpp 的 include 顺序一致
ORDER = [
    "CF-Drone", "battery", "cli", "control", "estimate", "imu",
    "led", "log", "mavlink", "motors", "parameters", "rc",
    "safety", "time", "web_rc", "wifi",
]


def split_windows_cmd(cmd):
    """按 Windows 命令行规则拆分：空格分隔，双引号包裹整体，无转义。"""
    cmd = cmd.replace('\\"', '"')
    out, cur, in_quote = [], [], False
    for ch in cmd:
        if ch == '"':
            in_quote = not in_quote
            continue
        if ch in " \t" and not in_quote:
            if cur:
                out.append("".join(cur))
                cur = []
            continue
        cur.append(ch)
    if cur:
        out.append("".join(cur))
    return out


def expand_response_files(tokens, base_dir):
    """把 @xxx 响应文件就地展开（IDF 的 toolchain/cxxflags 就是这种）。"""
    out = []
    for t in tokens:
        if t.startswith("@"):
            path = t[1:].strip('"')
            if not os.path.isabs(path):
                path = os.path.join(base_dir, path)
            if os.path.exists(path):
                with open(path, encoding="utf-8", errors="replace") as f:
                    out.extend(split_windows_cmd(f.read()))
                continue
            print("警告：响应文件不存在，已跳过 %s" % path)
        out.append(t)
    return out


def load_main_args():
    if not os.path.exists(BUILD_DB):
        sys.exit("找不到 %s，请先用 ESP-IDF 完整构建一次" % BUILD_DB)

    with open(BUILD_DB, encoding="utf-8") as f:
        db = json.load(f)

    entry = None
    for e in db:
        f_ = e.get("file", "").replace("\\", "/")
        if f_.endswith("main/main.cpp"):
            entry = e
            break
    if entry is None:
        sys.exit("compile_commands.json 里没有 main/main.cpp，请先构建一次")

    base = entry.get("directory", ROOT)
    if entry.get("arguments"):
        args = [a for a in entry["arguments"] if a]
    else:
        args = split_windows_cmd(entry["command"])

    compiler = args[0]
    rest = expand_response_files(args[1:], base)

    flags = []
    skip_next = False
    for a in rest:
        if skip_next:
            skip_next = False
            continue
        if a == "-c":
            continue
        if a == "-o":
            skip_next = True
            continue
        if a.lower().endswith("main.cpp"):
            continue
        # 这些会干扰 clangd 独立解析，去掉
        if a in ("-MD", "-MT", "-MF") or a.startswith("-MF"):
            if a == "-MT":
                skip_next = True
            continue
        flags.append(a)
    return compiler, flags


def main():
    compiler, flags = load_main_args()
    entries = []

    for i, name in enumerate(ORDER):
        src = os.path.join(ROOT, name + ".ino")
        if not os.path.exists(src):
            print("跳过（文件不存在）: %s" % src)
            continue
        src_posix = src.replace("\\", "/")
        cmd = [compiler] + flags + [
            "-xc++",
            "-DCLANGD_PART=%d" % i,
            "-include", "clangd/ino_prefix.h",
            "-c", src_posix,
        ]
        entries.append({
            "directory": ROOT.replace("\\", "/"),
            "file": src_posix,
            "arguments": cmd,
        })

    # ino_prefix.h 是被 -include 强制拉进来的，不属于任何 TU 的包含链，
    # clangd 的头文件推断找不到参数，会退化成空命令（'Arduino.h' 找不到）。
    # 这里单独给它一条，独立打开时也能正常解析。
    prefix = os.path.join(ROOT, "clangd", "ino_prefix.h")
    if os.path.exists(prefix):
        prefix_posix = prefix.replace("\\", "/")
        entries.append({
            "directory": ROOT.replace("\\", "/"),
            "file": prefix_posix,
            "arguments": [compiler] + flags + [
                "-xc++",
                "-DCLANGD_PART=0",
                "-c", prefix_posix,
            ],
        })

    os.makedirs(os.path.dirname(OUT_DB), exist_ok=True)
    with open(OUT_DB, "w", encoding="utf-8") as f:
        json.dump(entries, f, ensure_ascii=False, indent=1)

    print("已写入 %s" % OUT_DB)
    print("共 %d 条" % len(entries))
    print("接下来：VS Code 命令面板 -> clangd: Restart language server")


if __name__ == "__main__":
    main()
