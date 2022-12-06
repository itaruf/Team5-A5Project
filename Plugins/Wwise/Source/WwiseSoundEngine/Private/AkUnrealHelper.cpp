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

#include "AkUnrealHelper.h"
#include "Wwise/Stats/SoundEngine.h"

#include "Misc/Paths.h"

namespace AkUnrealHelper
{
	const TCHAR* MediaFolderName = TEXT("Media");
	const TCHAR* ExternalSourceFolderName = TEXT("ExternalSources");
	constexpr auto SoundBankNamePrefix = TEXT("SB_");
	const FGuid InitBankID(0x701ECBBD, 0x9C7B4030, 0x8CDB749E, 0xE5D1C7B9);

	FString(*GetWwisePluginDirectoryPtr)();
	FString(*GetWwiseProjectPathPtr)();
	FString(*GetSoundBankDirectoryPtr)();
	FString(*GetStagePathPtr)();
	
	void SetHelperFunctions(FString(* GetWwisePluginDirectoryImpl)(), FString(* GetWwiseProjectPathImpl)(),
		FString(* GetSoundBankDirectoryImpl)(), FString(* GetStagePathImpl)())
	{
		GetWwisePluginDirectoryPtr = GetWwisePluginDirectoryImpl;
		GetWwiseProjectPathPtr = GetWwiseProjectPathImpl;
		GetSoundBankDirectoryPtr = GetSoundBankDirectoryImpl;
		GetStagePathPtr = GetStagePathImpl;
	}

	FString GetWwisePluginDirectory()
	{
		if (!GetWwisePluginDirectoryPtr)
		{
			UE_LOG(LogWwiseSoundEngine, Error, TEXT("AkUnrealHelper::GetWwisePluginDirectory implementation not set."));
			return {};
		}	
		return GetWwisePluginDirectoryPtr();
	}

	FString GetWwiseProjectPath()
	{
		if (!GetWwiseProjectPathPtr)
		{
			UE_LOG(LogWwiseSoundEngine, Error, TEXT("AkUnrealHelper::GetWwiseProjectPath implementation not set."));
			return {};
		}	
		return GetWwiseProjectPathPtr();
	}

	FString GetSoundBankDirectory()
	{
		if (!GetSoundBankDirectoryPtr)
		{
			UE_LOG(LogWwiseSoundEngine, Error, TEXT("AkUnrealHelper::GetSoundBankDirectory implementation not set."));
			return {};
		}	
		return GetSoundBankDirectoryPtr();
	}

	FString GetStagePath()
	{
		if (!GetStagePathPtr)
		{
			UE_LOG(LogWwiseSoundEngine, Error, TEXT("AkUnrealHelper::GetStagePath implementation not set."));
			return {};
		}	
		return GetStagePathPtr();
	}

	void TrimPath(FString& Path)
	{
		Path.TrimStartAndEndInline();
	}

