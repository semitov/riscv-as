# riscv-as

A tiny and simple RISC-V assembler.

## Building - RP2350
The assembler includes a `CMakeLists.txt` that makes it a library for any RP2350 project!  
If you want to use it just clone the repository inside your project:
```bash
$ git clone https://github.com/semitov/riscv-as.git
```
Include the directory (`CMakeLists.txt`):
```bash
add_subdirectory(riscv-as)
```
Then link it in your project (`CMakeLists.txt`):
```bash
target_link_libraries(your_cool_project PUBLIC stv-assembler ...)
```
In the end you probably will need the files `instruction.c` and `writer.c` to use its functions, but fill free to hack it as you prefer!

__NOTE__ : Don't forget to define the macro `PICO_BUILD` to build it correctly.
## Building - Native
To build it (and use it) natively on any host PC simply use our meson build system:
```bash
$ meson setup build
$ cd build
$ meson compile
```
## Usage
```bash
$ ./semitov-riscv-as -c source_file.s -o output_binary
```

## Testing

If you want to test the generated binaries with [spike](https://github.com/riscv-software-src/riscv-isa-sim) and/or [pk](https://github.com/riscv-software-src/riscv-pk), you need a RISC-V toolchain.

- Recommended: prebuilt [riscv-gnu-toolchain-builds](https://github.com/FrancescoRocca/riscv-gnu-toolchain-builds).
- Alternative: build the [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain) yourself with multilib enabled.

> **Note**: `pk` should be built for 32-bit targets.

## Contributing

Please format the code before submitting any contribution. Run `ninja format` inside the `build/` folder.
