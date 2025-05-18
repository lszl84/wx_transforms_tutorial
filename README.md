# wx_transforms_tutorial

Implementing Affine Transformations in C++ in a Paint App to let the user change the size, position, and rotation of an object interactively.

[![Video](/output.gif)](https://www.youtube.com/watch?v=hYhM2tLwQKs)

Full Tutorial: https://www.youtube.com/watch?v=hYhM2tLwQKs

## Requirements

This works on Windows, Mac, and Linux. You'll need `cmake` and a C++ compiler (tested on `clang`, `gcc`, and MSVC).

Linux builds require the GTK3 library and headers installed in the system.

## Building

To build the project, use:

```bash
cmake -S. -Bbuild
cmake --build build
```

This will create a directory named `build` and create all build artifacts there. The main executable can be found in the `build/subprojects/Build/wx_transforms_tutorial_core` folder.

---
Check out the blog for more! [www.onlyfastcode.com](https://www.onlyfastcode.com)
---

