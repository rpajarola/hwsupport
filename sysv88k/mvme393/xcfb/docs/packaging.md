# MVME393 X Server (Xcfb) Packaging & Versioning Infrastructure

This document details the versioning workflow, package formats, and automation tools for the Motorola MVME393 X11 server (`Xcfb`) on Motorola System V/88 Release 4 (m88k SVR4).

---

## 1. Versioning Architecture

The project adheres to [Semantic Versioning (SemVer 2.0.0)](https://semver.org/): `MAJOR.MINOR.PATCH`.

### Single Source of Truth
* **`VERSION`**: A plain text file located at the root of `xcfb/` containing the canonical version string (e.g. `1.0.0`).

### Synchronized Components
Whenever the version changes, the following files are synchronized:
1. **`VERSION`**: Contains `X.Y.Z`.
2. **`packaging/pkginfo`**: SVR4 package description containing `VERSION="X.Y.Z"`.
3. **`src/xcfb/xcfb/xcfb_version.h`**: C header defining `XCFB_VERSION_MAJOR`, `XCFB_VERSION_MINOR`, `XCFB_VERSION_PATCH`, and `XCFB_VERSION_STRING`.
4. **`packages/xcfb-X.Y.Z-m88k.pkg`**: SVR4 datastream package.
5. **`packages/xcfb-X.Y.Z-m88k.tar.Z`**: Standalone distribution tarball.
6. **`packages/checksums.txt`**: SHA-256 and MD5 checksum records.

---

## 2. Automation Scripts

The `scripts/` directory provides Python utilities for managing versions and package artifacts:

### `scripts/bump_version.py`
Inspects and updates the canonical version across all components.

```bash
# Query the current version
python3 scripts/bump_version.py show

# Bump the patch level (e.g., 1.0.0 -> 1.0.1)
python3 scripts/bump_version.py patch

# Bump minor or major level
python3 scripts/bump_version.py minor
python3 scripts/bump_version.py major

# Set an explicit version
python3 scripts/bump_version.py 1.1.0

# Bump version, commit changes, and create git tag
python3 scripts/bump_version.py patch --commit --tag
```

### `scripts/package.py`
Orchestrates package verification, checksum computation, and full native compilation inside the Motorola SVR4 VM.

```bash
# Verify integrity of package files against packages/checksums.txt
python3 scripts/package.py --verify

# Recompute and update packages/checksums.txt for current version
python3 scripts/package.py --checksums

# Run full native SVR4 compile, packaging, and extraction via VM
python3 scripts/package.py --build-vm
```

---

## 3. SVR4 Package Structure (`pkgmk` & `pkgtrans`)

The official package follows the AT&T / USL System V Release 4 Application Binary Interface (ABI) datastream packaging format.

### Control Files (`packaging/`)
* **`pkginfo`**: Package identification (name, version, architecture, vendor).
* **`prototype`**: Manifest of files to package with target installation paths, permissions, and ownership.
* **`postinstall`**: Executed automatically by `pkgadd` after file extraction. Sets up `/dev/memregion/12` (major 19, minor 140), sticky permissions on `/tmp/.X11-unix`, and the `/usr/bin/X11/X` symlink.
* **`preremove`**: Executed automatically by `pkgrm` before file removal. Shuts down active `Xcfb` and `xdm` processes and cleans up symlinks.

### Package Creation Pipeline
Inside the SVR4 environment:
```sh
# 1. Create file system directory tree package
cd /tmp/pkgbuild && pkgmk -o -r /tmp/pkgroot -d /tmp -f prototype

# 2. Convert filesystem package into single-file datastream
pkgtrans -s /tmp /tmp/xcfb-X.Y.Z-m88k.pkg xcfb
```

---

## 4. Standalone Distribution Tarball

For systems without `pkgadd` or for manual installation, the `xcfb-X.Y.Z-m88k.tar.Z` archive provides:
* Precompiled `Xcfb` binary (stripped, dynamically linked against SVR4 libc, libX11, libm).
* Supporting scripts: `startxcfb`, `enable-xdm`, `disable-xdm`, `install.sh`.
* X11 RGB color database (`rgb.dir`, `rgb.pag`).
* Single-step automated installer: `sh install.sh` (or `sh install.sh --enable-xdm`).
