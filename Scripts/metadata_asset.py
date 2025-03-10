import unreal

def add_metadata_to_assets(target_folder, metadata_key, metadata_value):
    """
    Aggiunge MetaData personalizzato agli asset in Unreal Engine.
    
    Args:
    - target_folder (str): Il percorso della cartella in cui cercare gli asset.
    - metadata_key (str): Il nome del MetaData da aggiungere.
    - metadata_value (str): Il valore del MetaData.
    """

    # Ottieni il riferimento al registro degli asset
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    
    # Ottieni tutti gli asset nella cartella target
    assets = asset_registry.get_assets_by_path(target_folder, recursive=True)

    for asset in assets:
        asset_path = asset.package_name
        print(f"Processing: {asset_path}")

        # Carica l'asset in memoria
        loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

        # Aggiunge il MetaData
        if loaded_asset:
            unreal.EditorAssetLibrary.set_metadata_tag(loaded_asset, metadata_key, metadata_value)
            print(f"✅ MetaData '{metadata_key}' aggiunto con valore '{metadata_value}' all'asset: {asset_path}")
            unreal.EditorAssetLibrary.save_asset(asset_path)

    print("✅ MetaData aggiunto a tutti gli asset nella cartella.")

# Imposta la cartella target e il MetaData
target_folder = "/Game/MyAssets"  # Cambia con il percorso della tua cartella
metadata_key = "Watermark_ID"
metadata_value = "UE5-Watermarked-Asset"

# Esegui la funzione
add_metadata_to_assets(target_folder, metadata_key, metadata_value)
