# detective-conan
detective-conan is a lightweight GPU hang analyzer. This library is intended to help locating potential GPU hangs by inserting writebacks into command buffer. It currently support Vulkan only.

# Build
The library is a served in header-only manner. No build is required to use it. The CMake script in the repository is to build samples and tests only.

# Integration
The library aims for easy integration with any Vulkan project. Everything you need is included in [inc](inc) folder.

Here are the step by step instructions to integrate it with your project:
- Copy everything in [inc](inc) folder into your project's source folder.
- Include [detective-conan.h](inc/detective-conan/detective-conan.h) anywhere you like.
- In one and only one of your source files, include [detective-conan.h](inc/detective-conan/detective-conan.h) with DETECTIVE_CONAN_IMPLEMENTATION macro defined in front of it. Then you are good to go:
```
// in your headers:
#include <detective-conan/detective-conan.h>

// in one of your source files:
#define DETECTIVE_CONAN_IMPLEMENTATION
#include <detective-conan/detective-conan.h>
```

# Usage
Check out sampler folder for a basic demonstration of how to use detective-conan library to detect GPU hang in Vulkan app.

# License
This library is released under MIT license.
