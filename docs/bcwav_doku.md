# BCWAV File Format

**Note that this Docs are based on Current State of development and are very unfinished**

**This file can be very incorect due to copying from bcstm_doku.md**

## Contents

- 1 [Header](#header)
- 2 [Reference](#reference)
  - 2.1 [Sized Reference](#sized-reference)
  - 2.2 [Reference Table](#reference-table)
  - 2.3 [Reference Types](#reference-types)
- 3 [Block Header](#block-header)
- 4 [Info Block](#info-block)
  - 4.1 [Channel Info](#channel-info)
- 5 [DSP ADPCM Info](#dsp-adpcm-info)
  - 5.1 [DSP ADPCM Param](#dsp-adpcm-param)
  - 5.2 [DSP ADPCM Context](#dsp-adpcm-context)
- 6 [Basic Data Types](#basic-datatypes)
- 7 [Tools Devices used for Research](#tools--devices--file-sources-used-for-research)

## Header

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 4 | [u32](#u32) | Magic **'CWAV'** `0x56415743` |
| 0x04 | 2 | [u16](#u16) | Endianness `Big == 0xfffe` `Little == 0xfeff` |
| 0x06 | 2 | [u16](#u16) | Header Size `0x40` |
| 0x08 | 4 | [u32](#u32) | Version |
| 0x0c | 4 | [u32](#u32) | File Size |
| 0x10 | 2 | [u16](#u16) | Num Blocks (Should be 2) |
| 0x12 | 2 | [u16](#u16) | Reserved |
| 0x14 | 12 | [Sized Reference](#sized-reference) | Info Block Sized Reference |
| 0x20 | 12| [Sized Reference](#sized-reference) | Data Block Sized Reference |

## Reference

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 2 | [Reference Type](#reference-types) | TypeID |
| 0x02 | 2 | [u16](u16) | Padding |
| 0x04 | 4 | [u32](u32) | Offset **(0xffffffff represents null)** |

### Sized Reference

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 8 | [Reference](#reference) | Reference |
| 0x08 | 4 | [u32](u32) | Size |

### Reference Table

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 4 | [u32](#u32) | Count |
| 0x04 | Count*8 | [Reference](#reference) | References |

### Reference Types

| ID | Type |
|---|---|
| 0x0300 | [DSP ADPCM Info](#dsp-adpcm-info) |
| 0x0301 | IMA ADPCM Info |
| 0x1f00 | Sample Data |
| 0x7000 | [Info Block](#info-block) |
| 0x7001 | Data Block |
| 0x7100 | [Channel Info](#channel-info) |

## Block Header

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 4 | [u32](#u32) | Magic |
| 0x04 | 4 | [u32](#u32) | Size |

## Info Block

All Reference Offsets at the beginning of Info Block are Relative to the **Infoblock + 0x08 (size of Block Header) Position**

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 8 | [Block Header](#block-header) | Block Header |
| 0x08 | 1 | [u8](#u8) | Encoding |
| 0x09 | 1 | [u8](#u8) | Loop `1 == true -- 0  == false` |
| 0x0a | 1 | [u16](#u16) | Padding |
| 0x0c | 4 | [u32](#u32) | Sample Rate |
| 0x10 | 4 | [u32](#u32) | Loop Start |
| 0x14 | 4 | [u32](#u32) | Loop End |
| 0x18 | 4 | [u32](#u32) | Reserved |
| 0x18 | 8 | [Reference Table](#reference-table) | Channel Info Reference Table |

### Channel Info

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 8 | [Reference](#reference) | Reference to [DSP ADPCM Info](#dsp-adpcm-info) **!!! The Offsets are Relative to the start Pos of the Channel Info Object !!!** |

## DSP ADPCM Info

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 32 | [Param](#dsp-adpcm-param) | Coefficients |
| 0x20 | 6 | [DSP Context](#dsp-adpcm-context) | Context |
| 0x26 | 6 | [DSP Context](#dsp-adpcm-context) | Loop Context |
| 0x2c | 2 | [u16](#u16) | Padding |

### DSP ADPCM Param

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 32 | [u16](#u16) | 16 Bit Coefficients |

### DSP ADPCM Context

| Offset | Size | Datatype | Description |
|---|---|---|---|
| 0x00 | 1 | [u8](#u8) | 4Bit Predictor and 4Bit Scale |
| 0x01 | 1 | [u8](#u8) | Reserved |
| 0x02 | 2 | [16](#u16) | Previous Sample |
| 0x02 | 2 | [16](#u16) | Second Previous Sample |

## Basic Datatypes

### u32

```cpp
using u32 = unsigned int; // or uint32_t
```

### u16

```cpp
using u16 = unsigned short; // or uint16_t
```

### u8

```cpp
using u8 = unsigned char; // or uint8_t
```

## Tools / Devices / File Sources used for research

| Name | Description |
|---|---|
| Visual Studio Code | Used for creating ctrff c++ code for bcstm |
| ImHex | Used to Analyze the Hex Code of the bcstm Files |
| Citra | Fast way to generate Log files when developing ctrff |
| LoopingAudioConverter | Used to create some files for testing |
| ctrff-cli | Tool to generate Debug Output on Desktop OS like seen in BCSTM-Player File inspector |
| New 3ds XL | Testing on Real Hardware (BCSTM-Player) |
| Mario Kart 7 (Cartridge) | Used to get Test files |
| CTGP 7 | Used to get Test files |
| Super Mario Maker 3ds (Cartridge) | Used to get Test files |
| Mario and Luigi Bowsers inside story (Cartridge) | Used to get Test files |
| Donkey Kong Country Returns 3D (Cardridge) | Used to get Test files |
