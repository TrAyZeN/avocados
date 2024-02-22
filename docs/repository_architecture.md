# Repository architecture
This document describes the repository architecture.

## Architecture of src/
- `arch`: Architecture dependent code (only amd64 is targetted for now)
- `drivers`: Drivers
- `libk`: libc-like library for the kernel
- `mm`: Memory management code
- `tools`: Unit test framework, sanitizer implementations
