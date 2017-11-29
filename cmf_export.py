import bpy
import bmesh
import struct

def triangulate(face):
    triangles = []
    if len(face) == 4:
        triangles.append([face[0], face[1], face[2]])
        triangles.append([face[2], face[3], face[0]])
    else:
        triangles.append(face)

    return triangles

def faceValues(face, mesh, matrix):
    fv = []
    for verti in face.vertices:
        fv.append((matrix * mesh.vertices[verti].co)[:])
    return fv

def faceToLine(face):
    return " ".join([("%.6f %.6f %.6f" % v) for v in face] + ["\n"])

def faces(select_only=False):
    scene = bpy.context.scene
    selection = bpy.context.selectable_objects
    if select_only:
    	selection = bpy.context.selected_objects

    faces = []
    for obj in selection:
        if obj.type != 'MESH':
            try:
                me = obj.to_mesh(scene, True, "PREVIEW")
            except:
                me = None
            is_tmp_mesh = True
        else:
            me = obj.data
            if not me.tessfaces and me.polygons:
                me.calc_tessface()
            is_tmp_mesh = False

        if me is not None:
            matrix = obj.matrix_world.copy()
            for face in me.tessfaces:
                fv = faceValues(face, me, matrix)
                faces.extend(triangulate(fv))

            if is_tmp_mesh:
                bpy.data.meshes.remove(me)

    return faces

def writeFile(filepath="", select_only=False):
    triangles = faces(select_only)
    
    file = open(filepath, "wb")
    file.write(b'COLUMBUS MODEL FILE') #Write magic
    file.write(struct.pack('>i', len(triangles * 3))) #Write vertices count

    for face in triangles:
        for tri in face:
            for val in tri:
                file.write(struct.pack('>f', val)) #Write vertices
    file.close()
