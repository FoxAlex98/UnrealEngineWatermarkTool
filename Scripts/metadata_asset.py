import unreal

def add_metadata_to_assets(target_folder, metadata_key, metadata_value):

    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    
    assets = asset_registry.get_assets_by_path(target_folder, recursive=True)

    for asset in assets:
        asset_path = asset.package_name
        print(f"Processing: {asset_path}")

        loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

        if loaded_asset:
            unreal.EditorAssetLibrary.set_metadata_tag(loaded_asset, metadata_key, metadata_value)
            print(f"MetaData '{metadata_key}' aggiunto con valore '{metadata_value}' all'asset: {asset_path}")
            unreal.EditorAssetLibrary.save_asset(asset_path)

    print("MetaData aggiunto a tutti gli asset nella cartella.")

target_folder = "/Game/MyAssets"  
metadata_key = "Watermark_ID"
metadata_value = "UE5-Watermarked-Asset"

add_metadata_to_assets(target_folder, metadata_key, metadata_value)
