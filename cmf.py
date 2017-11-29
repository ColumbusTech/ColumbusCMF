bl_info = {
    "name": "Export Columbus Model Format (CMF) file",
    "category": "Import-Export",
}

import bpy

class ExportCMF(bpy.types.Operator):
    bl_idname = "export_mesh.cmf"
    bl_label = "Export CMF"
    filename_ext = ".cmf"
    filter_glob = bpy.props.StringProperty(default="*.cmf", options={'HIDDEN'})
    filepath = bpy.props.StringProperty(subtype='FILE_PATH')

    select_only = bpy.props.BoolProperty(
            name="Selection Only",
            description="Export selected objects only",
            default=False)

    def execute(self, context):
        import cmf_export

        cmf_export.writeFile(
                self.filepath,
                self.select_only)
        
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




