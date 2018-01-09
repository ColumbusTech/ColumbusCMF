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
    
def uvs(select_only=False):
    scene = bpy.context.scene
    selection = bpy.context.selectable_objects
    if select_only:
    	selection = bpy.context.selected_objects

    texcoords = []
    for obj in selection:
        if obj.type != 'MESH':
            try:
                me = obj.to_mesh(scene, True, "PREVIEW")
            except:
                me = None
            is_tmp_mesh = True
        else:
            me = obj.data
            is_tmp_mesh = False

        if me is not None:
            for uv in me.uv_layers:
                texcoords.extend(uv)

            if is_tmp_mesh:
                bpy.data.meshes.remove(me)
    
    return texcoords

def writeFile(filepath="", select_only=False):
    file = open(filepath, "wb")
    file.write(b"COLUMBUS MODEL FILE")

    count = 0
    for ob in bpy.context.scene.objects:
        try:
            ob.data.polygons
        except AttributeError:
            continue

        bpy.context.scene.objects.active = ob
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.quads_convert_to_tris()
        bpy.ops.object.mode_set(mode='OBJECT')

        count+=len(ob.data.polygons)

    file.write(struct.pack("I", count))
    bpy.ops.object.mode_set(mode='OBJECT')

    verts = []
    uvs = []
    norms = []

    for ob in bpy.context.scene.objects:
        try:
            ob.data.polygons
        except AttributeError:
            continue

        for face in ob.data.polygons:
            for vert, loop in zip(face.vertices, face.loop_indices):
                for item in ob.data.vertices[vert].co: #Vertex
                    verts.append(item)
                for item in (ob.data.uv_layers.active.data[loop].uv if ob.data.uv_layers.active != None else (0, 0)): #UV
                    uvs.append(item)
                for item in ob.data.vertices[vert].normal: #Normal
                    norms.append(item)

        for elem in verts:
            file.write(struct.pack('f', elem))

        for elem in uvs:
            file.write(struct.pack('f', elem))

        for elem in norms:
            file.write(struct.pack('f', elem))

            
