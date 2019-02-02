import bpy
import bmesh
import struct
from enum import IntEnum
from itertools import groupby

class Compression(IntEnum):
    No   = 1 << 0
    ZSTD = 1 << 1

class Type(IntEnum):
    Positions = 0
    Texcoords = 1
    Normals   = 2
    Tangents  = 3
    Colors    = 4
    Indices   = 5

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

def sizeFromFormat(f):
    if f == Format.Byte or f == Format.UByte: return 1
    if f == Format.Short or f == Format.UShort: return 2
    if f == Format.Int or f == Format.UInt: return 4
    if f == Format.Half: return 2
    if f == Format.Float: return 4
    if f == Format.Double: return 8

    return 0

def writeArray(file, array_type, array_format, array):
    element_size = sizeFromFormat(array_format)
    element_pack = ""

    if array_format == Format.Byte: element_pack = "b"
    elif array_format == Format.UByte: element_pack = "B"
    elif array_format == Format.Short: element_pack = "h"
    elif array_format == Format.UShort: element_pack = "H"
    elif array_format == Format.Int: element_pack = "i"
    elif array_format == Format.UInt: element_pack = "I"
    #elif array_format = Format.Half:
    elif array_format == Format.Float: element_pack = "f"
    elif array_format == Format.Double: element_pack = "d"

    file.write(struct.pack("I", array_type))
    file.write(struct.pack("I", array_format))
    file.write(struct.pack("I", len(array) * element_size))

    for elem in array:
        file.write(struct.pack(element_pack, elem))

def writeFile(filepath="",
              select_only=False,
              write_indexes=False,
              write_positions=True,
              write_texcoords=True,
              write_normals=True,
              write_tangents=False,
              write_colors=False):

    magic = b"COLUMBUS MODEL FORMAT  \0"
    version = 1
    filesize = 48
    flags = 0
    compression = Compression.No
    num_vertices = 0
    num_arrays = 0

    verts = []
    uvs = []
    norms = []
    tangs = []
    cols = []
    inds = []
    indices_format = Format.UByte

    for ob in bpy.context.scene.objects:
        try:
            ob.data.polygons
        except AttributeError:
            continue

        bpy.context.scene.objects.active = ob
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.quads_convert_to_tris()
        bpy.ops.object.mode_set(mode='OBJECT')

        if write_indexes:
            vert_hash = {}

            for face in ob.data.polygons:
                for vert, loop in zip(face.vertices, face.loop_indices):
                    v = ob.data.vertices[vert].co
                    u = ob.data.uv_layers.active.data[loop].uv if ob.data.uv_layers.active != None else (0, 0)
                    n = ob.data.vertices[vert].normal if face.use_smooth else face.normal.xyz
                    c = ob.data.vertex_colors.active.data[loop].color if ob.data.vertex_colors.active != None else (0, 0, 0)
                    new_vert = (tuple(v), tuple(u), tuple(n), tuple(c))

                    if new_vert in vert_hash:
                        inds.append(vert_hash[new_vert])
                    else:
                        vert_hash[new_vert] = num_vertices
                        inds.append(num_vertices)
                        num_vertices += 1

                        verts.extend(v)
                        uvs.extend(u)
                        norms.extend(n)
                        cols.extend(c)

            if len(inds) > 255:
                indices_format = Format.UShort

            if len(inds) > 65535:
                indices_format = Format.UInt
        else:
            for face in ob.data.polygons:
                for vert, loop in zip(face.vertices, face.loop_indices):
                    num_vertices += 1

                    verts.extend(ob.data.vertices[vert].co)
                    uvs.extend(ob.data.uv_layers.active.data[loop].uv if ob.data.uv_layers.active != None else (0, 0))
                    norms.extend(ob.data.vertices[vert].normal if face.use_smooth else face.normal.xyz)
                    cols.extend(ob.data.vertex_colors.active.data[loop].color if ob.data.vertex_colors.active != None else (0, 0, 0))

    if write_positions:
        num_arrays += 1
        filesize += len(verts) * 4
    if write_texcoords:
        num_arrays += 1
        filesize += len(uvs) * 4
    if write_normals:
        num_arrays += 1
        filesize += len(norms) * 4
    if write_tangents:
        num_arrays += 1
        filesize += len(tangs) * 4
    if write_colors:
        num_arrays += 1
        filesize += len(cols) * 4
    if write_indexes:
        num_arrays += 1
        filesize += len(inds) * sizeFromFormat(indices_format)

    array_header_size = 12
    filesize += num_arrays * array_header_size

    file = open(filepath, "wb")

    file.write(magic)
    file.write(struct.pack("I", version))
    file.write(struct.pack("I", filesize))
    file.write(struct.pack("I", flags))
    file.write(struct.pack("I", compression))
    file.write(struct.pack("I", num_vertices))
    file.write(struct.pack("I", num_arrays))

    if write_positions: writeArray(file, Type.Positions, Format.Float,    verts)
    if write_texcoords: writeArray(file, Type.Texcoords, Format.Float,    uvs)
    if write_normals:   writeArray(file, Type.Normals,   Format.Float,    norms)
    if write_tangents:  writeArray(file, Type.Tangents,  Format.Float,    tangs)
    if write_colors:    writeArray(file, Type.Colors,    Format.Float,    cols)
    if write_indexes:   writeArray(file, Type.Indices,   indices_format,  inds)

    file.close()