	FString GetProjectDirectory()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	}

	FString GetContentDirectory()
	{
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	}

	FString GetThirdPartyDirectory()
	{
		return FPaths::Combine(GetWwisePluginDirectory(), TEXT("ThirdParty"));
	}

	FString GetExternalSourceDirectory()
	{
		return FPaths::Combine(GetSoundBankDirectory(), ExternalSourceFolderName);
	}

	FString GetWwiseProjectDirectoryPath()
	{
		return FPaths::GetPath(GetWwiseProjectPath()) + TEXT("/");
	}

	bool MakePathRelativeToWwiseProject(FString& AbsolutePath)
	{

		auto wwiseProjectRoot = AkUnrealHelper::GetWwiseProjectDirectoryPath();
#if PLATFORM_WINDOWS
		AbsolutePath.ReplaceInline(TEXT("/"), TEXT("\\"));
		wwiseProjectRoot.ReplaceInline(TEXT("/"), TEXT("\\"));
#endif
		bool success = FPaths::MakePathRelativeTo(AbsolutePath, *wwiseProjectRoot);
#if PLATFORM_WINDOWS
		AbsolutePath.ReplaceInline(TEXT("/"), TEXT("\\"));
#endif
		return success;
	}

	FString GetWwiseSoundBankInfoCachePath()
	{
		return FPaths::Combine(FPaths::GetPath(GetWwiseProjectPath()), TEXT(".cache"), TEXT("SoundBankInfoCache.dat"));
	}

	const TCHAR* GetResultString(AKRESULT InResult)
	{
		switch (InResult)
		{
		case AKRESULT::AK_NotImplemented:				return TEXT("This feature is not implemented.");
		case AKRESULT::AK_Success:						return TEXT("The operation was successful.");
		case AKRESULT::AK_Fail:							return TEXT("The operation failed.");
		case AKRESULT::AK_PartialSuccess:				return TEXT("The operation succeeded partially.");
		case AKRESULT::AK_NotCompatible:				return TEXT("Incompatible formats");
		case AKRESULT::AK_AlreadyConnected:				return TEXT("The stream is already connected to another node.");
		case AKRESULT::AK_InvalidFile:					return TEXT("The provided file is the wrong format or unexpected values causes the file to be invalid.");
		case AKRESULT::AK_AudioFileHeaderTooLarge:		return TEXT("The file header is too large.");
		case AKRESULT::AK_MaxReached:					return TEXT("The maximum was reached.");
		case AKRESULT::AK_InvalidID:					return TEXT("The ID is invalid.");
		case AKRESULT::AK_IDNotFound:					return TEXT("The ID was not found.");
		case AKRESULT::AK_InvalidInstanceID:			return TEXT("The InstanceID is invalid.");
		case AKRESULT::AK_NoMoreData:					return TEXT("No more data is available from the source.");
		case AKRESULT::AK_InvalidStateGroup:			return TEXT("The StateGroup is not a valid channel.");
		case AKRESULT::AK_ChildAlreadyHasAParent:		return TEXT("The child already has a parent.");
		case AKRESULT::AK_InvalidLanguage:				return TEXT("The language is invalid (applies to the Low-Level I/O).");
		case AKRESULT::AK_CannotAddItseflAsAChild:		return TEXT("It is not possible to add itself as its own child.");
		case AKRESULT::AK_InvalidParameter:				return TEXT("Something is not within bounds, check the documentation of the function returning this code.");
		case AKRESULT::AK_ElementAlreadyInList:			return TEXT("The item could not be added because it was already in the list.");
		case AKRESULT::AK_PathNotFound:					return TEXT("This path is not known.");
		case AKRESULT::AK_PathNoVertices:				return TEXT("Stuff in vertices before trying to start it");
		case AKRESULT::AK_PathNotRunning:				return TEXT("Only a running path can be paused.");
		case AKRESULT::AK_PathNotPaused:				return TEXT("Only a paused path can be resumed.");
		case AKRESULT::AK_PathNodeAlreadyInList:		return TEXT("This path is already there.");
		case AKRESULT::AK_PathNodeNotInList:			return TEXT("This path is not there.");
		case AKRESULT::AK_DataNeeded:					return TEXT("The consumer needs more.");
		case AKRESULT::AK_NoDataNeeded:					return TEXT("The consumer does not need more.");
		case AKRESULT::AK_DataReady:					return TEXT("The provider has available data.");
		case AKRESULT::AK_NoDataReady:					return TEXT("The provider does not have available data.");
		case AKRESULT::AK_InsufficientMemory:			return TEXT("Memory error.");
		case AKRESULT::AK_Cancelled:					return TEXT("The requested action was cancelled (not an error).");
		case AKRESULT::AK_UnknownBankID:				return TEXT("Trying to load a bank using an ID which is not defined.");
		case AKRESULT::AK_BankReadError:				return TEXT("Error while reading a bank.");
		case AKRESULT::AK_InvalidSwitchType:			return TEXT("Invalid switch type (used with the switch container)");
		case AKRESULT::AK_FormatNotReady:				return TEXT("Source format not known yet.");
		case AKRESULT::AK_WrongBankVersion:				return TEXT("The bank version is not compatible with the current bank reader.");
		case AKRESULT::AK_FileNotFound:					return TEXT("File not found.");
		case AKRESULT::AK_DeviceNotReady:				return TEXT("Specified ID doesn't match a valid hardware device: either the device doesn't exist or is disabled.");
		case AKRESULT::AK_BankAlreadyLoaded:			return TEXT("The bank load failed because the bank is already loaded.");
		case AKRESULT::AK_RenderedFX:					return TEXT("The effect on the node is rendered.");
		case AKRESULT::AK_ProcessNeeded:				return TEXT("A routine needs to be executed on some CPU.");
		case AKRESULT::AK_ProcessDone:					return TEXT("The executed routine has finished its execution.");
		case AKRESULT::AK_MemManagerNotInitialized:		return TEXT("The memory manager should have been initialized at this point.");
		case AKRESULT::AK_StreamMgrNotInitialized:		return TEXT("The stream manager should have been initialized at this point.");
		case AKRESULT::AK_SSEInstructionsNotSupported:	return TEXT("The machine does not support SSE instructions (required on PC).");
		case AKRESULT::AK_Busy:							return TEXT("The system is busy and could not process the request.");
		case AKRESULT::AK_UnsupportedChannelConfig:		return TEXT("Channel configuration is not supported in the current execution context.");
		case AKRESULT::AK_PluginMediaNotAvailable:		return TEXT("Plugin media is not available for effect.");
		case AKRESULT::AK_MustBeVirtualized:			return TEXT("Sound was Not Allowed to play.");
		case AKRESULT::AK_CommandTooLarge:				return TEXT("SDK command is too large to fit in the command queue.");
		case AKRESULT::AK_RejectedByFilter:				return TEXT("A play request was rejected due to the MIDI filter parameters.");
		case AKRESULT::AK_InvalidCustomPlatformName:	return TEXT("Detecting incompatibility between Custom platform of banks and custom platform of connected application");
		case AKRESULT::AK_DLLCannotLoad:				return TEXT("Plugin DLL could not be loaded, either because it is not found or one dependency is missing.");
		case AKRESULT::AK_DLLPathNotFound:				return TEXT("Plugin DLL search path could not be found.");
		case AKRESULT::AK_NoJavaVM:						return TEXT("No Java VM provided in AkInitSettings.");
		case AKRESULT::AK_OpenSLError:					return TEXT("OpenSL returned an error.  Check error log for more details.");
		case AKRESULT::AK_PluginNotRegistered:			return TEXT("Plugin is not registered.  Make sure to implement a AK::PluginRegistration class for it and use AK_STATIC_LINK_PLUGIN in the game binary.");
		case AKRESULT::AK_DataAlignmentError:			return TEXT("A pointer to audio data was not aligned to the platform's required alignment (check AkTypes.h in the platform-specific folder)");
		case AKRESULT::AK_DeviceNotCompatible:			return TEXT("Incompatible Audio device.");
		case AKRESULT::AK_DuplicateUniqueID:			return TEXT("Two Wwise objects share the same ID.");
		case AKRESULT::AK_InitBankNotLoaded:			return TEXT("The Init bank was not loaded yet, the sound engine isn't completely ready yet.");
		case AKRESULT::AK_DeviceNotFound:				return TEXT("The specified device ID does not match with any of the output devices that the sound engine is currently using.");
		case AKRESULT::AK_PlayingIDNotFound:			return TEXT("Calling a function with a playing ID that is not known.");
		case AKRESULT::AK_InvalidFloatValue:			return TEXT("One parameter has a invalid float value such as NaN, INF or FLT_MAX.");
		case AKRESULT::AK_FileFormatMismatch:			return TEXT("Media file format unexpected");
		case AKRESULT::AK_NoDistinctListener:			return TEXT("No distinct listener provided for AddOutput");
		case AKRESULT::AK_ACP_Error:					return TEXT("Generic XMA decoder error.");
		case AKRESULT::AK_ResourceInUse:				return TEXT("Resource is in use and cannot be released.");
		case AKRESULT::AK_InvalidBankType:				return TEXT("Invalid bank type. The bank type was either supplied through a function call (e.g. LoadBank) or obtained from a bank loaded from memory.");
		case AKRESULT::AK_AlreadyInitialized:			return TEXT("Init() was called but that element was already initialized.");
		case AKRESULT::AK_NotInitialized:				return TEXT("The component being used is not initialized. Most likely AK::SoundEngine::Init() was not called yet, or AK::SoundEngine::Term was called too early.");
		case AKRESULT::AK_FilePermissionError:			return TEXT("The file access permissions prevent opening a file.");
		case AKRESULT::AK_UnknownFileError:				return TEXT("Rare file error occured, as opposed to AK_FileNotFound or AK_FilePermissionError. This lumps all unrecognized OS file system errors.");
		default: return TEXT("Unknown Error.");
		}
	}


#if WITH_EDITOR
	FString GuidToBankName(const FGuid& Guid)
	{
		if (Guid == InitBankID)
		{
			return TEXT("Init");
		}

		return FString(SoundBankNamePrefix) + Guid.ToString(EGuidFormats::Digits);
	}

	FGuid BankNameToGuid(const FString& BankName)
	{
		FString copy = BankName;
		copy.RemoveFromStart(SoundBankNamePrefix);

		FGuid result;
		FGuid::ParseExact(copy, EGuidFormats::Digits, result);

		return result;
	}

	FString FormatFolderPath(const FString folderPath)
	{
		auto path = folderPath.Replace(TEXT("\\"), TEXT("/"));
		if (path[0] == '/') {
			path.RemoveAt(0);
		}
		return path;
	}
#endif
}