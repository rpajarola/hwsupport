#!/usr/bin/env python3
"""
package.py - Packaging and Release Orchestrator for MVME393 Xcfb

Manages package building, checksumming, verification, and distribution for
the Motorola System V/88 MVME393 X Server.
"""

import os
import sys
import hashlib
import argparse
import subprocess
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
XCFB_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
VERSION_FILE = os.path.join(XCFB_DIR, "VERSION")
PACKAGES_DIR = os.path.join(XCFB_DIR, "packages")
CHECKSUMS_FILE = os.path.join(PACKAGES_DIR, "checksums.txt")
SRC_DIR = os.path.join(XCFB_DIR, "src", "xcfb")
PACKAGING_DIR = os.path.join(XCFB_DIR, "packaging")


def get_version():
    if not os.path.isfile(VERSION_FILE):
        return "1.0.0"
    with open(VERSION_FILE, "r") as f:
        return f.read().strip()


def compute_hashes(filepath):
    sha256 = hashlib.sha256()
    md5 = hashlib.md5()
    with open(filepath, "rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            sha256.update(chunk)
            md5.update(chunk)
    return sha256.hexdigest(), md5.hexdigest()


def update_checksums(version=None):
    if not version:
        version = get_version()

    pkg_name = f"xcfb-{version}-m88k.pkg"
    tar_name = f"xcfb-{version}-m88k.tar.Z"

    pkg_path = os.path.join(PACKAGES_DIR, pkg_name)
    tar_path = os.path.join(PACKAGES_DIR, tar_name)

    if not os.path.isfile(pkg_path) or not os.path.isfile(tar_path):
        print(f"Error: Required package files for version {version} not found in {PACKAGES_DIR}")
        print(f"  Missing: {pkg_name if not os.path.isfile(pkg_path) else ''} "
              f"{tar_name if not os.path.isfile(tar_path) else ''}")
        return False

    pkg_sha, pkg_md5 = compute_hashes(pkg_path)
    tar_sha, tar_md5 = compute_hashes(tar_path)

    lines = [
        f"{pkg_sha}  {pkg_name}\n",
        f"{tar_sha}  {tar_name}\n"
    ]

    with open(CHECKSUMS_FILE, "w") as f:
        f.writelines(lines)

    print(f"Updated {CHECKSUMS_FILE} for version {version}:")
    print(f"  {pkg_name}:")
    print(f"    SHA-256: {pkg_sha}")
    print(f"    MD5:     {pkg_md5}")
    print(f"  {tar_name}:")
    print(f"    SHA-256: {tar_sha}")
    print(f"    MD5:     {tar_md5}")
    return True


def verify_packages(version=None):
    if not version:
        version = get_version()

    pkg_name = f"xcfb-{version}-m88k.pkg"
    tar_name = f"xcfb-{version}-m88k.tar.Z"

    pkg_path = os.path.join(PACKAGES_DIR, pkg_name)
    tar_path = os.path.join(PACKAGES_DIR, tar_name)

    print(f"Verifying packages for version {version}...")

    all_ok = True
    for name, path in [(pkg_name, pkg_path), (tar_name, tar_path)]:
        if not os.path.isfile(path):
            print(f"  [FAIL] File missing: {path}")
            all_ok = False
        else:
            size = os.path.getsize(path)
            print(f"  [OK] Found {name} ({size:,} bytes)")

    if not os.path.isfile(CHECKSUMS_FILE):
        print(f"  [FAIL] Checksums file missing: {CHECKSUMS_FILE}")
        return False

    with open(CHECKSUMS_FILE, "r") as f:
        recorded_hashes = {}
        for line in f:
            parts = line.strip().split()
            if len(parts) >= 2:
                recorded_hashes[parts[1]] = parts[0]

    for name, path in [(pkg_name, pkg_path), (tar_name, tar_path)]:
        if name in recorded_hashes:
            actual_sha, _ = compute_hashes(path)
            expected_sha = recorded_hashes[name]
            if actual_sha.lower() == expected_sha.lower():
                print(f"  [PASS] SHA-256 matches for {name}: {actual_sha[:16]}...")
            else:
                print(f"  [FAIL] SHA-256 mismatch for {name}:")
                print(f"         Expected: {expected_sha}")
                print(f"         Actual:   {actual_sha}")
                all_ok = False
        else:
            print(f"  [WARN] No checksum recorded in {CHECKSUMS_FILE} for {name}")
            all_ok = False

    return all_ok


def build_in_vm(version=None):
    if not version:
        version = get_version()

    print(f"Initiating native VM build & packaging for version {version}...")
    import pty, select

    gxemul_bin = "/home/ricop/src/claude/gxemul-88k/gxemul"
    vm_dir = "/home/ricop/src/claude/88k"
    vm_config = "mvme187.json"
    transfer_img = os.path.join(vm_dir, "transfer.img")

    if not os.path.isfile(gxemul_bin):
        print(f"Error: GXemul binary not found at {gxemul_bin}")
        return False

    # Create / zero transfer image
    subprocess.check_call(["dd", "if=/dev/zero", f"of={transfer_img}", "bs=1M", "count=10", "status=none"])

    master, slave = pty.openpty()
    proc = os.fork()
    if proc == 0:
        os.close(master)
        os.dup2(slave, 0); os.dup2(slave, 1); os.dup2(slave, 2)
        os.close(slave)
        os.chdir(vm_dir)
        os.execv(gxemul_bin, ['gxemul', vm_config])
    os.close(slave)

    def read_until(targets, timeout=180.0):
        buf = ''
        t0 = time.time()
        while time.time() - t0 < timeout:
            r, _, _ = select.select([master], [], [], 0.1)
            if master in r:
                try:
                    c = os.read(master, 1024).decode('latin1', errors='ignore')
                    buf += c
                    for target in targets:
                        if target in buf:
                            return target, buf
                except OSError:
                    break
        return None, buf

    def exec_cmd(cmd, timeout=60.0):
        while True:
            r, _, _ = select.select([master], [], [], 0.05)
            if not r: break
            os.read(master, 1024)
        os.write(master, cmd.encode('latin1') + b'\r')
        target, out = read_until(['GXM88K_PROMPT> '], timeout=timeout)
        return out

    try:
        print("Booting SVR4 guest VM...")
        while True:
            target, _ = read_until(['Enter Menu #:', '187-Diag>', '187-Bug>', 'login:', '# '], timeout=180.0)
            if not target:
                print("Failed to boot guest VM")
                return False
            if target == 'Enter Menu #:':
                time.sleep(0.5); os.write(master, b'3\r')
            elif target == '187-Diag>':
                time.sleep(0.5); os.write(master, b'sd\r')
            elif target == '187-Bug>':
                time.sleep(0.5); os.write(master, b'bo 0\r')
            elif target == 'login:':
                time.sleep(0.5); os.write(master, b'root\r')
            elif target == '# ':
                break

        time.sleep(0.5)
        os.write(master, b'PS1="GXM88K_PROMPT> "\r')
        read_until(['GXM88K_PROMPT> '], timeout=10.0)

        print("Building Xcfb binary inside SVR4 VM...")
        exec_cmd("cd /home/xcfb/xcfb_src && make clean && make -j1", timeout=300.0)

        print("Staging package files...")
        pkg_base = f"xcfb-{version}-m88k"
        exec_cmd(f"rm -rf /tmp/pkgroot /tmp/pkgbuild /tmp/xcfb /tmp/dist /tmp/{pkg_base}.*")
        exec_cmd("mkdir -p /tmp/pkgroot/usr/bin/X11")
        exec_cmd("mkdir -p /tmp/pkgroot/usr/lib/X11")
        exec_cmd("mkdir -p /tmp/pkgbuild")
        exec_cmd("cp /home/xcfb/xcfb_src/Xcfb /tmp/pkgroot/usr/bin/X11/Xcfb")
        exec_cmd("chmod 755 /tmp/pkgroot/usr/bin/X11/Xcfb")

        # Copy packaging scripts
        for script in ["startxcfb", "enable-xdm", "disable-xdm"]:
            host_path = os.path.join(PACKAGING_DIR, script)
            with open(host_path, "r") as f:
                content = f.read()
            exec_cmd(f"cat << 'EOF' > /tmp/pkgroot/usr/bin/X11/{script}\n{content}\nEOF")
            exec_cmd(f"chmod 755 /tmp/pkgroot/usr/bin/X11/{script}")

        # Copy rgb db
        exec_cmd("if [ -f /usr/lib/X11/rgb.dir ]; then cp /usr/lib/X11/rgb.* /tmp/pkgroot/usr/lib/X11/; fi")

        # Package control files
        for cfile in ["pkginfo", "postinstall", "preremove"]:
            host_path = os.path.join(PACKAGING_DIR, cfile)
            with open(host_path, "r") as f:
                content = f.read()
            exec_cmd(f"cat << 'EOF' > /tmp/pkgbuild/{cfile}\n{content}\nEOF")
            exec_cmd(f"chmod 755 /tmp/pkgbuild/{cfile}")

        # Read prototype
        with open(os.path.join(PACKAGING_DIR, "prototype"), "r") as f:
            proto = f.read()
        exec_cmd(f"cat << 'EOF' > /tmp/pkgbuild/prototype\n{proto}\nEOF")

        print("Running pkgmk and pkgtrans...")
        exec_cmd("cd /tmp/pkgbuild && pkgmk -o -r /tmp/pkgroot -d /tmp -f /tmp/pkgbuild/prototype")
        exec_cmd(f"pkgtrans -s /tmp /tmp/{pkg_base}.pkg xcfb")

        print("Creating standalone distribution archive...")
        exec_cmd(f"mkdir -p /tmp/dist/{pkg_base}")
        exec_cmd(f"cp /tmp/pkgroot/usr/bin/X11/* /tmp/dist/{pkg_base}/")
        exec_cmd(f"if [ -f /tmp/pkgroot/usr/lib/X11/rgb.dir ]; then cp /tmp/pkgroot/usr/lib/X11/rgb.* /tmp/dist/{pkg_base}/; fi")
        exec_cmd(f"cp /tmp/{pkg_base}.pkg /tmp/dist/{pkg_base}/")

        with open(os.path.join(PACKAGING_DIR, "install.sh"), "r") as f:
            inst = f.read()
        exec_cmd(f"cat << 'EOF' > /tmp/dist/{pkg_base}/install.sh\n{inst}\nEOF")
        exec_cmd(f"chmod 755 /tmp/dist/{pkg_base}/install.sh")

        exec_cmd(f"cd /tmp/dist && tar cvf /tmp/{pkg_base}.tar {pkg_base}")
        exec_cmd(f"compress -f /tmp/{pkg_base}.tar")

        print("Writing build artifacts to transfer disk...")
        exec_cmd(f"cd /tmp && tar cvf /dev/rdsk/m187_c0d1s7 {pkg_base}.pkg {pkg_base}.tar.Z")
        exec_cmd("sync; sync; sync")

        print("Halting VM...")
        exec_cmd("/sbin/uadmin 2 0")
        time.sleep(3)
    finally:
        try: os.close(master)
        except: pass
        try:
            os.kill(proc, 9)
            os.waitpid(proc, 0)
        except: pass

    # Extract artifacts on host
    print(f"Extracting artifacts from {transfer_img}...")
    subprocess.check_call(["tar", "xvf", transfer_img], cwd=PACKAGES_DIR)

    # Update checksums
    update_checksums(version)
    print(f"\nBuild and packaging of {pkg_base} completed successfully!")
    return True


def main():
    parser = argparse.ArgumentParser(description="MVME393 Xcfb Packaging Orchestrator")
    parser.add_argument("--version", help="Override version (defaults to VERSION file)")
    parser.add_argument("--verify", action="store_true", help="Verify package files and checksums")
    parser.add_argument("--checksums", action="store_true", help="Recalculate and update checksums.txt")
    parser.add_argument("--build-vm", action="store_true", help="Perform full native compile and packaging in VM")

    args = parser.parse_args()
    ver = args.version or get_version()

    if args.verify:
        success = verify_packages(ver)
        sys.exit(0 if success else 1)
    elif args.checksums:
        success = update_checksums(ver)
        sys.exit(0 if success else 1)
    elif args.build_vm:
        success = build_in_vm(ver)
        sys.exit(0 if success else 1)
    else:
        # Default behavior: verify current packages
        print(f"Xcfb Version: {ver}")
        verify_packages(ver)


if __name__ == "__main__":
    main()
