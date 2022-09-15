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

#include "Wwise/WwiseDeferredQueue.h"
#include "WwiseSoundEngineModule.h"

#include "AkInclude.h"

class FWwiseIOHook;
class IWwiseSoundBankManager;
class IWwiseExternalSourceManager;
class IWwiseMediaManager;

class WWISESOUNDENGINE_API FWwiseGlobalCallbacks
{
public:
	static FWwiseGlobalCallbacks* Get()
	{
		IWwiseSoundEngineModule::ForceLoadModule();
		return IWwiseSoundEngineModule::GlobalCallbacks;
	}

	virtual ~FWwiseGlobalCallbacks() { Term(); }

	virtual bool Init();
	virtual void Term();

	void RegisterAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { RegisterQueue.AsyncDefer(MoveTemp(InFunction)); }
	void RegisterSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { RegisterQueue.SyncDefer(MoveTemp(InFunction)); }
	void RegisterGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { RegisterQueue.GameDefer(MoveTemp(InFunction)); }

	void BeginAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { BeginQueue.AsyncDefer(MoveTemp(InFunction)); }
	void BeginSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { BeginQueue.SyncDefer(MoveTemp(InFunction)); }
	void BeginGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { BeginQueue.GameDefer(MoveTemp(InFunction)); }

	void PreProcessMessageQueueForRenderAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { PreProcessMessageQueueForRenderQueue.AsyncDefer(MoveTemp(InFunction)); }
	void PreProcessMessageQueueForRenderSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { PreProcessMessageQueueForRenderQueue.SyncDefer(MoveTemp(InFunction)); }
	void PreProcessMessageQueueForRenderGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { PreProcessMessageQueueForRenderQueue.GameDefer(MoveTemp(InFunction)); }

	void PostMessagesProcessedAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { PostMessagesProcessedQueue.AsyncDefer(MoveTemp(InFunction)); }
	void PostMessagesProcessedSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { PostMessagesProcessedQueue.SyncDefer(MoveTemp(InFunction)); }
	void PostMessagesProcessedGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { PostMessagesProcessedQueue.GameDefer(MoveTemp(InFunction)); }

	void BeginRenderAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { BeginRenderQueue.AsyncDefer(MoveTemp(InFunction)); }
	void BeginRenderSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { BeginRenderQueue.SyncDefer(MoveTemp(InFunction)); }
	void BeginRenderGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { BeginRenderQueue.GameDefer(MoveTemp(InFunction)); }

	void EndRenderAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { EndRenderQueue.AsyncDefer(MoveTemp(InFunction)); }
	void EndRenderSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { EndRenderQueue.SyncDefer(MoveTemp(InFunction)); }
	void EndRenderGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { EndRenderQueue.GameDefer(MoveTemp(InFunction)); }

	void EndAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { EndQueue.AsyncDefer(MoveTemp(InFunction)); }
	void EndSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { EndQueue.SyncDefer(MoveTemp(InFunction)); }
	void EndGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { EndQueue.GameDefer(MoveTemp(InFunction)); }

	void TermAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { TermQueue.AsyncDefer(MoveTemp(InFunction)); }
	void TermSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { TermQueue.SyncDefer(MoveTemp(InFunction)); }
	void TermGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { TermQueue.GameDefer(MoveTemp(InFunction)); }

	void MonitorAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { MonitorQueue.AsyncDefer(MoveTemp(InFunction)); }
	void MonitorSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { MonitorQueue.SyncDefer(MoveTemp(InFunction)); }
	void MonitorGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { MonitorQueue.GameDefer(MoveTemp(InFunction)); }

	void MonitorRecapAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { MonitorRecapQueue.AsyncDefer(MoveTemp(InFunction)); }
	void MonitorRecapSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { MonitorRecapQueue.SyncDefer(MoveTemp(InFunction)); }
	void MonitorRecapGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { MonitorRecapQueue.GameDefer(MoveTemp(InFunction)); }

	void InitAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { InitQueue.AsyncDefer(MoveTemp(InFunction)); }
	void InitSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { InitQueue.SyncDefer(MoveTemp(InFunction)); }
	void InitGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { InitQueue.GameDefer(MoveTemp(InFunction)); }

	void SuspendAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { SuspendQueue.AsyncDefer(MoveTemp(InFunction)); }
	void SuspendSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { SuspendQueue.SyncDefer(MoveTemp(InFunction)); }
	void SuspendGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { SuspendQueue.GameDefer(MoveTemp(InFunction)); }

	void WakeupFromSuspendAsync(FWwiseDeferredQueue::FAsyncFunction&& InFunction) { WakeupFromSuspendQueue.AsyncDefer(MoveTemp(InFunction)); }
	void WakeupFromSuspendSync(FWwiseDeferredQueue::FSyncFunction&& InFunction) { WakeupFromSuspendQueue.SyncDefer(MoveTemp(InFunction)); }
	void WakeupFromSuspendGame(FWwiseDeferredQueue::FSyncFunction&& InFunction) { WakeupFromSuspendQueue.GameDefer(MoveTemp(InFunction)); }

protected:
	bool bInitialized = false;
	FWwiseDeferredQueue RegisterQueue;
	FWwiseDeferredQueue BeginQueue;
	FWwiseDeferredQueue PreProcessMessageQueueForRenderQueue;
	FWwiseDeferredQueue PostMessagesProcessedQueue;
	FWwiseDeferredQueue BeginRenderQueue;
	FWwiseDeferredQueue EndRenderQueue;
	FWwiseDeferredQueue EndQueue;
	FWwiseDeferredQueue TermQueue;
	FWwiseDeferredQueue MonitorQueue;
	FWwiseDeferredQueue MonitorRecapQueue;
	FWwiseDeferredQueue InitQueue;
	FWwiseDeferredQueue SuspendQueue;
	FWwiseDeferredQueue WakeupFromSuspendQueue;

	virtual void OnRegisterCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnBeginCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnPreProcessMessageQueueForRenderCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnPostMessagesProcessedCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnBeginRenderCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnEndRenderCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnEndCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnTermCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnMonitorCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnMonitorRecapCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnInitCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnSuspendCallback(AK::IAkGlobalPluginContext* in_pContext);
	virtual void OnWakeupFromSuspendCallback(AK::IAkGlobalPluginContext* in_pContext);

private:
	static void OnRegisterCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnBeginCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnPreProcessMessageQueueForRenderCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnPostMessagesProcessedCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnBeginRenderCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnEndRenderCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnEndCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnTermCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnMonitorCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnMonitorRecapCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnInitCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnSuspendCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
	static void OnWakeupFromSuspendCallbackStatic(
		AK::IAkGlobalPluginContext * in_pContext,	///< Engine context.
		AkGlobalCallbackLocation in_eLocation,		///< Location where this callback is fired.
		void * in_pCookie							///< User cookie passed to AK::SoundEngine::RegisterGlobalCallback().
	);
};
