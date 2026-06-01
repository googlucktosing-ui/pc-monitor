#!/usr/bin/env python3
"""
版本号同步工具
用法: python sync_version.py [新版本号]
      python sync_version.py 1.0.1
"""

import os, sys, subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
VER_FILE = ROOT / "VERSION"
VERSION_H = ROOT / "pc_monitor" / "main" / "version.h"
PC_SERVER_PY = ROOT / "pc_server" / "pc_server.py"

def read_version():
    with open(VER_FILE, encoding="utf-8-sig") as f:
        return f.read().strip()

def write_version(v):
    with open(VER_FILE, "w", encoding="utf-8") as f:
        f.write(v + "\n")
    print(f"  [OK] VERSION -> {v}")

def sync_version_h(v):
    content = (
        "// Auto-generated from VERSION file - DO NOT EDIT\n"
        "#ifndef VERSION_H\n#define VERSION_H\n"
        f'#define APP_VERSION "{v}"\n'
        "#endif\n"
    )
    with open(VERSION_H, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"  [OK] version.h -> {v}")

def main():
    old_ver = read_version()
    print(f"当前版本: v{old_ver}")
    print()

    if len(sys.argv) >= 2:
        new_ver = sys.argv[1].strip()
    else:
        new_ver = input(f"新版本号 (当前 v{old_ver}): ").strip()
        if not new_ver:
            print("取消")
            return

    if new_ver == old_ver:
        print("版本号未变化，无需操作")
        return

    print(f"\n正在更新 v{old_ver} -> v{new_ver} ...")
    write_version(new_ver)
    sync_version_h(new_ver)

    print(f"\n更新完成! v{old_ver} -> v{new_ver}")
    print()
    print("接下来提交到 Git:")
    print(f"  git add VERSION pc_monitor/main/version.h")
    print(f'  git commit -m "chore: bump to v{new_ver}"')
    print(f"  git tag v{new_ver}")
    print("  git push && git push --tags")
    print()

    yn = input("现在自动执行 git 提交? (y/N): ").strip().lower()
    if yn == "y":
        cmds = [
            ["git", "add", str(VER_FILE), str(VERSION_H)],
            ["git", "commit", "-m", f"chore: bump to v{new_ver}"],
            ["git", "tag", f"v{new_ver}"],
        ]
        for cmd in cmds:
            r = subprocess.run(cmd, capture_output=True, text=True)
            if r.returncode != 0:
                print(f"  [!] {' '.join(cmd)} 失败: {r.stderr.strip()}")
            else:
                print(f"  [OK] {' '.join(cmd)}")
        print("Git 操作完成! 别忘了 git push && git push --tags")
    else:
        print("跳过 Git 操作，手动执行即可")

if __name__ == "__main__":
    main()
