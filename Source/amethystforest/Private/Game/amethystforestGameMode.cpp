

#include "amethystforest.h"
#include "Classes/Game/amethystforestGameMode.h"
#include "Classes/Player/amethystforestPlayerController.h"
#include "Classes/UI/AmethystHUD.h"

AamethystforestGameMode::AamethystforestGameMode(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
    
    // Set default pawn class to our Blueprinted character
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/AmethystCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
    
    // Set default bot pawn class to our Blueprinted bot
    static ConstructorHelpers::FClassFinder<APawn> BotPawnClassFinder(TEXT("/Game/Blueprints/AmethystBot"));
    BotPawnClass = BotPawnClassFinder.Class;
    
    PlayerControllerClass = AamethystforestPlayerController::StaticClass();
    HUDClass = AAmethystHUD::StaticClass();
    
    MinRespawnDelay = 5.0f;
    
    bAllowBots = true;
}

void AamethystforestGameMode::StartPlay()
{
    Super::StartPlay();
    
    StartMatch();
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("HELLO WORLD"));
    }
}

FString AamethystforestGameMode::GetBotsCountOptionName()
{
    return FString(TEXT("Bots"));
}

void AamethystforestGameMode::SetAllowBots(bool bInAllowBots, int32 InMaxBots)
{
    bAllowBots = bInAllowBots;
    MaxBots = InMaxBots;
}

void AamethystforestGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    const int32 BotsCountOptionValue = UGameplayStatics::GetIntOption(Options, GetBotsCountOptionName(), 0);
    SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);
    
    Super::InitGame(MapName, Options, ErrorMessage);
}

void AamethystforestGameMode::RequestFinishAndExitToMainMenu()
{
    
    APlayerController* LocalPrimaryController = nullptr;
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* Controller = *Iterator;
        if (Controller && !Controller->IsLocalController())
        {
            Controller->ClientReturnToMainMenu("Return To Main Menu");
        }
        else
        {
            LocalPrimaryController = Controller;
        }
    }
    
    if (LocalPrimaryController != NULL)
    {
        LocalPrimaryController->ClientReturnToMainMenu("Return To Main Menu");
    }
}

bool AamethystforestGameMode::AllowCheats(APlayerController* P)
{
    return true;
}

bool AamethystforestGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
    return false;
}

AActor* AamethystforestGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    TArray<APlayerStart*> PreferredSpawns;
    TArray<APlayerStart*> FallbackSpawns;
    
    APlayerStart* BestStart = NULL;
    for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
    {
        APlayerStart* TestSpawn = *It;
        if (TestSpawn->IsA<APlayerStartPIE>())
        {
            // Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
            BestStart = TestSpawn;
            break;
        }
        else
        {
            if (IsSpawnpointAllowed(TestSpawn, Player))
            {
                if (IsSpawnpointPreferred(TestSpawn, Player))
                {
                    PreferredSpawns.Add(TestSpawn);
                }
                else
                {
                    FallbackSpawns.Add(TestSpawn);
                }
            }
        }
    }
    
    
    if (BestStart == NULL)
    {
        if (PreferredSpawns.Num() > 0)
        {
            BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
        }
        else if (FallbackSpawns.Num() > 0)
        {
            BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
        }
    }
    
    return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}

UClass* AamethystforestGameMode::GetDefaultPawnClassForController(AController* InController)
{
    if (Cast<AAmethystAIController>(InController))
    {
        return BotPawnClass;
    }
    
    return Super::GetDefaultPawnClassForController(InController);
}

bool AamethystforestGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
    /* TO DO: TeamStart or Spawn Location Class
    AAmethystTeamStart* AmethystSpawnPoint = Cast<AAmethystTeamStart>(SpawnPoint);
    if (AmethystSpawnPoint)
    {
        AAmethystAIController* AIController = Cast<AAmethystAIController>(Player);
        if (AmethystSpawnPoint->bNotForBots && AIController)
        {
            return false;
        }
        
        if (AmethystSpawnPoint->bNotForPlayers && AIController == NULL)
        {
            return false;
        }
    }
    */
    //return true;
    return false; // Remove
    
}

bool AamethystforestGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
    ACharacter* MyPawn = Player ? Cast<ACharacter>(Player->GetPawn()) : NULL;
    if (MyPawn)
    {
        const FVector SpawnLocation = SpawnPoint->GetActorLocation();
        for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
        {
            ACharacter* OtherPawn = Cast<ACharacter>(*It);
            if (OtherPawn && OtherPawn != MyPawn)
            {
                const float CombinedHeight = (MyPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
                const float CombinedRadius = MyPawn->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
                const FVector OtherLocation = OtherPawn->GetActorLocation();
                
                // check if player start overlaps this pawn
                if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedRadius)
                {
                    return false;
                }
            }
        }
    }
    
    return true;
}

class AAmethystBot* AamethystforestGameMode::SpawnBot(FVector SpawnLocation, FRotator SpawnRotation)
{
    if (BotPawnClass)
    {
        FActorSpawnParameters SpawnInfo;
        SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        //SpawnInfo.bNoCollisionFail = true;
        AAmethystBot* Bot = GetWorld()->SpawnActor<AAmethystBot>(BotPawnClass, SpawnLocation, SpawnRotation, SpawnInfo);
        if (Bot)
        {
            Bot->SpawnDefaultController();
            return Bot;
        }
    }
    
    return NULL;
}

void AamethystforestGameMode::SpawnBotsForGame()
{
    // getting max number of players
    int32 MaxPlayers = -1;
    if (GameSession)
    {
        MaxPlayers = GameSession->MaxPlayers;
    }
    
    // checking number of human players
    int32 NumPlayers = 0;
    for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
    {
        AamethystforestPlayerController* PC = Cast<AamethystforestPlayerController>(*It);
        if (PC)
        {
            ++NumPlayers;
        }
    }
    
    // adding bots
    BotControllers.Empty();
    int32 NumBots = 0;
    while (NumPlayers < MaxPlayers && NumBots < MaxBots)
    {
        AAmethystBot* Bot = SpawnBot(FVector(ForceInitToZero), FRotator(ForceInitToZero));
        if (Bot)
        {
            InitBot(Bot, NumBots + 1);
            ++NumBots;
        }
    }
}

void AamethystforestGameMode::InitBot(AAmethystBot* Bot, int BotNumber)
{
    AAmethystAIController* AIPC = Bot ? Cast<AAmethystAIController>(Bot->GetController()) : NULL;
    if (AIPC)
    {
        if (AIPC->PlayerState)
        {
            FString BotName = FString::Printf(TEXT("Bot %d"), BotNumber);
            AIPC->PlayerState->PlayerName = BotName;
        }
        AActor* BestStart = ChoosePlayerStart(AIPC);
        UE_LOG(LogAmethyst, Log, TEXT("InitBot: %s spawned on %s"), *Bot->GetName(), *BestStart->GetName());
        Bot->TeleportTo(BestStart->GetActorLocation(), BestStart->GetActorRotation(), false, true);
    }
}

