import unreal

def embed_watermark_in_static_mesh(mesh_path: str, bitstring: str, delta=0.0005):
    mesh = unreal.load_asset(mesh_path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh):
        unreal.log_error("Asset is not a StaticMesh")
        return

    if mesh.get_num_lods() == 0:
        unreal.log_error("Mesh has no LODs")
        return

    # Access editable mesh
    editor_mesh = unreal.EditorStaticMeshLibrary
    lod_index = 0
    section_index = 0
    vertices = editor_mesh.get_raw_mesh_vertex_positions(mesh, lod_index)

    normals = editor_mesh.get_raw_mesh_wedge_tangent_z(mesh, lod_index)

    if len(vertices) < len(bitstring):
        unreal.log_error("Mesh has too few vertices for watermark")
        return

    for i, bit in enumerate(bitstring):
        pos = vertices[i]
        normal = normals[i]
        offset = normal * delta * (1 if bit == '1' else -1)
        new_pos = pos + offset
        vertices[i] = new_pos

    # Apply changes
    editor_mesh.set_raw_mesh_vertex_positions(mesh, vertices, lod_index)
    mesh.mark_package_dirty()
    mesh.post_edit_change()
    unreal.log("Watermark embedded successfully.")

# Esempio uso
embed_watermark_in_static_mesh("/Game/Meshes/MyMesh", "1010100111")
