import unreal

def add_metadata_to_assets(target_folder, metadata_entries):
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    assets = asset_registry.get_assets_by_path(target_folder, recursive=True)

    for asset in assets:
        add_metadata_to_asset(asset.package_name, metadata_entries)

    unreal.log("Metadata added to all assets in folder: " + target_folder)

def add_metadata_to_asset(asset_path, metadata_entries):
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

    if loaded_asset:
        for key, value in metadata_entries.items():
            unreal.EditorAssetLibrary.set_metadata_tag(loaded_asset, key, value)
        #unreal.EditorAssetLibrary.save_asset(asset_path)
        unreal.log(f"Metadata added to asset: {asset_path}")
    else:
        unreal.log_error(f"Asset not found: {asset_path}")

def get_all_metadata(asset_path):
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

    if loaded_asset:
        metadata_keys = unreal.EditorAssetLibrary.get_metadata_tag_values(loaded_asset)
        if metadata_keys:
            unreal.log(f"Metadata for asset {asset_path}:")
            for key in metadata_keys:
                value = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, key)
                unreal.log(f"{key}: {value}")
        else:
            unreal.log_warning(f"No metadata found for asset: {asset_path}")
    else:
        unreal.log_error(f"Asset not found: {asset_path}")

def remove_metadata_from_assets(target_folder, metadata_keys):
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    assets = asset_registry.get_assets_by_path(target_folder, recursive=True)

    for asset in assets:
        remove_metadata_from_asset(asset.package_name, metadata_keys)

    unreal.log("Metadata removed from all assets in folder: " + target_folder)

def remove_metadata_from_asset(asset_path, metadata_keys):
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

    if loaded_asset:
        for key in metadata_keys:
            unreal.EditorAssetLibrary.remove_metadata_tag(loaded_asset, key)
        #unreal.EditorAssetLibrary.save_asset(asset_path)
        unreal.log(f"Metadata removed from asset: {asset_path}")
    else:
        unreal.log_error(f"Asset not found: {asset_path}")

def clear_metadata(asset_path):
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

    if loaded_asset:
        metadata_keys = unreal.EditorAssetLibrary.get_metadata_tag_values(loaded_asset)
        if metadata_keys:
            for key in metadata_keys:
                unreal.EditorAssetLibrary.remove_metadata_tag(loaded_asset, key)
            #unreal.EditorAssetLibrary.save_asset(asset_path)
            unreal.log(f"All metadata cleared from asset: {asset_path}")
        else:
            unreal.log_warning(f"No metadata found to clear in asset: {asset_path}")
    else:
        unreal.log_error(f"Asset not found: {asset_path}")
