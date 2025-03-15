import unreal

import_subsystem = unreal.get_editor_subsystem(unreal.ImportSubsystem)

def on_asset_post_import(factory: unreal.Factory, created_object: unreal.Object):
    if created_object:
        asset_name = created_object.get_name()
        asset_path = created_object.get_path_name()
        unreal.log(f"Nuovo asset importato: {asset_name} ({asset_path})")
    else:
        unreal.log("Importazione fallita.")

if import_subsystem:
    import_subsystem.on_asset_post_import.add_callable(on_asset_post_import)

unreal.log("Monitoraggio degli asset importati avviato...")
