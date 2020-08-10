#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaypointActor.generated.h"

class USceneComponent;
class UBillboardComponent;
class UArrowComponent;

UCLASS()
class BEARBROOMSTICK_API AWaypointActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaypointActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	AWaypointActor* PreviousWaypoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	AWaypointActor* NextWaypoint;

	USceneComponent* GetScene() const { return Scene; }
	UBillboardComponent* GetBillboard() const { return Billboard; }
	UArrowComponent* GetArrow() const { return Arrow; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UBillboardComponent* Billboard;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UArrowComponent* Arrow;
};