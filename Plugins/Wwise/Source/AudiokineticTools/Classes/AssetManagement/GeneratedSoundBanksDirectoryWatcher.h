#pragma once
#include "DirectoryWatcher/Public/IDirectoryWatcher.h"
#include "Widgets/Notifications/SNotificationList.h"

DECLARE_MULTICAST_DELEGATE(OnSoundBankGenerationDoneDelegate);

class AUDIOKINETICTOOLS_API GeneratedSoundBanksDirectoryWatcher
{
public:
	bool DoesWwiseProjectExist();
	void CheckIfCachePathChanged();
	void Initialize();
	void Uninitialize(const bool bIsModuleShutdown =false);
	void StartWatchers();

	//Watch the SoundBankInfoCache.dat file. Changes to this file indicate that sound data generation is done
	bool StartCacheWatcher(const FString& CachePath);

	//Watch for changes to GeneratedSoundbanks folder, triggering a countdown timer to parse once no more changes are detected after a certain delay
	void StartSoundBanksWatcher(const FString& GeneratedSoundBanksFolder);

	void StopWatchers();
	void StopSoundBanksWatcher();
	void StopCacheWatcher();
	void RestartWatchers();
	bool ShouldRestartWatchers();

	OnSoundBankGenerationDoneDelegate OnSoundBanksGenerated;

private:
	FString CachePath;
	FString SoundBankDirectory;
	void OnCacheChanged(const TArray<FFileChangeData>& ChangedFiles);
	void OnGeneratedSoundBanksChanged(const TArray<FFileChangeData>& ChangedFiles);

	void TimerTick(float DeltaSeconds);
	void EndParseTimer();
	void OnSoundBankGenerationDone();
	void NotifyFilesChanged();
	void HideNotification();
	void UpdateNotificationOnGenerationComplete();
	void UpdateNotification();

	FDelegateHandle GeneratedSoundBanksHandle;
	FDelegateHandle CacheChangedHandle;
	FDelegateHandle PostEditorTickHandle;
	FDelegateHandle ProjectParsedHandle;

	FDelegateHandle SettingsChangedHandle;
	FDelegateHandle UserSettingsChangedHandle;

	bool bParseTimerRunning=false;
	float ParseTimer= 0;
	const float ParseDelaySeconds = 10.0f;

	TSharedPtr<SNotificationItem> NotificationItem;

	bool bCacheFolderExists = true;
	bool bGeneratedSoundBanksFolderExists = true;

};

