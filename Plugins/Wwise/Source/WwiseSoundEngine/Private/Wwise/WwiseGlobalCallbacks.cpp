/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

#include "Wwise/WwiseGlobalCallbacks.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/SoundEngine.h"

#include "AkUnrealHelper.h"

bool FWwiseGlobalCallbacks::Init()
{
	if (UNLIKELY(bInitialized))
	{
		UE_LOG(LogWwiseSoundEngine, Fatal, TEXT("Global Callbacks already initialized"));
		return false;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseSoundEngine, Fatal, TEXT("Could not implement callbacks."));
		return false;
	}
	bInitialized = true;

	bool bResult = true;
	AKRESULT Result;
	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnRegisterCallbackStatic, AkGlobalCallbackLocation_Register, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `Register` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnBeginCallbackStatic, AkGlobalCallbackLocation_Begin, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `Begin` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnPreProcessMessageQueueForRenderCallbackStatic, AkGlobalCallbackLocation_PreProcessMessageQueueForRender, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `PreProcessMessageQueueForRender` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnPostMessagesProcessedCallbackStatic, AkGlobalCallbackLocation_PostMessagesProcessed, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `PostMessagesProcessed` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnBeginRenderCallbackStatic, AkGlobalCallbackLocation_BeginRender, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `BeginRender` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnEndRenderCallbackStatic, AkGlobalCallbackLocation_EndRender, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `EndRender` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnEndCallbackStatic, AkGlobalCallbackLocation_End, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `End` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnTermCallbackStatic, AkGlobalCallbackLocation_Term, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `Term` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnMonitorCallbackStatic, AkGlobalCallbackLocation_Monitor, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `Monitor` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnMonitorRecapCallbackStatic, AkGlobalCallbackLocation_MonitorRecap, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `MonitorRecap` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnInitCallbackStatic, AkGlobalCallbackLocation_Init, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `Init` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnSuspendCallbackStatic, AkGlobalCallbackLocation_Suspend, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `Suspend` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	Result = SoundEngine->RegisterGlobalCallback(&FWwiseGlobalCallbacks::OnWakeupFromSuspendCallbackStatic, AkGlobalCallbackLocation_WakeupFromSuspend, (void*)this);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Error, TEXT("Cannot Register `WakeupFromSuspend` Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
	bResult = bResult && (Result == AK_Success);

	return bResult;
}

void FWwiseGlobalCallbacks::Term()
{
	if (!bInitialized)
	{
		return;
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		return;
	}
	bInitialized = false;

	AKRESULT Result;
	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnRegisterCallbackStatic, AkGlobalCallbackLocation_Register);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register Register Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnBeginCallbackStatic, AkGlobalCallbackLocation_Begin);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register Begin Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnPreProcessMessageQueueForRenderCallbackStatic, AkGlobalCallbackLocation_PreProcessMessageQueueForRender);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register PreProcessMessageQueueForRender Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnPostMessagesProcessedCallbackStatic, AkGlobalCallbackLocation_PostMessagesProcessed);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register PostMessagesProcessed Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnBeginRenderCallbackStatic, AkGlobalCallbackLocation_BeginRender);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register BeginRender Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnEndRenderCallbackStatic, AkGlobalCallbackLocation_EndRender);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register EndRender Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnEndCallbackStatic, AkGlobalCallbackLocation_End);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register End Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnTermCallbackStatic, AkGlobalCallbackLocation_Term);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register Term Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnMonitorCallbackStatic, AkGlobalCallbackLocation_Monitor);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register Monitor Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnMonitorRecapCallbackStatic, AkGlobalCallbackLocation_MonitorRecap);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register MonitorRecap Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnInitCallbackStatic, AkGlobalCallbackLocation_Init);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register Init Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnSuspendCallbackStatic, AkGlobalCallbackLocation_Suspend);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register Suspend Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));

	Result = SoundEngine->UnregisterGlobalCallback(&FWwiseGlobalCallbacks::OnWakeupFromSuspendCallbackStatic, AkGlobalCallbackLocation_WakeupFromSuspend);
	UE_CLOG(Result != AK_Success, LogWwiseSoundEngine, Verbose, TEXT("Cannot Register WakeupFromSuspend Callback: %d (%s)"), Result, AkUnrealHelper::GetResultString(Result));
}

