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
	SReloadPopup.h
------------------------------------------------------------------------------------*/
#pragma once

#include "Widgets/Notifications/SNotificationList.h"
#include "Templates/SharedPointer.h"

/*------------------------------------------------------------------------------------
	SReloadPopup
------------------------------------------------------------------------------------*/
#if WITH_EDITOR
class AKAUDIO_API SReloadPopup
{
	static TSharedPtr<SNotificationItem> RefreshNotificationItem;
public:
	SReloadPopup();
	void NotifyProjectRefresh();
	void HideRefreshNotification();
};
#endif