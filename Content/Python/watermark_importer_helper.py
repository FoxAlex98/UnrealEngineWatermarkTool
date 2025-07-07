import unreal
import metadata_asset_utils as metadata
from watermark_data_config import *

import_subsystem = unreal.get_editor_subsystem(unreal.ImportSubsystem)

def add_metadata_in_asset(asset, metadata_dict):
    asset_name = asset.get_name()
    asset_path = asset.get_path_name()
    
    if any(asset_path.startswith(folder) for folder in get_folders_to_exclude()):
        unreal.log_warning(f"Asset ignored (excluded folder): {asset_path}")
        return

    if get_should_add_metadata_on_import():
        unreal.log(f"New asset imported: {asset_name} ({asset_path})")
        metadata.add_metadata_to_asset(asset_path, metadata_dict)

def on_asset_post_import(factory: unreal.Factory, created_object: unreal.Object):
    if created_object:
        asset_path = created_object.get_path_name()
        asset_class = created_object.get_class().get_name()
        add_metadata_in_asset(created_object, get_metadata_to_add(asset_path))
        '''
        if asset_class == "Texture2D":
            unreal.log("Watermarking Texture2D asset...")
        elif asset_class == "SoundWave":
            unreal.log("Watermarking SoundWave asset...")
        elif asset_class == "StaticMesh":
            unreal.log("Embedding metadata into StaticMesh...")
            unreal.WatermarkFunctionLibrary.embed_watermark_in_static_mesh(created_object, "10001", 0.0005)
            #embed_watermark_in_static_mesh(asset_path, "10001")
        elif asset_class == "Blueprint":
            unreal.log("Tagging Blueprint with watermark...")
        else:
            unreal.log(f"Unhandled asset type: {asset_class}")
        '''
    else:
        unreal.log("Import failed.")

def start_listen_import_event():
    if import_subsystem:
        if not import_subsystem.on_asset_post_import.contains_callable(on_asset_post_import):
            import_subsystem.on_asset_post_import.add_callable(on_asset_post_import)
            unreal.log("Started monitoring imported assets...")
        else:
            unreal.log("Asset import monitoring is already active.")