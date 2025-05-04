# ctrff

Tool/Library to work with Nintendo 3ds File formats

## Note

**CTRFF Requires Palladium Headers from branch devel040 (as this is the branch the lib was latest tested with)**

## Building

- Desktop OS

```bash
# Note that -DCTRFF_DESKTOP is not required to build the lib for desktop
cmake -B build . -DCTRFF_DESKTOP=ON -DCMAKE_BUILD_TYPE=Release
cd build
make
```

- Nintendo 3ds

```bash
# Probably should go for Debug (to debug crashes)
# You could also do --toolchain path/to/debkitpro/cmake/3DS.cmake
# instead of -DCTRFF_3DS=ON
cmake -B build . -DCTRFF_3DS=ON -DCMAKE_BUILD_TYPE=Release
cd build
make
```

## File Formats

Not all Planned formates are listed here yet

| Format | State | Notes |
| ------ | ----- | ----- |
| 3dsx | Basic Loading and Viewing of Meta Data Smdh | |
| bcstm | Loading of almost every Data | Not capable of playing them yet (prefetch kernel panic) |
| bcwav | Basic Loading (not tested yet) | Not finished yet |
| bclim | Nothing done yet (Started creating header) | |
| lz11 | Encoder done, Decoder missing | Files are bit diffrent to the ones bannertool generates (don't know why) |
| romfs | Nothing Done yet (Started creating header) | |
| smdh | Almost done | missing safetey checks |
| cbmd | Nothing done yet | |
| cgfx | Nothing Done yet | |
| darc | Nothing done yet | |
