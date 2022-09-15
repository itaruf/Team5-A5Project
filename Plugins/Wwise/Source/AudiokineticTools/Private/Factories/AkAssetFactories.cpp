/*******************************************************************************
The content of the files in this repository include portions of the
AUDIOKINETIC Wwise Technology released in source code form as part of the SDK
package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
*******************************************************************************/


#include "AkAssetFactories.h"

#include "AkAcousticTexture.h"
#include "AkAudioBank.h"
#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkAuxBus.h"
#include "AkRtpc.h"
#include "AkSettings.h"
#include "AkSettingsPerUser.h"
#include "AkStateValue.h"
#include "AkSwitchValue.h"
#include "AkTrigger.h"
#include "AkEffectShareSet.h"
#include "AssetManagement/AkAssetDatabase.h"
#include "AssetTools/Public/AssetToolsModule.h"

struct AkAssetFactory_Helper
{
	enum class WwiseSync
	{
		Enabled,
		Disabled
	};

	template<typename AkAssetType = UAkAudioType>
	static UObject* FactoryCreateNew(UClass* Class, UObject* InParent, const FName& Name, EObjectFlags Flags, FGuid AssetID = FGuid{}, uint32 ShortID = 0)
	{
		auto AkSettingsPerUser = GetDefault<UAkSettingsPerUser>();
		auto ContainingPath = InParent->GetName();

		auto newObject = NewObject<AkAssetType>(InParent, Name, Flags);
		FWwiseBasicInfo* Info = newObject->GetInfoMutable();
		Info->AssetGuid = AssetID;
		Info->AssetName = Name.ToString();
		Info->AssetShortId = FAkAudioDevice::GetShortID(nullptr, Name.ToString());
		

		newObject->MarkPackageDirty();
		newObject->LoadData();
		return newObject;
	}

	template<typename AkAssetType>
	static bool CanCreateNew()
	{
		const UAkSettings* akSettings = GetDefault<UAkSettings>();
		if (akSettings)
		{
			return true;
		}
		return false;
	}

private:
	static FString ConvertAssetPathToWwisePath(FString ContainingPath, const FString& AssetName, const FString& BasePath)
	{
		ContainingPath.RemoveFromStart(BasePath, ESearchCase::IgnoreCase);
		ContainingPath.RemoveFromEnd(FString("/") + AssetName);
		return ContainingPath.Replace(TEXT("/"), TEXT("\\")).Replace(TEXT("_"), TEXT(" "));
	}
};

//////////////////////////////////////////////////////////////////////////
// UAkAcousticTextureFactory

UAkAcousticTextureFactory::UAkAcousticTextureFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkAcousticTexture::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkAcousticTextureFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkAcousticTexture>(Class, InParent, Name, Flags, AssetID, ShortID);
}

bool UAkAcousticTextureFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkAcousticTexture>();
}

//////////////////////////////////////////////////////////////////////////
// UAkAudioEventFactory

UAkAudioEventFactory::UAkAudioEventFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkAudioEvent::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkAudioEventFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkAudioEvent>(Class, InParent, Name, Flags, AssetID, ShortID);
}

bool UAkAudioEventFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkAudioEvent>();
}

//////////////////////////////////////////////////////////////////////////
// UAkAuxBusFactory

UAkAuxBusFactory::UAkAuxBusFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkAuxBus::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkAuxBusFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkAuxBus>(Class, InParent, Name, Flags, AssetID, ShortID);
}

bool UAkAuxBusFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkAuxBus>();
}

//////////////////////////////////////////////////////////////////////////
// UAkRtpcFactory

UAkRtpcFactory::UAkRtpcFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkRtpc::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkRtpcFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkRtpc>(Class, InParent, Name, Flags, AssetID, ShortID);
}

bool UAkRtpcFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkRtpc>();
}

//////////////////////////////////////////////////////////////////////////
// UAkTriggerFactory

UAkTriggerFactory::UAkTriggerFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkTrigger::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkTriggerFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkTrigger>(Class, InParent, Name, Flags, AssetID, ShortID);
}

bool UAkTriggerFactory::CanCreateNew() const
{
	return AkAssetFactory_Helper::CanCreateNew<UAkTrigger>();
}

//////////////////////////////////////////////////////////////////////////
// UAkStateValueFactory

UAkStateValueFactory::UAkStateValueFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkStateValue::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkStateValueFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkStateValue>(Class, InParent, Name, Flags, AssetID, ShortID);
}

//////////////////////////////////////////////////////////////////////////
// UAkSwitchValueFactory

UAkSwitchValueFactory::UAkSwitchValueFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkSwitchValue::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkSwitchValueFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkSwitchValue>(Class, InParent, Name, Flags, AssetID, ShortID);
}

//////////////////////////////////////////////////////////////////////////
// UAkEffectShareSetFactory

UAkEffectShareSetFactory::UAkEffectShareSetFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAkEffectShareSet::StaticClass();
	bCreateNew = bEditorImport = bEditAfterNew = true;
}

UObject* UAkEffectShareSetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return AkAssetFactory_Helper::FactoryCreateNew<UAkEffectShareSet>(Class, InParent, Name, Flags, AssetID, ShortID);
}