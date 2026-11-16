#!/usr/bin/env python3
"""
bump_version.py - Version management for MVME393 Xcfb Server

Manages canonical version in VERSION, syncs to:
  - packaging/pkginfo
  - src/xcfb/xcfb/xcfb_version.h
  - README.md & documentation
"""

import os
import sys
import re
import argparse
import subprocess

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
XCFB_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
VERSION_FILE = os.path.join(XCFB_DIR, "VERSION")
PKGINFO_FILE = os.path.join(XCFB_DIR, "packaging", "pkginfo")
VERSION_H_FILE = os.path.join(XCFB_DIR, "src", "xcfb", "xcfb", "xcfb_version.h")
README_FILE = os.path.join(XCFB_DIR, "README.md")


def get_current_version():
    if not os.path.isfile(VERSION_FILE):
        return "1.0.0"
    with open(VERSION_FILE, "r") as f:
        return f.read().strip()


def parse_semver(ver_str):
    m = re.match(r"^(\d+)\.(\d+)\.(\d+)$", ver_str.strip())
    if not m:
        raise ValueError(f"Version '{ver_str}' is not valid SemVer (expected X.Y.Z)")
    return int(m.group(1)), int(m.group(2)), int(m.group(3))


def bump(ver_str, part):
    major, minor, patch = parse_semver(ver_str)
    if part == "major":
        return f"{major + 1}.0.0"
    elif part == "minor":
        return f"{major}.{minor + 1}.0"
    elif part == "patch":
        return f"{major}.{minor}.{patch + 1}"
    else:
        # Explicit version string provided
        parse_semver(part)
        return part.strip()


def sync_version(new_ver):
    major, minor, patch = parse_semver(new_ver)

    # 1. Update VERSION file
    print(f"Writing {new_ver} to {os.path.relpath(VERSION_FILE, XCFB_DIR)}")
    with open(VERSION_FILE, "w") as f:
        f.write(new_ver + "\n")

    # 2. Update packaging/pkginfo
    if os.path.isfile(PKGINFO_FILE):
        print(f"Updating {os.path.relpath(PKGINFO_FILE, XCFB_DIR)}")
        with open(PKGINFO_FILE, "r") as f:
            content = f.read()
        new_content = re.sub(r'VERSION="[^"]*"', f'VERSION="{new_ver}"', content)
        with open(PKGINFO_FILE, "w") as f:
            f.write(new_content)

    # 3. Update src/xcfb/xcfb/xcfb_version.h
    print(f"Updating {os.path.relpath(VERSION_H_FILE, XCFB_DIR)}")
    version_h_content = f"""/*
 * xcfb_version.h - Version definitions for Motorola MVME393 X Server
 */

#ifndef _XCFB_VERSION_H_
#define _XCFB_VERSION_H_

#define XCFB_VERSION_MAJOR {major}
#define XCFB_VERSION_MINOR {minor}
#define XCFB_VERSION_PATCH {patch}
#define XCFB_VERSION_STRING "{new_ver}"

#endif /* _XCFB_VERSION_H_ */
"""
    os.makedirs(os.path.dirname(VERSION_H_FILE), exist_ok=True)
    with open(VERSION_H_FILE, "w") as f:
        f.write(version_h_content)

    print(f"Successfully synchronized Xcfb version to {new_ver}")


def main():
    parser = argparse.ArgumentParser(description="MVME393 Xcfb Version Manager")
    parser.add_argument("action", nargs="?", default="show",
                        help="Action: show | major | minor | patch | <X.Y.Z>")
    parser.add_argument("--commit", action="store_true",
                        help="Create a git commit with the version bump")
    parser.add_argument("--tag", action="store_true",
                        help="Create a git tag for the new version")

    args = parser.parse_args()
    curr_ver = get_current_version()

    if args.action == "show":
        print(curr_ver)
        return

    new_ver = bump(curr_ver, args.action)
    print(f"Bumping version: {curr_ver} -> {new_ver}")
    sync_version(new_ver)

    if args.commit:
        cmd = ["git", "add", VERSION_FILE, PKGINFO_FILE, VERSION_H_FILE]
        subprocess.check_call(cmd, cwd=XCFB_DIR)
        commit_msg = f"xcfb: bump version to {new_ver}"
        subprocess.check_call(["git", "commit", "-m", commit_msg], cwd=XCFB_DIR)
        print(f"Committed: {commit_msg}")

    if args.tag:
        tag_name = f"xcfb-v{new_ver}"
        subprocess.check_call(["git", "tag", "-a", tag_name, "-m", f"Xcfb release {new_ver}"], cwd=XCFB_DIR)
        print(f"Created git tag: {tag_name}")


if __name__ == "__main__":
    main()
