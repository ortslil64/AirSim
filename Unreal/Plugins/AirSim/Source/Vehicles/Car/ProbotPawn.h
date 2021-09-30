#pragma once
#pragma warning(push)
#pragma warning(disable : 4458)

#include "CoreMinimal.h"
#include "CarPawn.h"

#include "Windows/AllowWindowsPlatformTypes.h"

#include <STnVector.h>
#include <MotionCore/ITnMotionQueries.h>
#include <MotionCore/ITnPhysicalItem.h>
#include <MotionCore/ITnPhysicalItemBinder.h>
#include <MotionCore/ITnAppItem.h>
#include <MotionCore/ITnVehicleMotionModel.h>
#include <ControlCore/ITnMotionControlPath.h>
#include <MotionCore/ITnMotionCore.h>

#include "Windows/HideWindowsPlatformTypes.h"

#include "ProbotPawn.generated.h"

class UUnrealDTMSensor;

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

	ITnVehicleMotionModel* m_pMotionModel;
    float VehicleSpeed;
    float SlowMoFactor;
        void DoPhysics(float DeltaTime);

        virtual void Bind(ITnPhysicalItem*) override;
        virtual void OnUpdate(ITnPhysicalItem** pITnPhysicalItemsArray, int numItems) override;
        bool OnCollision(ITnCollisionPointPhysicalItem** pITnCollisionPointsArray, int numItems) override;

        virtual void GetTerrainHeight(double x, double y, bool* isHeightFound, double* pdHeight) override;
        
	    virtual void GetTerrainMaterial(const STnVector3D& WorldPos, bool* bpMaterialFound, ITnMotionMaterial::STerrainMaterialType& TerrainMaterialType, double& moisture) override;

        virtual void StartTimer() override {}
            virtual void OnDataUpdate(double timeSeconds) override {}
        virtual double GetTimeSeconds() override { return 0; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UUnrealDTMSensor* DTMSensor;

    	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<UStaticMeshComponent*> PlatformComponents;

//         private:
//     void setupInputBindings();
//     void onMoveForward(float Val);
//     void onMoveRight(float Val);
//     void onHandbrakePressed();
//     void onHandbrakeReleased();
//     void onFootBrake(float Val);
//     void onReversePressed();
//     void onReverseReleased();
};

 #pragma warning(pop)