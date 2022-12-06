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

#pragma once

#include "Wwise/WwiseIOHook.h"

#include "Templates/Atomic.h"

class IWwiseStreamingManagerHooks;

class WWISEFILEHANDLER_API FWwiseIOHookImpl : public FWwiseDefaultIOHook
{
public:
	FWwiseIOHookImpl();
	bool Init(const AkDeviceSettings& InDeviceSettings) override;

	/**
	 * IAkFileLocationResolver: Returns a file descriptor for a given file name (string).
	 *
	 * @param in_pszFileName	File name.
	 * @param in_eOpenMode		Open mode.
	 * @param in_pFlags			Special flags. Can pass NULL.
	 * @param io_bSyncOpen		If true, the file must be opened synchronously. Otherwise it is left at the File Location Resolver's discretion. Return false if Open needs to be deferred.
	 * @param out_fileDesc		Returned file descriptor.
	 * @return	AK_Success if operation was successful, error code otherwise
	 */
	AKRESULT Open(
		const AkOSChar*			in_pszFileName,
		AkOpenMode				in_eOpenMode,
		AkFileSystemFlags*		in_pFlags,
		bool&					io_bSyncOpen,
		AkFileDesc&				out_fileDesc
	) override;

	/**
	 * IAkFileLocationResolver: Returns a file descriptor for a given file ID.
	 *
	 * @param in_fileID			File ID.
	 * @param in_eOpenMode		Open mode.
	 * @param in_pFlags			Special flags. Can pass NULL.
	 * @param io_bSyncOpen		If true, the file must be opened synchronously. Otherwise it is left at the File Location Resolver's discretion. Return false if Open needs to be deferred.
	 * @param out_fileDesc		Returned file descriptor.
	 * @return	AK_Success if operation was successful, error code otherwise
	 */
	AKRESULT Open(
		AkFileID				in_fileID,          // File ID.
		AkOpenMode				in_eOpenMode,       // Open mode.
		AkFileSystemFlags*		in_pFlags,			// Special flags. Can pass NULL.
		bool&					io_bSyncOpen,		// If true, the file must be opened synchronously. Otherwise it is left at the File Location Resolver's discretion. Return false if Open needs to be deferred.
		AkFileDesc&				out_fileDesc        // Returned file descriptor.
	) override;

	/**
	 * Reads data from a file (asynchronous).
	 *
	 * @param in_fileDesc		File descriptor.
	 * @param in_heuristics		Heuristics for this data transfer.
	 * @param io_transferInfo	Asynchronous data transfer info.
	 * @return	AK_Success if operation was successful, error code otherwise
	 */
	AKRESULT Read(
		AkFileDesc &			in_fileDesc,
		const AkIoHeuristics &	in_heuristics,
		AkAsyncIOTransferInfo & io_transferInfo
	) override;

	// Writes data to a file (asynchronous).
	AKRESULT Write(
		AkFileDesc &			in_fileDesc,        // File descriptor.
		const AkIoHeuristics &	in_heuristics,		// Heuristics for this data transfer.
		AkAsyncIOTransferInfo & io_transferInfo		// Platform-specific asynchronous IO operation info.
	) override;

	virtual void Cancel(
		AkFileDesc &			in_fileDesc,
		AkAsyncIOTransferInfo & io_transferInfo,
		bool & io_bCancelAllTransfersForThisFile
	);

	/**
	 * Cleans up a file.
	 *
	 * @param in_fileDesc		File descriptor.
	 * @return	AK_Success if operation was successful, error code otherwise
	 */
	AKRESULT Close(AkFileDesc& in_fileDesc) override;

	// Returns the block size for the file or its storage device.
	AkUInt32 GetBlockSize(AkFileDesc& in_fileDesc) override;

	// Returns a description for the streaming device above this low-level hook.
	void GetDeviceDesc(AkDeviceDesc& out_deviceDesc) override;

	// Returns custom profiling data: 1 if file opens are asynchronous, 0 otherwise.
	AkUInt32 GetDeviceData() override;

protected:
	AkDeviceID m_deviceID = AK_INVALID_DEVICE_ID;

#ifndef AK_OPTIMIZED
	TAtomic<uint32> CurrentDeviceData;
	TAtomic<uint32> MaxDeviceData;
#endif

	virtual IWwiseStreamingManagerHooks* GetStreamingHooks(const AkFileSystemFlags& InFileSystemFlag);

	virtual AKRESULT OpenFileForWrite(
		const AkOSChar*			in_pszFileName,
		AkOpenMode				in_eOpenMode,
		AkFileSystemFlags*		in_pFlags,
		bool&					io_bSyncOpen,
		AkFileDesc&				out_fileDesc
	);
};
