// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnrealDTMSensor.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UUnrealDTMSensor : public USceneCaptureComponent2D
{
    GENERATED_BODY()

public:
    double GetTerrainHeight(double x, double y);

    UPROPERTY(EditAnywhere)
    bool IsForceCPU;
    UUnrealDTMSensor();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    void InitRenderTarget();
    void ReadDepth();
    UMaterial* DTMSensorMaterial;

    UPROPERTY(EditAnywhere)
    AActor* OtherActor;

    uint8* m_pHeightMap;

    	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTextureRenderTarget2D* RenderTarget = nullptr;
};
