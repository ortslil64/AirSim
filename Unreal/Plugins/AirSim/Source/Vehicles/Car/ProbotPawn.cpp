#include "ProbotPawn.h"
#include <Interfaces/IPluginManager.h>

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

        if (FPaths::FileExists(excelPath) == false) {
            excelPath = "C:\\SimulationBins\\Ford350Multi.xls";
        }

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
    DeltaTime = __min(DeltaTime, 0.05);
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
            FVector Location = FVector(ItemPosition.y, ItemPosition.x, -ItemPosition.z);
            FRotator Rotation = FRotator(-ItemRotation.fPitch, ItemRotation.fYaw, ItemRotation.fRoll);

            ITnPhysicalItem::EPhysicalItemType eType;
            eType = pITnPhysicalItem->GetType();

            const char* tag = pITnPhysicalItem->GetTag();
            if (eType == ITnPhysicalItem::EPIT_BODY) {
                if (FString(tag).Equals("CHASSIS")) {
                    GetRootComponent()->SetWorldLocationAndRotation(Location, Rotation);
                }
                pSaticMesh->SetWorldLocation(Location);
                pSaticMesh->SetWorldRotation(Rotation);
            }
            else {
                pSaticMesh->SetRelativeLocation(Location);
                pSaticMesh->SetRelativeRotation(Rotation);
            }

            if (eType == ITnPhysicalItem::EPIT_SPRING) {
                void* t = (void*)pITnPhysicalItem;
                ITnSpringPhysicalItem* pSpringItem = (ITnSpringPhysicalItem*)t;
                //ITnSpringPhysicalItem* pSpringItem = dynamic_cast<ITnSpringPhysicalItem*>(pITnPhysicalItem);
                STnVector3D scale = pSpringItem->GetScale();
                pSaticMesh->SetWorldScale3D(FVector(scale.x, scale.y, scale.z));
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
