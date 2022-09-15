/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "AkUnrealEditorHelper.h"
#include "Net/RepLayout.h"
#include "Wwise/Stats/SimpleExtSrc.h"

#include "WwiseExternalSourceSettings.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnTablesChanged);

UCLASS(config = Game)
class WWISESIMPLEEXTERNALSOURCE_API UWwiseExternalSourceSettings : public UObject
{
	GENERATED_BODY()

public:
	//Table of all relevant information to properly load all external source media in the project
	//All files in this table will be packaged in the built project
	UPROPERTY(config, EditAnywhere, Category = ExternalSources, meta = (AllowedClasses = "DataTable"))
	FSoftObjectPath MediaInfoTable;

	//Optional table to define a default media entry in the MediaInfoTable to load when an External Source is loaded
	UPROPERTY(config, EditAnywhere, Category = ExternalSources, meta = (AllowedClasses = "DataTable"))
	FSoftObjectPath ExternalSourceDefaultMedia;

	//Staging location for External Source Media when cooking the project
	//Where external source media will be loaded from in the built pproject
	UPROPERTY(config, EditAnywhere, Category = ExternalSources, meta =(RelativeToGameContentDir))
	FDirectoryPath ExternalSourceStagingDirectory;

	FOnTablesChanged OnTablesChanged;

	static FString GetExternalSourceStagingDirectory()
	{
		if (const UWwiseExternalSourceSettings* ExtSettings = GetDefault<UWwiseExternalSourceSettings>())
		{
			return ExtSettings->ExternalSourceStagingDirectory.Path;
		}
		UE_LOG(LogWwiseSimpleExtSrc, Error, 
			TEXT("UWwiseExternalSourceSettings::GetExternalSourceStagingDirectory : Could not get staging directory from external source settings"));
		return {};
	}


	virtual void PostInitProperties() override
	{
		Super::PostInitProperties();
#if WITH_EDITOR
		AkUnrealEditorHelper::SaveConfigFile(this);
#endif

	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent ) override
	{
		Super::PostEditChangeProperty( PropertyChangedEvent );
		AkUnrealEditorHelper::SaveConfigFile(this);
		const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

		if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UWwiseExternalSourceSettings, MediaInfoTable) )
		{
			OnTablesChanged.Broadcast();
		}
		else if ( MemberPropertyName == GET_MEMBER_NAME_CHECKED(UWwiseExternalSourceSettings, ExternalSourceDefaultMedia))
		{
			OnTablesChanged.Broadcast();
		}
	}
#endif

};
