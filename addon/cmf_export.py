import bpy
import bmesh
import struct
from enum import IntEnum

class Compression(IntEnum):
    No   = 1 << 0
    ZSTD = 1 << 1

class Type(IntEnum):
    Positions = 0
    Texcoords = 1
    Normals   = 2
    Tangents  = 3
    Colors    = 4

class Format(IntEnum):
    Byte   = 0
    UByte  = 1
    Short  = 2
    UShort = 3
    Int    = 4
    UInt   = 5
    Half   = 6
    Float  = 7
    Double = 8

def writeArray(file, array_type, array_format, array):
    element_size = 0
    element_pack = ""

    if array_format is Format.Byte:
        element_size = 1
        element_pack = "b"
    elif array_format is Format.UByte:
        element_size = 1
        element_pack = "B"
    elif array_format is Format.Short:
        element_size = 2
        element_pack = "h"
    elif array_format is Format.UShort:
        element_size = 2
        element_pack = "H"
    elif array_format is Format.Int:
        element_size = 4
        element_pack = "i"
    elif array_format is Format.UInt:
        element_size = 4
        element_pack = "I"
    #elif array_format is Format.Half:
    elif array_format is Format.Float:
        element_size = 4
        element_pack = "f"
    elif array_format is Format.Double:
        element_size = 8
        element_pack = "d"

    file.write(struct.pack("I", array_type))
    file.write(struct.pack("I", array_format))
    file.write(struct.pack("I", len(array) * element_size))

    for elem in array:
        file.write(struct.pack(element_pack, elem))

def writeFile(filepath="",
              select_only=False,
              smooth_normals=False,
              write_indexes=False,
              write_positions=True,
              write_texcoords=True,
              write_normals=True,
              write_tangents=False,
              write_colors=False):

    magic = b"COLUMBUS MODEL FORMAT  \0"
    version = 1
    filesize = 0
    flags = 0
    compression = Compression.No
    num_vertices = 0
    num_arrays = 0

    verts = []
    uvs = []
    norms = []
    tangs = []
    cols = []

    for ob in bpy.context.scene.objects:
        try:
            ob.data.polygons
        except AttributeError:
            continue

        bpy.context.scene.objects.active = ob
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.quads_convert_to_tris()
        bpy.ops.object.mode_set(mode='OBJECT')

        for face in ob.data.polygons:
           for vert, loop in zip(face.vertices, face.loop_indices):
                num_vertices += 1

                verts.extend(ob.data.vertices[vert].co)
                uvs.extend(ob.data.uv_layers.active.data[loop].uv if ob.data.uv_layers.active != None else (0, 0))
                norms.extend(ob.data.vertices[vert].normal if smooth_normals else face.normal.xyz)
                cols.extend(ob.data.vertex_colors.active.data[loop].color if ob.data.vertex_colors.active != None else (0, 0, 0))

    if write_positions: num_arrays += 1
    if write_texcoords: num_arrays += 1
    if write_normals:   num_arrays += 1
    if write_tangents:  num_arrays += 1
    if write_colors:    num_arrays += 1

    file = open(filepath, "wb")

    file.write(magic)
    file.write(struct.pack("I", version))
    file.write(struct.pack("I", filesize))
    file.write(struct.pack("I", flags))
    file.write(struct.pack("I", compression))
    file.write(struct.pack("I", num_vertices))
    file.write(struct.pack("I", num_arrays))

    if write_positions: writeArray(file, Type.Positions, Format.Float, verts)
    if write_texcoords: writeArray(file, Type.Texcoords, Format.Float, uvs)
    if write_normals:   writeArray(file, Type.Normals,   Format.Float, norms)
    if write_tangents:  writeArray(file, Type.Tangents,  Format.Float, tangs)
    if write_colors:    writeArray(file, Type.Colors,    Format.Float, cols)

    file.close()


