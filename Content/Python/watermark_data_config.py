import unreal
import os
import getpass

watermark_config = unreal.WatermarkConfig()

def get_should_add_metadata_on_import():
    return watermark_config.get_editor_property("bAutoAddMetadataOnImport")

def get_company_name():
    if(watermark_config.get_editor_property("bShouldAddCompanyName")):
        return watermark_config.get_editor_property("CompanyName")
    else:
        return None
    
def get_game_name():
    return unreal.SystemLibrary.get_game_name()

def get_author():
    user_name = unreal.SystemLibrary.get_platform_user_name()
    
    git_user = os.popen('git config --get user.name').read().strip()
    
    return git_user if git_user else user_name if user_name else getpass.getuser()

def get_folders_to_exclude():
    return watermark_config.get_editor_property("FoldersToExclude")


def get_metadata_to_add(asset_path):
    loaded_asset = unreal.EditorAssetLibrary.load_asset(asset_path)
    metadata_to_add = {"Author": get_author()}
    if loaded_asset:
        stored_company = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, "CompanyName")
        if not stored_company and get_company_name():
            metadata_to_add["CompanyName"] = get_company_name()

        last_used_project = unreal.EditorAssetLibrary.get_metadata_tag(loaded_asset, "LastUsedInProject")
        if not last_used_project:
            metadata_to_add["LastUsedInProject"] = get_game_name()

        return metadata_to_add