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
#include "AkInclude.h"

#include "Wwise/WwiseSoundEngineModule.h"

class WWISESOUNDENGINE_API FWwiseLowLevelComm
{
public:
	inline static FWwiseLowLevelComm* Get()
	{
		IWwiseSoundEngineModule::ForceLoadModule();
		return IWwiseSoundEngineModule::Comm;
	}

	///////////////////////////////////////////////////////////////////////
	/// @name Initialization
	//@{

	/// Initializes the communication module. When this is called, and AK::SoundEngine::RenderAudio()
	/// is called periodically, you may use the authoring tool to connect to the sound engine.
	///
	/// \warning This function must be called after the sound engine and memory manager have
	///          been properly initialized.
	///
	///
	/// \remark The AkCommSettings structure should be initialized with
	///         AK::Comm::GetDefaultInitSettings(). You can then change some of the parameters
	///			before calling this function.
	///
	/// \return
	///      - AK_Success if initialization was successful.
	///      - AK_InvalidParameter if one of the settings is invalid.
	///      - AK_InsufficientMemory if the specified pool size is too small for initialization.
	///      - AK_Fail for other errors.
	///		
	/// \sa
	/// - \ref initialization_comm
	/// - AK::Comm::GetDefaultInitSettings()
	/// - AkCommSettings::Ports
	AKRESULT Init(
		const AkCommSettings& in_settings///< Initialization settings.			
		);

	/// Gets the last error from the OS-specific communication library.
	/// \return The system error code.  Check the code in the platform manufacturer documentation for details about the error.
	AkInt32 GetLastError();

	/// Gets the communication module's default initialization settings values.
	/// \sa
	/// - \ref initialization_comm 
	/// - AK::Comm::Init()
	void GetDefaultInitSettings(
		AkCommSettings& out_settings	///< Returned default initialization settings.
		);

	/// Terminates the communication module.
	/// \warning This function must be called before the memory manager is terminated.		
	/// \sa
	/// - \ref termination_comm 
	void Term();

	/// Terminates and reinitialize the communication module using current settings.
	///
	/// \return
	///      - AK_Success if initialization was successful.
	///      - AK_InvalidParameter if one of the settings is invalid.
	///      - AK_InsufficientMemory if the specified pool size is too small for initialization.
	///      - AK_Fail for other errors.
	///
	/// \sa
	/// - \ref AK::SoundEngine::iOS::WakeupFromSuspend()
	AKRESULT Reset();


	/// Get the initialization settings currently in use by the CommunicationSystem
	///
	/// \return
	///      - AK_Success if initialization was successful.
	const AkCommSettings& GetCurrentSettings();

	//@}
};
