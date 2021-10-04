#include "ProbotPawn.h"

#include <Interfaces/IPluginManager.h>

#include <MotionCore/ITnMotionCore.h>
#include <MotionCore/ITnPhysicalItem.h>
#include <MotionCore/ITnVehicleMotionModel.h>

AProbotPawn::AProbotPawn(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , m_pMotionModel(nullptr)
    , VehicleSpeed(2)
    , SlowMoFactor(1)
{
}

void AProbotPawn::BeginPlay()
{
    Super::BeginPlay();

    if (m_pMotionModel == nullptr) {
        m_pMotionModel = ITnMotionCore::CreateVehicleMotionModel3D(this, this);
        m_pMotionModel->SetListender(this);
        FString BaseDir = IPluginManager::Get().FindPlugin("AirSim")->GetBaseDir();

        FString excelPath = BaseDir + "/Source/AirLib/deps/MotionCore/Probot3DMulti.xls";

        bool isReload = false;
        m_pMotionModel->Generate(TCHAR_TO_ANSI(*excelPath), isReload);

        STnVector3D InitPos(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
        m_pMotionModel->Init(InitPos, (double)GetActorLocation().Rotation().Yaw);
    }

    AddTickPrerequisiteComponent(DTMSensor);
}

void AProbotPawn::Tick(float Delta)
{
    DoPhysics(Delta);
    Super::Tick(Delta);
    pawn_events_.getPawnTickSignal().emit(Delta);
}

void AProbotPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    delete m_pMotionModel;
    m_pMotionModel = nullptr;
}

void AProbotPawn::DoPhysics(float DeltaTime)
{
    DeltaTime = FMath::Min(DeltaTime, 0.05f);
    m_pMotionModel->Update(DeltaTime * SlowMoFactor);
    bool ret = m_pMotionModel->IsCollisionDetected();
}

void AProbotPawn::Bind(ITnPhysicalItem* pItem)
{
    ITnPhysicalItem::EPhysicalItemType eType;
    eType = pItem->GetType();
    const char* itemID = pItem->GetTag();
    ITnAppItem* pAppItem;

    if (eType == ITnPhysicalItem::EPIT_CP) { // Collision point
        ;
    }
    else {
        for (int idx = 0; idx < PlatformComponents.Num(); idx++) {
            UStaticMeshComponent* pMesh = PlatformComponents[idx];

            if (pMesh != nullptr &&
                pMesh->ComponentTags.Contains(itemID)) {
                pAppItem = new UnrealAppItem(idx);
                pItem->SetAppItem(pAppItem);
            }
        }
    }
}

void AProbotPawn::OnUpdate(ITnPhysicalItem** pITnPhysicalItemsArray, int numItems)
{
    for (int i = 0; i < numItems; i++) {
        ITnPhysicalItem* pITnPhysicalItem = pITnPhysicalItemsArray[i];

        if (pITnPhysicalItem->AppItemExists()) {
            ITnAppItem* pAppItem = pITnPhysicalItem->GetAppItem();
            UnrealAppItem* pUnrealItem = (UnrealAppItem*)pAppItem;
            UStaticMeshComponent* pSaticMesh = PlatformComponents[pUnrealItem->Index];

            STnFVector3D ItemPosition = pITnPhysicalItem->GetGlobalPosition();
            STnRotation ItemRotation = pITnPhysicalItem->GetGlobalRotation();

            ItemPosition *= 100;
            FVector Location = FVector(ItemPosition.y, ItemPosition.x, ItemPosition.z);
            FRotator Rotation = FRotator(-ItemRotation.fPitch, ItemRotation.fYaw, ItemRotation.fRoll);

            ITnPhysicalItem::EPhysicalItemType eType;
            eType = pITnPhysicalItem->GetType();

            const char* tag = pITnPhysicalItem->GetTag();
            if (eType == ITnPhysicalItem::EPIT_BODY) {
                if (FString(tag).Equals("CHASSIS")) {
                    // We need to update location and rotation of the root CarPawn (AirSim) component
                    // to make AirSim features work in this platform.
                    // This is a workaround to "attach" the root to the chassis, bc inherited component can't be moved.
                    GetRootComponent()->SetWorldLocationAndRotation(Location, Rotation);
                }
                pSaticMesh->SetWorldLocation(Location);
                pSaticMesh->SetWorldRotation(Rotation);
            }
            else {
                // Handle the platform components which need to have relative transform, instead world transform
                pSaticMesh->SetRelativeLocation(Location);
                pSaticMesh->SetRelativeRotation(Rotation);
            }
        }
    }
}

bool AProbotPawn::OnCollision(ITnCollisionPointPhysicalItem** pITnCollisionPointsArray, int numItems)
{
    return false;
}

void AProbotPawn::GetTerrainHeight(double x, double y, bool* isFound, double* pdHeight)
{
    double temp = x;
    x = y;
    y = temp;
    x *= 100.0;
    y *= 100.0;
    *isFound = true;

    *pdHeight = DTMSensor->GetTerrainHeight(x, y) / 100.0;
}

void AProbotPawn::GetTerrainMaterial(const STnVector3D& WorldPos, bool* bpMaterialFound, ITnMotionMaterial::STerrainMaterialType& TerrainMaterialType, double& moisture)
{
}
