bl_info = {
    "name": "Export Columbus Model Format (CMF) file",
    "category": "Import-Export",
}

import bpy
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

def faces():
    scene = bpy.context.scene

    faces = []
    for obj in bpy.context.selectable_objects:
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

def writeFile(filepath=""):
    triangles = faces()

    file = open(filepath, "wb")
    file.write(b'COLUMBUS MODEL FILE')
    for face in triangles:
        #file.write(struct.pack('>f>f>f', faceToLine(face)))
        #tri = faceToLine(face)
        for tri in face:
            for val in tri:
                file.write(struct.pack('>f', val))
    file.close()

class ExportCMF(bpy.types.Operator):
    bl_idname = "export_mesh.cmf"
    bl_label = "Export CMF"
    filename_ext = ".cmf"
    filter_glob = bpy.props.StringProperty(default="*.cmf", options={'HIDDEN'})
    filepath = bpy.props.StringProperty(subtype='FILE_PATH')

    def execute(self, context):
        writeFile(self.filepath)
        print ('Successfully exported CMF file')
        return {'FINISHED'}

    def invoke(self, context, event):
        wm = context.window_manager
        wm.fileselect_add(self)
        return {'RUNNING_MODAL'}


def menu_export(self, context):
    self.layout.operator(ExportCMF.bl_idname, text="Columbus Model Format (.cmf)")

def register():
    bpy.utils.register_class(ExportCMF)


def unregister():
    bpy.utils.unregister_class(ExportCMF)

if __name__ == "__main__":
    register()




