# ColumbusCMF
Columbus Model Format add-on for Blender and console util for models conversion

## Getting started
Columbus Model Format (CMF) developed for Columbus Engine. Engine needed fast, small, simple format for containing 3-dimensional data.
## File structure
CMF is very simple. It has 26-byte header, containing magic bytes and some parameters and data field, which can be compressed with ZSTD algorithm.

### Header
| Field | Size in bytes | Value  |        Description         |
|-------|---------------|--------|----------------------------|
| Magic | 21 | string | File magic "COLUMBUS MODEL FORMAT" |
| Count | 4 | uint32 | Count of polygons im model  |
| Compression | 1 | uint8  | Compression of data field, 0x00 - no, 0xFF - ZSTD compression |

### Data field
| Part | Size in bytes | Description |
|------|---------------|-------------|
| Vertices | Count * 3 * 3 * sizeof(float) | Array of floats (XYZ, XYZ,...) with **ALL** vertices positions |
| UVs | Count * 3 * 2 * sizeof(float) | Array of floats (UV, UV,...) with **ALL** UV coordinates |
| Normals |  Count * 3 * 3 * sizeof(float) | Array of floats (XYZ, XYZ,...) with **ALL** normal directions |

## Console util

### Using
#### cmf [input] [output] [flags]

### Console util flags
| Flag           | Description |
|----------------|-------------|
| -h, --help     | Print help message |
| -c, --compress | Enable compression for output file |



