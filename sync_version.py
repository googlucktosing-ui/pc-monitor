#!/usr/bin/env python3
"""
版本号同步工具
用法:
  python sync_version.py             查看当前版本
  python sync_version.py 1.0.1       更新到指定版本
  python sync_version.py 1.0.1 --git 更新 + 自动 git 提交 + 打标签
"""

import os, sys, subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent
VER_FILE = ROOT / "VERSION"
VERSION_H = ROOT / "pc_monitor" / "main" / "version.h"

def read_version():
    with open(VER_FILE, encoding="utf-8-sig") as f:
        return f.read().strip()

def write_version(v):
    with open(VER_FILE, "w", encoding="utf-8") as f:
        f.write(v + "\n")
    print(f"  VERSION -> {v}")

def sync_version_h(v):
    content = (
        "// Auto-generated from VERSION file - DO NOT EDIT\n"
        "#ifndef VERSION_H\n#define VERSION_H\n"
        f'#define APP_VERSION "{v}"\n'
        "#endif\n"
    )
    with open(VERSION_H, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"  version.h -> {v}")

def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    do_git = "--git" in sys.argv

    old_ver = read_version()
    print(f"当前版本: v{old_ver}")

    if not args:
        print("\n用法: python sync_version.py <新版本号> [--git]")
        print("  python sync_version.py 1.0.1      仅更新文件")
        print("  python sync_version.py 1.0.1 --git  更新 + git 提交 + 打标签")
        return

    new_ver = args[0].strip()
    if new_ver == old_ver:
        print("版本号未变化，无需操作")
        return

    print(f"\n更新: v{old_ver} -> v{new_ver}")
    write_version(new_ver)
    sync_version_h(new_ver)
    print("完成!\n")

    if do_git:
        cmds = [
            ["git", "add", str(VER_FILE), str(VERSION_H)],
            ["git", "commit", "-m", f"chore: bump to v{new_ver}"],
            ["git", "tag", f"v{new_ver}"],
        ]
        for cmd in cmds:
            r = subprocess.run(cmd, capture_output=True, text=True)
            if r.returncode != 0:
                print(f"  [!] {' '.join(cmd)}")
                print(f"      {r.stderr.strip()}")
            else:
                print(f"  [OK] {' '.join(cmd)}")
        print(f"\n别忘了推送: git push && git push --tags")
    else:
        print("下一步 Git 操作:")
        print(f"  git add VERSION pc_monitor/main/version.h")
        print(f'  git commit -m "chore: bump to v{new_ver}"')
        print(f"  git tag v{new_ver}")
        print(f"  git push && git push --tags")

if __name__ == "__main__":
    main()
