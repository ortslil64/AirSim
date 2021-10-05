// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealDTMSensor.h"
#include <memory>
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include <UObject/ConstructorHelpers.h>

UUnrealDTMSensor::UUnrealDTMSensor()
    : m_pHeightMap(nullptr)
{
    ConstructorHelpers::FObjectFinder<UMaterial> DTMMaterialRes(TEXT("Material'/Game/Materials/DTMSensor/DTMSensorMaterial.DTMSensorMaterial'"));
    if (DTMMaterialRes.Object != NULL) {
        DTMSensorMaterial = DTMMaterialRes.Object;
    }

    CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    ProjectionType = ECameraProjectionMode::Type::Orthographic;
    Deactivate();
    TextureTarget = RenderTarget;
    bCaptureOnMovement = false;
    bCaptureEveryFrame = false;
    bAlwaysPersistRenderingState = true;
    OrthoWidth = 512;
    UpdateContent();
    Activate();
}

void UUnrealDTMSensor::BeginPlay()
{
    InitRenderTarget();
    Super::BeginPlay();
}

void UUnrealDTMSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (m_pHeightMap != nullptr)
    {
        delete m_pHeightMap;
        m_pHeightMap = nullptr;
    }
}

void UUnrealDTMSensor::InitRenderTarget()
{
    RenderTarget = NewObject<UTextureRenderTarget2D>(this);
    RenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
    RenderTarget->SRGB = false;
    RenderTarget->bAutoGenerateMips = false;
    RenderTarget->AddressX = TextureAddress::TA_Clamp;
    RenderTarget->AddressY = TextureAddress::TA_Clamp;
    RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_R32f;
    RenderTarget->InitAutoFormat(512, 512);
    TextureTarget = RenderTarget;
    PostProcessSettings.AddBlendable(DTMSensorMaterial, 1.0f);
}

void UUnrealDTMSensor::ReadDepth()
{
    const uint32 num_bytes_per_pixel = 4; // PF_R32

    FRHITexture2D* pTexture = RenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();

    int m_width = RenderTarget->GetSurfaceWidth();
    int m_height = RenderTarget->GetSurfaceHeight();

    const uint32 dest_stride = m_width * m_height * num_bytes_per_pixel;
    uint32 src_stride;
    uint8* src = reinterpret_cast<uint8*>(
        RHILockTexture2D(pTexture, 0, RLM_ReadOnly, src_stride, false));

    std::unique_ptr<uint8[]> dest = nullptr;
    // Direct 3D uses additional rows in the buffer,so we need check the result
    // stride from the lock:
    if (IsD3DPlatform(GMaxRHIShaderPlatform, false) && (dest_stride != src_stride)) {
        const uint32 copy_row_stride = m_width * num_bytes_per_pixel;

        if (m_pHeightMap == nullptr) {
            m_pHeightMap = new uint8[dest_stride];
        }

        dest = std::make_unique<uint8[]>(dest_stride);

        // Copy per row
        uint8* dest_row = m_pHeightMap;
        uint8* src_row = src;
        for (uint32 Row = 0; Row < (uint32)m_height; ++Row) {
            FMemory::Memcpy(dest_row, src_row, copy_row_stride);
            dest_row += copy_row_stride;
            src_row += src_stride;
        }

        src = dest.get();
    }

    RHIUnlockTexture2D(pTexture, 0, false);
}

void UUnrealDTMSensor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    if (GetWorld()->WorldType == EWorldType::Editor) {
        return;
    }

    if (TextureTarget == nullptr) {
        InitRenderTarget();
    }

    CaptureScene();

    ENQUEUE_RENDER_COMMAND(FWritePixels)
    (
        [this](FRHICommandListImmediate& RHICmdList) {
            ReadDepth();
        });
}

double UUnrealDTMSensor::GetTerrainHeight(double x, double y)
{
    bool IsNoHeightMap = false;

    if (m_pHeightMap == nullptr) {
        IsNoHeightMap = true;
    }

    float* pFloatHeightMap = (float*)m_pHeightMap;

    int m_width = 0;
    int m_height = 0;

    if (RenderTarget != nullptr) {
        m_width = RenderTarget->GetSurfaceWidth();
        m_height = RenderTarget->GetSurfaceHeight();
    }

    FVector selfWorldPos = GetComponentLocation();
    FVector topLeft = selfWorldPos - FVector(-m_width / 2, m_height / 2, 0);

    int mapX = abs(x - topLeft.X);
    int mapY = abs(y - topLeft.Y);

    int arrayPos = mapX * m_width + mapY;

    float height = 0;

    bool isCPU = IsForceCPU || IsNoHeightMap || arrayPos > m_height * m_width || arrayPos < 0;

    if (isCPU) { // Out of bounds. Do a CPU call.

        FHitResult hitResult;
        FVector startPos;
        startPos.X = x;
        startPos.Y = y;
        startPos.Z = 10000;

        FVector endPos = startPos;
        endPos.Z = -10000;

        FCollisionQueryParams qParams;
        qParams.AddIgnoredActor(GetOwner());
        bool isHit = GetWorld()->LineTraceSingleByChannel(hitResult, startPos, endPos, ECC_Vehicle, qParams);

        if (isHit) {
            height = hitResult.ImpactPoint.Z;
        }
    }
    else { // Use GPU

        height = pFloatHeightMap[arrayPos];
    }

    return height;
}