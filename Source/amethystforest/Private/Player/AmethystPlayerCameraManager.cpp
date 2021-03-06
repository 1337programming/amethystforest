
#include "amethystforest.h"
#include "Classes/Player/AmethystPlayerCameraManager.h"

AAmethystPlayerCameraManager::AAmethystPlayerCameraManager(const class FObjectInitializer& PCIP) : Super(PCIP)
{
    NormalFOV = 90.0f;
    TargetingFOV = 60.0f;
    ViewPitchMin = -87.0f;
    ViewPitchMax = 87.0f;
    bAlwaysApplyModifiers = true;
}

void AAmethystPlayerCameraManager::UpdateCamera(float DeltaTime)
{
    AAmethystCharacter* MyPawn = PCOwner ? Cast<AAmethystCharacter>(PCOwner->GetPawn()) : NULL;
    if (MyPawn && MyPawn->IsFirstPerson())
    {
        const float TargetFOV = MyPawn->IsTargeting() ? TargetingFOV : NormalFOV;
        DefaultFOV = FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, 20.0f);
    }
    
    Super::UpdateCamera(DeltaTime);
    
    if (MyPawn && MyPawn->IsFirstPerson())
    {
        MyPawn->OnCameraUpdate(GetCameraLocation(), GetCameraRotation());
    }
}