void FWwiseGlobalCallbacks::WaitForRegister()
{
	FEventRef Event;
	RegisterAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForBegin()
{
	FEventRef Event;
	BeginAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForPreProcessMessageQueueForRender()
{
	FEventRef Event;
	PreProcessMessageQueueForRenderAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForPostMessagesProcessed()
{
	FEventRef Event;
	PostMessagesProcessedAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForBeginRender()
{
	FEventRef Event;
	BeginRenderAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForEndRender()
{
	FEventRef Event;
	EndRenderAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForEnd()
{
	FEventRef Event;
	EndAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForTerm()
{
	FEventRef Event;
	TermAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForMonitor()
{
	FEventRef Event;
	MonitorAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForMonitorRecap()
{
	FEventRef Event;
	MonitorRecapAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForInit()
{
	FEventRef Event;
	InitAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForSuspend()
{
	FEventRef Event;
	SuspendAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::WaitForWakeupFromSuspend()
{
	FEventRef Event;
	WakeupFromSuspendAsync([&Event]() {Event->Trigger(); return EWwiseDeferredAsyncResult::Done; });
	Event->Wait();
	Event->Reset();
}

void FWwiseGlobalCallbacks::OnRegisterCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	RegisterQueue.Run();
}

void FWwiseGlobalCallbacks::OnBeginCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	BeginQueue.Run();
}

void FWwiseGlobalCallbacks::OnPreProcessMessageQueueForRenderCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	PreProcessMessageQueueForRenderQueue.Run();
}

void FWwiseGlobalCallbacks::OnPostMessagesProcessedCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	PostMessagesProcessedQueue.Run();
}

void FWwiseGlobalCallbacks::OnBeginRenderCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	BeginRenderQueue.Run();
}

void FWwiseGlobalCallbacks::OnEndRenderCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	EndRenderQueue.Run();
}

void FWwiseGlobalCallbacks::OnEndCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	EndQueue.Run();
}

void FWwiseGlobalCallbacks::OnTermCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	TermQueue.Run();
}

void FWwiseGlobalCallbacks::OnMonitorCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	MonitorQueue.Run();
}

void FWwiseGlobalCallbacks::OnMonitorRecapCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	MonitorRecapQueue.Run();
}

void FWwiseGlobalCallbacks::OnInitCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	InitQueue.Run();
}

void FWwiseGlobalCallbacks::OnSuspendCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	SuspendQueue.Run();
}

void FWwiseGlobalCallbacks::OnWakeupFromSuspendCallback(AK::IAkGlobalPluginContext* in_pContext)
{
	WakeupFromSuspendQueue.Run();
}

void FWwiseGlobalCallbacks::OnRegisterCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnRegisterCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnBeginCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnBeginCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnPreProcessMessageQueueForRenderCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnPreProcessMessageQueueForRenderCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnPostMessagesProcessedCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnPostMessagesProcessedCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnBeginRenderCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnBeginRenderCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnEndRenderCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnEndRenderCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnEndCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnEndCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnTermCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnTermCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnMonitorCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnMonitorCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnMonitorRecapCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnMonitorRecapCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnInitCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnInitCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnSuspendCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnSuspendCallback(in_pContext);
}

void FWwiseGlobalCallbacks::OnWakeupFromSuspendCallbackStatic(AK::IAkGlobalPluginContext* in_pContext,
	AkGlobalCallbackLocation in_eLocation, void* in_pCookie)
{
	static_cast<FWwiseGlobalCallbacks*>(in_pCookie)->OnWakeupFromSuspendCallback(in_pContext);
}
