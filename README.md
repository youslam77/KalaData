# KalaData

[![License](https://img.shields.io/badge/license-Zlib-blue)](LICENSE.md)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-brightgreen)
![Development Stage](https://img.shields.io/badge/development-Alpha-yellow)

![Logo](logo.png)

KalaData is a custom compression and decompression tool written in C++20, built entirely from scratch without external dependencies.  
It uses a hybrid LZSS + Huffman pipeline to compress data efficiently, while falling back to raw or empty storage when appropriate.  
All data is stored in a dedicated archival format with the `.kdat` extension.

> Other uses of the `.kdat` extension in unrelated software do not work with KalaData.  
KalaData archives have their own internal structure, designed specifically for this tool.

## Features
- Independent archive format `.kdat`, not based on `ZIP`, `RAR`, `7z` or other archival formats.
- Hybrid compression:
  - LZSS for dictionary-based redundancy removal.
  - Huffman coding for entropy reduction.
- Storage modes:
  - Compressed (LZSS + Huffman).
  - Raw (when compression is not effective).
  - Empty (for 0-byte files).
- Verbose logging (--tvb) with detailed per-file reporting.
- Summary statistics: input and output sizes, ratios, throughput (MB/s), file counts, and total duration.
- Cross-platform support for Windows 10/11 and Linux.
- No third-party libraries; relies only on the C++ Standard Library.

## Usage Model

KalaData supports two modes of operation:

1. **Direct mode**  
   Launch KalaData with a command from the system shell.  
   Example:  
   `KalaData.exe --c project_directory project.kdat`  
   This executes the command and then enters the KalaData CLI environment.

2. **Interactive mode**  
   Launch KalaData without arguments to enter its own CLI environment.  
   Commands such as `--c`, `--dc`, or `--tvb` can then be entered directly.  

Note: KalaData does not support command piping or chaining. Commands must be given either at startup in direct mode or entered manually in interactive mode within the CLI environment.

---

## Commands

### Notes:
  - KalaData accepts relative paths to current directory (or directory set with --go) or absolute paths.
  - the command `-help command` expects a valid command, like `--help c`
  - the command `--go path` expects a valid path in your device
  - the command `--sm mode` expects a valid mode, like `--sm balanced`

| Command          | Description                                            |
|------------------|--------------------------------------------------------|
| --v              | Prints KalaData version                                |
| --about          | Prints the KalaData description                        |
| --help           | Lists all commands                                     |
| --help `command` | Gets info about the specified command                  |
| --go `path`      | Goes to a directory on your device to be able to compress/decompress relative to that directory |
| --root           | Navigates to system root directory                     |
| --home           | Navigates to KalaData root directory                   |
| --where          | Prints your current path (KalaData root or the one set with --go) |
| --list           | Lists all files and directories in your current path (KalaData root or the one set with --go) |
| --create `path`  | Creates a new directory at the chosen path             |
| --delete `path`  | Deletes the file or directory at the chosen path (asks for confirmation before permanently deleting)|
| --sm `mode`      | Sets compression/decompression mode                    |
| --tvb            | Toggles verbosity (prints detailed logs when enabled)  |
| --c              | Compresses origin directory into target archive file path   |
| --dc             | Decompresses origin archive file into target directory path |
| --exit           | Quits KalaData                                         |

---

## Available compression modes

Note: All modes share the same min_match value `3`.

### Available modes

| Mode     | Best for            | Window size | Lookahead |
|----------|---------------------|-------------|-----------|
| fastest  | Temporary files     | 4 KB        | 18        |
| fast     | Quick backups       | 32 KB       | 32        |
| balanced | General use         | 256 KB      | 64        |
| slow     | Long-term storage   | 1 MB        | 128       |
| archive  | Maximum compression | 8 MB        | 255       |

---

## Verbose logging

Enabling verbose messages shows additional data that would otherwise flood your console window.
Use the `--tvb` command to toggle verbose messages on and off.

If enabled, then the following info is also displayed:

individual file logs:
  - compressed/decompressed file is empty
  - original file size is bigger than the compressed file size so it will not be compressed/decompressed
  - stored file size is smaller or equal than the compressed file size so it will be compressed/decompressed

compression/decompression success log additional rows:
  - compression/expansion ratio
  - compression/expansion factor
  - throughput
  - total files
  - compressed files
  - raw files
  - empty files
  
---

## KalaData Archive Layout

### Header data
| Offset | Size   | Field      | Description                        |
|--------|--------|------------|------------------------------------|
| 0x00   | 6 B    | magicVer   | Magic string + version (KDAT01)  |
| 0x06   | 4 B    | fileCount  | Number of file entries (uint32)    |

### Metadata + file data
| Offset (relative) | Size        | Field        | Description                                |
|-------------------|-------------|--------------|--------------------------------------------|
| +0x00             | 4 B         | pathLen      | Length of relative path string (uint32)    |
| +0x04             | pathLen B   | relPath      | Relative path string (not null-terminated) |
| +…                | 1 B         | method       | Storage flag (0 = raw, 1 = compressed)     |
| +…                | 8 B         | originalSize | Size before compression (uint64)           |
| +…                | 8 B         | storedSize   | Size after compression/raw (uint64)        |
| +…                | storedSizeB | data         | File data (omitted if storedSize = 0)      |

## Notes
- Archive always starts with `KDATxx` where `xx` is the version (01–99).
- Paths are stored exactly as written, with length prefix, no terminator.
- Compression is only applied if `storedSize < originalSize`; otherwise file is stored raw.
- Empty files are represented with `originalSize = 0` and `storedSize = 0`.

---

## Compression

The `--c` command takes in a directory which will be compressed into a `.kdat` file inside the target path parent directory.

Requirements and restrictions:

Origin:
  - path must exist
  - path must be a directory
  - directory must not be empty
  - directory size must not exceed 5GB

Target:
  - path must not exist
  - path must have the `.kdat` extension
  - path parent directory must be writable

> Example: `KalaData.exe --c C:\Projects\MyApp C:\Archives\MyApp.kdat`

---

## Decompression

The `--dc` command takes in a compressed `.kdat` file path which will be decompressed inside the target directory.

Requirements and restrictions:

Origin:
  - path must exist
  - path must be a regular file
  - path must have the `.kdat` extension

Target:
  - path must exist
  - path must be a directory
  - directory must be writable

> Example: `KalaData.exe --dc C:\Archives\MyApp.kdat C:\Extracted\MyApp`

## Prerequisites for building from source

### On Windows

> Read `Windows_prerequisites.txt` and use `Windows_prerequisites.zip`

### On Linux

> Not ready, coming soon

## How to build from source

The compiled executable and its files will be placed to `/release` and `/debug` in the root directory relative to the build stage

### On Windows

> Run `build_windows.bat`

### On Linux

> Not ready, coming soon
