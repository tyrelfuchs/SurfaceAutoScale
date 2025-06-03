# Surface Auto Scale

This repository contains a small Windows utility that adjusts the display scaling based on the number of connected monitors and whether the system is in tablet mode. The main source file is located in the `src` directory and the precompiled binaries are stored in `bin`.

## Building

The project was built using MSVC and the Windows SDK. To compile the application yourself, open a Developer Command Prompt for Visual Studio and run:

```bash
cl /EHsc src\AutoScale.cpp /link user32.lib gdi32.lib shell32.lib Advapi32.lib
```

The program relies on `SetDpi.exe` (included in `bin`) to change system scaling.
