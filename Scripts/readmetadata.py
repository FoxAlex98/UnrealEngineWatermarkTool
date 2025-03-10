import unreal

def get_metadata_from_asset(asset_path):
    """
    Recupera tutti i MetaData associati a un asset.
    
    Args:
    - asset_path (str): Il percorso dell'asset di cui leggere i MetaData.
    """
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

    if loaded_asset:
        metadata_keys = unreal.EditorAssetLibrary.get_metadata_tag_values(loaded_asset)
        print(f"🔍 MetaData per l'asset {asset_path}:")
        
        for key in metadata_keys:
            value = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, key)
            print(f"  - {key}: {value}")

    else:
        print("❌ Asset non trovato!")

# Specifica l'asset da controllare
asset_to_check = "/Game/MyAssets/MyTexture"

# Esegui la funzione
get_metadata_from_asset(asset_to_check)
