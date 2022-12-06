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

DECLARE_MULTICAST_DELEGATE(FOnDatabaseUpdateCompletedDelegate);

#define DEFINE_WWISE_DATABASE_DELEGATE(DelegateType) \
	public: F##DelegateType& Get##DelegateType() { return DelegateType; } \
	private: F##DelegateType DelegateType;

class WWISEPROJECTDATABASE_API FWwiseProjectDatabaseDelegates
{

	DEFINE_WWISE_DATABASE_DELEGATE(OnDatabaseUpdateCompletedDelegate);

public:
	static FWwiseProjectDatabaseDelegates& Get()
	{
		// return the singleton object
		static FWwiseProjectDatabaseDelegates Singleton;
		return Singleton;
	}
};