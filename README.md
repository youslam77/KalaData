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
- Independent archive format (.kdat), not based on ZIP, RAR, or 7z.
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
   `KalaData.exe --c project_folder project.kdat`  
   This executes the command and then enters the KalaData CLI environment.

2. **Interactive mode**  
   Launch KalaData without arguments to enter its own CLI environment.  
   Commands such as `--c`, `--dc`, or `--tvb` can then be entered directly.  

Note: KalaData does not support command piping or chaining. Commands must be given either at startup in direct mode or entered manually in interactive mode within the CLI environment.

---

## Commands

### Notes:
  - all paths are absolute and KalaData does not take relative paths
  - if you use `--help command` you must put a real command there, for example `--help c`

| Command          | Description                                            |
|------------------|--------------------------------------------------------|
| --v              | Print KalaData version                                 |
| --about          | Prints the KalaData description                        |
| --help           | List all commands                                      |
| --help *command* | Get info about the specified command                   |
| --tvb            | Toggle verbosity (prints detailed logs when enabled)   |
| --c              | Compress origin folder into target archive file path   |
| --dc             | Decompress origin archive file into target folder path |
| --exit           | Quit KalaData                                          |

---

## Compression

The `--c` command takes in a folder which will be compressed into a `.kdat` file inside the target path parent folder.

Requirements and restrictions:

Origin:
  - path must exist
  - path must be a folder
  - folder must not be empty
  - folder size must not exceed 5GB

Target:
  - path must not exist
  - path must have the `.kdat` extension
  - path parent folder must be writable

> Example: KalaData.exe --c C:\Projects\MyApp C:\Archives\MyApp.kdat

---

## Decompression

The `--dc` command takes in a compressed `.kdat` file path which will be decompressed inside the target folder.

Requirements and restrictions:

Origin:
  - path must exist
  - path must be a regular file
  - path must have the `.kdat` extension

Target:
  - path must exist
  - path must be a folder
  - folder must be writable

> Example: KalaData.exe --dc C:\Archives\MyApp.kdat C:\Extracted\MyApp

## Prerequisites for building from source

### On Windows

> Read Windows_prerequisites.txt and use Windows_prerequisites.zip

### On Linux

> Not ready, coming soon

## How to build from source

The compiled executable and its files will be placed to /release and /debug relative to the build stage

### On Windows

> Run `build_windows.bat`

### On Linux

> Not ready, coming soon