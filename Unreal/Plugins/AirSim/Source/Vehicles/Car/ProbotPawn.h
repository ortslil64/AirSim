#pragma once
#pragma warning(push)
#pragma warning(disable : 4458)

#include "CoreMinimal.h"
#include "CarPawn.h"

#include "Windows/AllowWindowsPlatformTypes.h"

#include <MotionCore/ITnAppItem.h>
#include <MotionCore/ITnMotionQueries.h>
#include <MotionCore/ITnPhysicalItemBinder.h>

#include "Windows/HideWindowsPlatformTypes.h"

#include "ProbotPawn.generated.h"

class UUnrealDTMSensor;
class ITnPhysicalItem;
class ITnVehicleMotionModel;

class UnrealAppItem : public ITnAppItem
{
public:
    UnrealAppItem(int idx)
    {
        Index = idx;
    }

    int Index;
};

UCLASS(config = Game)
class AProbotPawn : public ACarPawn
    , public ITnWheeledVehicleMotionModelListener
    , public ITnMotionQueries
    , public ITnPhysicalItemBinder
{
    GENERATED_BODY()

public:
    AProbotPawn(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void Tick(float Delta) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //     virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation,
    //                            FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

    // ITnPhysicalItemBinder override
    virtual void Bind(ITnPhysicalItem*) override;
    virtual void OnUpdate(ITnPhysicalItem** pITnPhysicalItemsArray, int numItems) override;
    bool OnCollision(ITnCollisionPointPhysicalItem** pITnCollisionPointsArray, int numItems) override;

    // ITnMotionQueries override
    virtual void GetTerrainHeight(double x, double y, bool* isHeightFound, double* pdHeight) override;
    virtual void GetTerrainMaterial(const STnVector3D& WorldPos, bool* bpMaterialFound, ITnMotionMaterial::STerrainMaterialType& TerrainMaterialType, double& moisture) override;

    // ITnWheeledVehicleMotionModelListener override
    virtual void StartTimer() override {}
    virtual void OnDataUpdate(double timeSeconds) override {}
    virtual double GetTimeSeconds() override { return 0; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UUnrealDTMSensor* DTMSensor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMeshComponent*> PlatformComponents;

    ITnVehicleMotionModel* m_pMotionModel;

private:
    void DoPhysics(float DeltaTime);

private:
    float VehicleSpeed;
    float SlowMoFactor;
};

#pragma warning(pop)