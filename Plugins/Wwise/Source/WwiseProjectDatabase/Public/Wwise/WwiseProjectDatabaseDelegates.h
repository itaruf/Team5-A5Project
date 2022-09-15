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