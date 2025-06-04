# Surface Auto Scale

Surface Auto Scale is a small Windows utility that automatically adjusts the display scaling of your primary monitor. When the device is in tablet mode it increases the DPI to make touch targets larger and reverts when a keyboard or additional monitor is detected. The code reuses functionality from [imniko/SetDPI](https://github.com/imniko/SetDPI) to change the DPI settings programmatically. The source code resides in the `src` directory and the compiled executable can be found in `dist`.

## Building

The project was built using MSVC and the Windows SDK. To compile the application yourself, open a Developer Command Prompt for Visual Studio and run:

```bash
cl /EHsc src\AutoScale.cpp src\DpiHelper.cpp src\SetDpiWrapper.cpp \
    /link user32.lib gdi32.lib shell32.lib Advapi32.lib
```

`SetDpi.exe` is no longer required because the DPI logic is now built into the
application.

## License
This project is released under [The Unlicense](LICENSE).

