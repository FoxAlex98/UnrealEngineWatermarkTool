import unreal
import csv
import os

def generate_watermark_report(target_folder, output_file):
    """
    Generates a CSV report of all assets with the 'Watermark_ID' MetaData in an Unreal folder.
    
    Args:
    - target_folder (str): Folder in the Content Browser to scan.
    - output_file (str): Full path of the output CSV file.
    """

    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    
    assets = asset_registry.get_assets_by_path(target_folder, recursive=True)
    
    watermark_assets = []

    for asset in assets:
        asset_path = asset.package_name
        asset_name = asset.asset_name
        asset_class = asset.asset_class_path

        loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)

        if loaded_asset:
            watermark_id = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, "Watermark_ID")

            if watermark_id:
                watermark_assets.append([asset_name, asset_class, asset_path, watermark_id])

    if watermark_assets:
        with open(output_file, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(["Asset Name", "Asset Type", "Path", "Watermark_ID"])
            writer.writerows(watermark_assets)
        
        print(f"Report generated successfully: {output_file}")
    else:
        print("No assets with Watermark_ID found.")

target_folder = "/Game/MyAssets"
output_file = os.path.join(os.path.expanduser("~"), "Desktop", "Unreal_Watermark_Report.csv")

generate_watermark_report(target_folder, output_file)
