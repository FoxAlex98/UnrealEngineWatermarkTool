import unreal
import sys
import os

script_dir = os.path.dirname(os.path.abspath(__file__))

if script_dir not in sys.path:
    sys.path.append(script_dir)

from metadata_asset_utils import *

import_subsystem = unreal.get_editor_subsystem(unreal.ImportSubsystem)

def on_asset_post_import(factory: unreal.Factory, created_object: unreal.Object):
    if created_object:
        asset_name = created_object.get_name()
        asset_path = created_object.get_path_name()
        unreal.log(f"New asset imported: {asset_name} ({asset_path})")
        add_metadata_to_asset(asset_path, {"Author": "Alessandro", "Version": "1.0"})
    else:
        unreal.log("Import failed.")

if import_subsystem:
    if not import_subsystem.on_asset_post_import.contains_callable(on_asset_post_import):
        import_subsystem.on_asset_post_import.add_callable(on_asset_post_import)
        unreal.log("Started monitoring imported assets...")
    else:
        unreal.log("Asset import monitoring is already active.")
