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

// Fill out your copyright notice in the Description page of Project Settings.

#include "AkSpotReflector.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "AkAuxBus.h"
#include "AkRoomComponent.h"
#include "Engine/Texture2D.h"
#include "Components/BillboardComponent.h"

AAkSpotReflector::WorldToSpotReflectorsMap AAkSpotReflector::sWorldToSpotReflectors;

// Sets default values
AAkSpotReflector::AAkSpotReflector(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AcousticTexture(NULL)
    , DistanceScalingFactor(2.f)
    , Level(1.f)
{
	static const FName ComponentName = TEXT("SpotReclectorRootComponent");
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, ComponentName);

#if WITH_EDITORONLY_DATA
	static const FName SpriteComponentName = TEXT("Sprite");
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(SpriteComponentName);
	if (SpriteComponent) 
	{
		SpriteComponent->SetSprite(LoadObject<UTexture2D>(NULL, TEXT("/Wwise/S_AkSpotReflector.S_AkSpotReflector")));
		SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->SetupAttachment(RootComponent);
	}
#endif

	// AActor properties 
	SetHidden(true);
	SetCanBeDamaged(false);
}

void AAkSpotReflector::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AddToWorld();
}

void AAkSpotReflector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	RemoveFromWorld();
}

void AAkSpotReflector::AddToWorld()
{
	UWorld* world = GetWorld();
	if (world)
	{
		SpotReflectorSet& SRSet = sWorldToSpotReflectors.FindOrAdd(world);
		SRSet.Add(this);
	}
}

void AAkSpotReflector::RemoveFromWorld()
{
	UWorld* world = GetWorld();
	if (world)
	{
		SpotReflectorSet* pSRSet = sWorldToSpotReflectors.Find(world);
		if (pSRSet)
		{
			pSRSet->Remove(this);

			if (pSRSet->Num() == 0)
			{
				sWorldToSpotReflectors.Remove(world);
			}
		}
	}
}

AkImageSourceID AAkSpotReflector::GetImageSourceID() const
{
	return (AkImageSourceID)(uint64)this;
}

AkAuxBusID AAkSpotReflector::GetAuxBusID() const
{
	return FAkAudioDevice::GetShortID(EarlyReflectionAuxBus, EarlyReflectionAuxBusName);
}

void AAkSpotReflector::SetImageSource(UAkComponent* AkComponent)
{
	FAkAudioDevice* pDev = FAkAudioDevice::Get();
	if (!pDev)
		return;

	const auto& RootTransform = RootComponent->GetComponentTransform();
	AkImageSourceSettings sourceInfo = AkImageSourceSettings(
		FAkAudioDevice::FVectorToAKVector64(RootTransform.GetTranslation()),
		DistanceScalingFactor, Level);

	if (AcousticTexture)
	{
		sourceInfo.SetOneTexture(AcousticTexture->GetShortID());
	}

	pDev->SetImageSource(this, sourceInfo, GetAuxBusID(), AkComponent);
}

void AAkSpotReflector::SetSpotReflectors(UAkComponent* AkComponent)
{
	FAkAudioDevice* pDev = FAkAudioDevice::Get();
	if (pDev)
	{
		pDev->ClearImageSources(AK_INVALID_AUX_ID, AkComponent);

		if (AkComponent->EnableSpotReflectors)
		{
			UWorld* world = AkComponent->GetWorld();
			if (world)
			{
				SpotReflectorSet* pSRSet = sWorldToSpotReflectors.Find(world);
				if (pSRSet)
				{
					for (auto It = pSRSet->CreateIterator(); It; ++It)
					{
						(*It)->SetImageSource(AkComponent);
					}
				}
			}
		}
	}
}
