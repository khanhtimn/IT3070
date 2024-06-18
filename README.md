# Mock Dynamic Linker

## Overview

This project demonstrates the process of loading and interpreting an ELF (Executable and Linkable Format) file to dynamically link shared libraries at runtime.

### Files

- `check_lib.sh`: A script to check and run the dynamic loader.
- `dynamic_loader`: Compiled binary of the dynamic loader program.
- `dynamic_loader.c`: Source code of the dynamic loader program.
- `example.c`: Example C code to generate a shared library.
- `libexample.so`: Compiled shared library used for testing.
- `main`: Compiled binary of the main program (if any).
- `main.c`: Source code of the main program (if any).
- `run.sh`: A script to compile and run the dynamic loader.

## Execution Steps

### Step 1: Compile the Shared Library

Compile the `example.c` to create a shared library `libexample.so`.

```sh
gcc -shared -fPIC -o libexample.so example.c
```

### Step 2: Compile the Dynamic Loader
Compile the dynamic_loader.c to create the dynamic_loader executable.

```sh
gcc -o dynamic_loader dynamic_loader.c
```

### Step 3: Execute the Run Script
Run the run.sh script to compile and execute the dynamic loader.

```sh
./dynamic_loader main
```
### Expected Output
The output should display the ELF header information, loaded segments, string table offset, and the names of the required libraries. For example:

```
ELF header read successfully
Loaded segment at offset 0x0 (virtual address: 0x0, size: 0x640)
Loaded segment at offset 0x1000 (virtual address: 0x1000, size: 0x15d)
Loaded segment at offset 0x2000 (virtual address: 0x2000, size: 0xa4)
Loaded segment at offset 0x2dc0 (virtual address: 0x3dc0, size: 0x260)
String table offset: 0x488
Needs library: libexample.so
Needs library: libc.so.6
```

```sh
./dynamic_loader libexample.so
```
### Expected Output
The output should display the ELF header information, loaded segments, string table offset, and the names of the required libraries. For example:

```
ELF header read successfully
Loaded segment at offset 0x0 (virtual address: 0x0, size: 0x538)
Loaded segment at offset 0x1000 (virtual address: 0x1000, size: 0x13d)
Loaded segment at offset 0x2000 (virtual address: 0x2000, size: 0xe4)
Loaded segment at offset 0x2df8 (virtual address: 0x3df8, size: 0x220)
String table offset: 0x3d0
Needs library: libc.so.6
```
