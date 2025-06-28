using UnrealBuildTool;

public class UEWatermarkToolMovieRender : ModuleRules
{
    public UEWatermarkToolMovieRender(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "MovieRenderPipelineCore",
                "MovieRenderPipelineSettings",
                "RenderCore",
                "RHI",
                "UMG",
                "UEWatermarkTool"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );
    }
}