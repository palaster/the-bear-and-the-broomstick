#include "GridActor.h"
#include "Components/BoxComponent.h"

// Sets default values
AGridActor::AGridActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GridCollision = CreateDefaultSubobject<UBoxComponent>("GridCollision");
	GridCollision->SetBoxExtent(FVector(25.0f, 25.0f, 25.0f), true);
	GridCollision->SetGenerateOverlapEvents(true);
	GridCollision->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	GridCollision->SetCollisionProfileName("OverlapAll", true);
	GridCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
	RootComponent = GridCollision;
}