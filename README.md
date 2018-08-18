# ColumbusCMF
Columbus Model Format add-on for Blender, C library and console util for models conversion

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

## C Library
C library cmf.h created for simple using CMF in applications.
### Using
```c
uint32_t Count;
CMF_Vertex* Vertices = CMF_Load("filename.cmf");
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
	sudo make
	sudo make install
```

### Using
#### cmf [input] [output] [flags]

#### Console util flags
| Flag           | Description |
|----------------|-------------|
| -h, --help     | Print help message |
| -c, --compress | Enable compression for output file |

### Uninstalling
```
	sudo make uninstall
```



