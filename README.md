<h1 align="center">
    avocados
</h1>

> avocados is a WIP hobby x86-64 kernel

## Features
This project is a work in progress so the feature set is restricted.

- Multiboot2 support
- PMM, VMM
- Partial ACPI support (AML interpreter not implemented)
- HPET support
- Unit test framework
- Assertions for pre-conditions and post-conditions checks

## Build
```sh
make 2>&1 | tee build.log
```

## Development
To generate `compile_commands.json`:
```sh
bear -- make
```

To run inside qemu:
```sh
make run
```

To run inside bochs:
```sh
make run_bochs
```

## Documentation
Intel manual references refer to december 2022 version.
