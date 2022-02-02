// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "MaterialMapping.generated.h"

UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    ETT_AUTOMATIC = 0,
    ETT_ROCK,
    ETT_SOIL,
    ETT_ROAD,
    ETT_VEGETATION,
    ETT_BLOCKED
};

UENUM(BlueprintType)
enum class ETerrainSubType : uint8
{
    ETST_AUTOMATIC = 0,

    // ROCK
    ETST_ROCK_ROCK = 10,
    ETST_ROCK_LIMESTONE,
    ETST_ROCK_DOLOMITE,
    ETST_ROCK_NARI,
    ETST_ROCK_BASALT,
    ETST_ROCK_CHALK,
    ETST_ROCK_MARAL,

    // ROAD
    ETST_ROAD_ROAD = 20,
    ETST_ROAD_PAVED,
    ETST_ROAD_UNPAVED,

    // SOIL
    ETST_SOIL_SOIL = 30,
    ETST_SOIL_TERRAROSA,
    ETST_SOIL_CLAYEYSOIL,
    ETST_SOIL_COLLUVIUM,
    ETST_SOIL_RENDZINA,
    ETST_SOIL_HYDROMORPHICSOIL,
    ETST_SOIL_CLAYEYDEEPSOIL,

    // VEGETATION
    ETST_VEGETATION_VEGETATION = 40,
    ETST_VEGETATION_FIELD,
    ETST_VEGETATION_GRASSLAND,

    // BLOCKED
    ETST_BLOCKED = 50
};

USTRUCT(BlueprintType)
struct AIRSIM_API FMaterialMapping : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMaterialInterface> Material;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETerrainType TerrainType = ETerrainType::ETT_AUTOMATIC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETerrainSubType TerrainSubType = ETerrainSubType::ETST_AUTOMATIC;
};