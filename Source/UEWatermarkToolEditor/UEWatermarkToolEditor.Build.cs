using UnrealBuildTool;

public class UEWatermarkToolEditor : ModuleRules
{
    public UEWatermarkToolEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "UEWatermarkTool",
                "UMG"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "UnrealEd",
                "EditorSubsystem",
                "EditorScriptingUtilities",
                "UEWatermarkTool"
            }
        );
    }
}