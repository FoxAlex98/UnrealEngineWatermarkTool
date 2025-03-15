import unreal
import os
import csv
import datetime
import getpass

output_file = os.path.join(os.path.expanduser("~"), "Desktop", "Unreal_Watermark_Report.csv")

excluded_folders = ["/Game/ThirdParty", "/Game/Temp", "/Game/Plugins"]

project_name = unreal.SystemLibrary.get_game_name()
engine_version = unreal.SystemLibrary.get_engine_version()

company_name = "Test Company Name"

def get_imported_by():
    user_name = unreal.SystemLibrary.get_platform_user_name()
    git_user = os.popen('git config --get user.name').read().strip()
    return git_user if git_user else user_name if user_name else getpass.getuser()

def add_advanced_metadata(asset_path):
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    
    if loaded_asset:
        watermark_id = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, "Watermark_ID")
        if not watermark_id:
            watermark_id = f"UE5-WM-{hash(asset_path) % 1000000}"
            unreal.EditorAssetLibrary.set_metadata_tag(loaded_asset, "Watermark_ID", watermark_id)

        creation_date = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, "CreationDate")
        if not creation_date:
            creation_date = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            unreal.EditorAssetLibrary.set_metadata_tag(loaded_asset, "CreationDate", creation_date)

        last_used_project = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, "LastUsedInProject")
        if not last_used_project:
            unreal.EditorAssetLibrary.set_metadata_tag(loaded_asset, "LastUsedInProject", project_name)

        imported_by = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, "ImportedBy")
        if not imported_by:
            imported_by = get_imported_by()
            unreal.EditorAssetLibrary.set_metadata_tag(loaded_asset, "ImportedBy", imported_by)

        stored_company = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, "CompanyName")
        if not stored_company:
            unreal.EditorAssetLibrary.set_metadata_tag(loaded_asset, "CompanyName", company_name)

        print(f"Metadata added to asset: {asset_path}")

        update_csv_report(asset_path, watermark_id, creation_date, project_name, imported_by, company_name)

        unreal.EditorDialog.show_message("Watermark Added",
                                         f"Watermark_ID: {watermark_id}\nCreated on: {creation_date}\nImported by: {imported_by}\nCompany: {company_name}",
                                         unreal.AppMsgType.OK)

def update_csv_report(asset_path, watermark_id, creation_date, project_name, imported_by, company_name):
    asset_name = asset_path.split("/")[-1]
    asset_class = "Unknown"

    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_data = asset_registry.get_asset_by_object_path(asset_path)
    
    if asset_data.is_valid():
        asset_class = asset_data.asset_class
    
    file_exists = os.path.isfile(output_file)

    with open(output_file, mode='a', newline='') as file:
        writer = csv.writer(file)
        if not file_exists:
            writer.writerow(["Asset Name", "Asset Type", "Path", "Watermark_ID", "CreationDate", "LastUsedInProject", "ImportedBy", "CompanyName"])
        
        writer.writerow([asset_name, asset_class, asset_path, watermark_id, creation_date, project_name, imported_by, company_name])

    print(f"Report updated: {output_file}")

def on_asset_added(asset_data):
    asset_path = asset_data.object_path
    
    if any(asset_path.startswith(folder) for folder in excluded_folders):
        print(f"Asset ignored (excluded folder): {asset_path}")
        return

    print(f"New asset detected: {asset_path}")
    add_advanced_metadata(asset_path)

asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
asset_registry.on_asset_added_delegate.add_callable(on_asset_added)

print("Monitoring active: watermark and metadata will be added to new assets.")
