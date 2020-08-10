#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridActor.generated.h"

class UBoxComponent;

UCLASS()
class BEARBROOMSTICK_API AGridActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn = "true"))
	AGridActor* PreviousGrid;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ExposeOnSpawn = "true"))
	TArray<AGridActor*> NextGrids;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AGridActor*> Rings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int Iteration = -1;

	UBoxComponent* GetGridCollision() const { return GridCollision; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* GridCollision;
};