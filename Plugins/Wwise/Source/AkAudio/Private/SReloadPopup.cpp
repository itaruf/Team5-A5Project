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


/*------------------------------------------------------------------------------------
	SReloadPopup.cpp
------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------
 includes.
------------------------------------------------------------------------------------*/
#include "SReloadPopup.h"
#include "Async/Async.h"
#include "AkAudioModule.h"
#include "AkAudioStyle.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "AkAudio"

#if WITH_EDITOR
TSharedPtr<SNotificationItem> SReloadPopup::RefreshNotificationItem;

SReloadPopup::SReloadPopup()
{
	
}

void SReloadPopup::NotifyProjectRefresh()
{
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		FText InfoString = LOCTEXT("ReloadPopupRefreshData", "Wwise project was updated.\nReload Wwise Assets Data?");
		FSimpleDelegate RefreshDelegate = FSimpleDelegate();
		RefreshDelegate.BindRaw(FAkAudioModule::AkAudioModuleInstance, &FAkAudioModule::ReloadWwiseAssetDataAsync);

		FSimpleDelegate HideDelegate = FSimpleDelegate();
		HideDelegate.BindRaw(this, &SReloadPopup::HideRefreshNotification);
		FNotificationButtonInfo RefreshButton = FNotificationButtonInfo(LOCTEXT("WwiseDataRefresh", "Refresh"), FText(), RefreshDelegate);
		FNotificationButtonInfo HideButton = FNotificationButtonInfo(LOCTEXT("WwiseDataNotNow", "Not Now"), FText(), HideDelegate);
		FNotificationInfo Info(InfoString);
		Info.ButtonDetails.Add(RefreshButton);
		Info.ButtonDetails.Add(HideButton);
		Info.Image = FAkAudioStyle::GetBrush(TEXT("AudiokineticTools.AkPickerTabIcon"));
		Info.bUseSuccessFailIcons = false;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 10.0f;

		RefreshNotificationItem = FSlateNotificationManager::Get().AddNotification(Info);

		if (RefreshNotificationItem.IsValid())
		{
			RefreshNotificationItem->SetCompletionState(SNotificationItem::CS_Pending);
		}
	});
}

void SReloadPopup::HideRefreshNotification()
{
	AsyncTask(ENamedThreads::Type::GameThread, [this]
	{
		if (RefreshNotificationItem)
		{
			RefreshNotificationItem->Fadeout();
		}
	});
}
#endif
#undef LOCTEXT_NAMESPACE