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
        add_metadata_in_asset(created_object, get_metadata_to_add(asset_path)) 
    else:
        unreal.log("Import failed.")

def start_listen_import_event():
    if import_subsystem:
        if not import_subsystem.on_asset_post_import.contains_callable(on_asset_post_import):
            import_subsystem.on_asset_post_import.add_callable(on_asset_post_import)
            unreal.log("Started monitoring imported assets...")
        else:
            unreal.log("Asset import monitoring is already active.")