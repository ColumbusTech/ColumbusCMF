# ColumbusCMF
Columbus Model Format add-on for Blender, C library and console util for models conversion

The master hosted on github: https://github.com/ColumbusTech/ColumbusCMF

## Getting started
Columbus Model Format (CMF) developed for Columbus Engine. Engine needed fast, small, simple and and binary file format for containing 3-dimensional data.

C library was created for simple using CMF in applications (for example, CMF console util).

Console util was created for converting formats from most popular (FBX, OBJ) to CMF. Now it can't do this but I will develop this when there is free time.

Addon for Blender was created for simple saving models from blender into CMF. Now it just can't use compression.
## File structure
CMF is very simple. It has 26-byte header, containing magic bytes and some parameters and data field, which can be compressed with ZSTD algorithm.

### Header
| Field | Size in bytes | Value  |        Description         |
|-------|---------------|--------|----------------------------|
| Magic | 21 | string | File magic "COLUMBUS MODEL FORMAT" |
| Count | 4 | uint32 | Count of polygons in model  |
| Compression | 1 | uint8  | Compression of data field, 0x00 - no, 0xFF - ZSTD compression |

### Data field
| Part | Size in bytes | Description |
|------|---------------|-------------|
| Vertices | Count * 3 * 3 * sizeof(float) | Array of floats (XYZ, XYZ,...) with **ALL** vertices positions |
| UVs | Count * 3 * 2 * sizeof(float) | Array of floats (UV, UV,...) with **ALL** UV coordinates |
| Normals |  Count * 3 * 3 * sizeof(float) | Array of floats (XYZ, XYZ,...) with **ALL** normal directions |

## C Library
C library cmf.h created for simple using CMF in applications.

### Using

```c
uint32_t Count;
CMF_Vertex* Vertices = CMF_Load("filename.cmf", &Count);
CMF_vec3 Pos;
CMF_vec2 UV;
CMF_vec3 Norm;

for (uint32_t i = 0; i < Count; i++)
{
	Pos = Vertices[i].Position;
	UV = Vertices[i].UV;
	Norm = Vertices[i].Normal;

	some_func1(Pos.X, Pos.Y, Pos.Z);
	some_func2(UV.X, UV.Y);
	some_func3(Norm.X, Norm.Y, Norm.Z);
}

int Result = CMF_Save(Count, 0xFF, Vertices, "out.cmf");
```

## Console util

### Installing

```
make
sudo make install
```

### Using

```
cmf [input] [output] [flags]
```

#### Console util flags
| Flag           | Description |
|----------------|-------------|
| -h, --help     | Print help message |
| -c, --compress | Enable compression for output file |
| -v, --vertices | Enable writing vertices in output file |
| -t, --texcoords| Enable writing texture coordinates in output file |
| -n, --normals  | Enable writing normals in output file |

### Uninstalling

```
sudo make uninstall
```



