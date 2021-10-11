#include "ProbotPawn.h"

#include <Interfaces/IPluginManager.h>

#include <MotionCore/ITnMotionCore.h>
#include <MotionCore/ITnPhysicalItem.h>
#include <MotionCore/ITnVehicleMotionModel.h>

#define LOCTEXT_NAMESPACE "VehiclePawn"

AProbotPawn::AProbotPawn()
    : ModelType(EModelType::None)
    , MaxThrottle(10.0)
    , MaxSteering(10.0)
    , MotionModel(nullptr)
    , VehicleSpeed(2)
    , SlowMoFactor(1)
{
}

void AProbotPawn::BeginPlay()
{
    Super::BeginPlay();

    if (MotionModel == nullptr) {
        MotionModel = ITnMotionCore::CreateVehicleMotionModel3D(this, this);
        MotionModel->SetListender(this);

        FString baseDir = FPaths::Combine(IPluginManager::Get().FindPlugin("AirSim")->GetBaseDir(), TEXT("/Source/AirLib/deps/MotionCore/"));
        FString configFilename;

        switch (ModelType) {
        case EModelType::None:
            UAirBlueprintLib::LogMessageString("ProbotPawn: ModelType can't be None", "", LogDebugLevel::Failure);
            UAirBlueprintLib::ShowMessage(EAppMsgType::Ok, "ProbotPawn: ModelType can't be None", "Error");
            break;
        case EModelType::Probot:
            configFilename = "Probot3DMulti.xls";
            break;
        case EModelType::Rook:
            configFilename = "Probot6X6.xls";
            break;
        }

        FString configFilePath = FPaths::Combine(baseDir, configFilename);
        if (!FPaths::FileExists(configFilePath)) {
            std::string msg = std::string("Couldn't find config file: ") + TCHAR_TO_UTF8(*configFilePath);
            UAirBlueprintLib::LogMessageString(msg, "", LogDebugLevel::Failure);
            UAirBlueprintLib::ShowMessage(EAppMsgType::Ok, msg, "Error");
        }

        bool isReload = false;
        bool ret = MotionModel->Generate(TCHAR_TO_ANSI(*configFilePath), isReload);
        if (!ret) {
            UAirBlueprintLib::LogMessageString("Motion Model couldn't be generated", "", LogDebugLevel::Failure);
        }

        InitPos = STnVector3D(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
        InitYaw = (double)GetActorLocation().Rotation().Yaw;
        InitModel();
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
    delete MotionModel;
    MotionModel = nullptr;
}

void AProbotPawn::DoPhysics(float DeltaTime)
{
    DeltaTime = FMath::Min(DeltaTime, 0.05f);
    MotionModel->Update(DeltaTime * SlowMoFactor);
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
    Exchange(x, y);
    x *= 100.0;
    y *= 100.0;
    *isFound = true;

    *pdHeight = DTMSensor->GetTerrainHeight(x, y) / 100.0;
}

void AProbotPawn::GetTerrainMaterial(const STnVector3D& WorldPos, bool* bpMaterialFound, ITnMotionMaterial::STerrainMaterialType& TerrainMaterialType, double& moisture)
{
}

void AProbotPawn::updateHUDStrings()
{
    float speed_unit_factor = AirSimSettings::singleton().speed_unit_factor;
    FText speed_unit_label = FText::FromString(FString(AirSimSettings::singleton().speed_unit_label.c_str()));
    float vel = FMath::Abs(MotionModel->GetSpeed());
    float vel_rounded = FMath::FloorToInt(vel * 10 * speed_unit_factor) / 10.0f;

    // Using FText because this is display text that should be localizable
    last_speed_ = FText::Format(LOCTEXT("SpeedFormat", "{0} {1}"), FText::AsNumber(vel_rounded), speed_unit_label);

    UAirBlueprintLib::LogMessage(TEXT("Speed: "), last_speed_.ToString(), LogDebugLevel::Informational);

    double RPM_R, RPM_L;
    MotionModel->GetEnginesRPM(RPM_R, RPM_L);
    UAirBlueprintLib::LogMessage(TEXT("RPM R: "), FText::AsNumber(RPM_R).ToString(), LogDebugLevel::Informational);
    UAirBlueprintLib::LogMessage(TEXT("RPM L: "), FText::AsNumber(RPM_L).ToString(), LogDebugLevel::Informational);
    UAirBlueprintLib::LogMessage(TEXT("Chassis Yaw: "), FText::AsNumber(MotionModel->GetChassisYaw()).ToString(), LogDebugLevel::Informational);
}

void AProbotPawn::InitModel()
{
    ITnErrors::EMotionCode ret = MotionModel->Init(InitPos, InitYaw);
    if (ret == ITnErrors::EMotionCode::SUCCESS) {
        UAirBlueprintLib::LogMessageString("Motion Model initialized successfully", "", LogDebugLevel::Informational);
    }
    else {
        UAirBlueprintLib::LogMessageString("Motion Model couldn't be initialized", "", LogDebugLevel::Failure);
    }
}

#undef LOCTEXT_NAMESPACE