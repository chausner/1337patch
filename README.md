# 1337patch
Simple command-line tool to apply patches exported by x64dbg to running processes

[![license](https://img.shields.io/github/license/chausner/1337patch.svg)](https://github.com/chausner/1337patch/blob/master/LICENSE.md)

Usage
-----
```
Usage:
  1337patch <patch file> <process name> [options]
  1337patch <patch file> -pid <pid> [options]

  patch file:   path to a x64dbg .1337 patch file
  process name: image name of the process to patch
  pid:          process ID of the process to patch

Allowed options:
  -revert: revert previously applied patches
  -force:  apply/revert patches even if patch locations contain unexpected data
```

Download
--------
[Latest release](https://github.com/chausner/1337patch/releases/latest)
