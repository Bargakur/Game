#include "Resourcenames.h"


// to do error handling
const UEnum* UGameResources::GetResourceKindEnum()
{
    static const UEnum* ResourceKindEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EResourceKind"));
    return ResourceKindEnum;
}

FName UGameResources::ResourceKindToName(EResourceKind ResourceKind)
{
    const UEnum* ResourceEnum = GetResourceKindEnum();
    if (!ResourceEnum)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find EResourceKind enum!"));
        return NAME_None;
    }
    
    // Get the display name from the enum
    FString DisplayName = ResourceEnum->GetDisplayNameTextByIndex((int32)ResourceKind).ToString();
    
    // If display name is empty or invalid, fall back to enum name
    if (DisplayName.IsEmpty() || DisplayName == TEXT("Invalid"))
    {
        DisplayName = ResourceEnum->GetNameStringByIndex((int32)ResourceKind);
        // Remove the enum prefix (e.g., "EResourceKind::wood" -> "wood")
        int32 ColonIndex;
        if (DisplayName.FindLastChar(':', ColonIndex))
        {
            DisplayName = DisplayName.RightChop(ColonIndex + 1);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("ResourceKindToName: %d -> %s"), (int32)ResourceKind, *DisplayName);
    return FName(*DisplayName);
}

EResourceKind UGameResources::NameToResourceKind(const FName& ResourceName)
{
    const UEnum* ResourceEnum = GetResourceKindEnum();
    if (!ResourceEnum)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find EResourceKind enum!"));
        return EResourceKind::MAX;
    }
    
    // First try to find by display name
    for (int32 i = 0; i < ResourceEnum->NumEnums() - 1; ++i) // -1 to skip MAX
    {
        FString DisplayName = ResourceEnum->GetDisplayNameTextByIndex(i).ToString();
        if (DisplayName == ResourceName.ToString())
        {
            return (EResourceKind)ResourceEnum->GetValueByIndex(i);
        }
    }
    
    // Fall back to enum name matching
    for (int32 i = 0; i < ResourceEnum->NumEnums() - 1; ++i) // -1 to skip MAX
    {
        FString EnumName = ResourceEnum->GetNameStringByIndex(i);
        // Remove the enum prefix
        int32 ColonIndex;
        if (EnumName.FindLastChar(':', ColonIndex))
        {
            EnumName = EnumName.RightChop(ColonIndex + 1);
        }
        
        if (EnumName == ResourceName.ToString())
        {
            return (EResourceKind)ResourceEnum->GetValueByIndex(i);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Could not find enum for resource name: %s"), *ResourceName.ToString());
    return EResourceKind::MAX;
}

TArray<EResourceKind> UGameResources::GetAllResourceKinds()
{
    TArray<EResourceKind> ResourceKinds;
    const UEnum* ResourceEnum = GetResourceKindEnum();
    
    if (ResourceEnum)
    {
        for (int32 i = 0; i < ResourceEnum->NumEnums() - 1; ++i) // -1 to skip MAX
        {
            ResourceKinds.Add((EResourceKind)ResourceEnum->GetValueByIndex(i));
        }
    }
    
    return ResourceKinds;
}

TArray<FName> UGameResources::GetAllResourceNames()
{
    TArray<FName> ResourceNames;
    TArray<EResourceKind> ResourceKinds = GetAllResourceKinds();
    
    for (EResourceKind Kind : ResourceKinds)
    {
        ResourceNames.Add(ResourceKindToName(Kind));
    }
    
    return ResourceNames;
}

bool UGameResources::IsValidResourceKind(EResourceKind ResourceKind)
{
    const UEnum* ResourceEnum = GetResourceKindEnum();
    if (!ResourceEnum)
    {
        return false;
    }
    
    return ResourceKind != EResourceKind::MAX && 
           ResourceEnum->IsValidEnumValue((int64)ResourceKind);
}

bool UGameResources::IsValidResourceName(const FName& ResourceName)
{
    return NameToResourceKind(ResourceName) != EResourceKind::MAX;
}