# Xenon game (uses the XEngine library)

Standalone build against an installed engine:
```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
      -DCMAKE_PREFIX_PATH=<engine-install-dir>
cmake --build build
```
(The engine's own dependencies are listed in vcpkg.json because the installed static library needs them at link time.)
