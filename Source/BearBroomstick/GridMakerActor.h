#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridMakerActor.generated.h"

class AGridActor;
class AWaypointActor;

UENUM(BlueprintType)
enum class EDirection : uint8
{
	D_None UMETA(DisplayName = "Nonek"),
	D_Up UMETA(DisplayName = "Up"),
	D_Down UMETA(DisplayName = "Down"),
	D_Left UMETA(DisplayName = "Left"),
	D_Right UMETA(DisplayName = "Right"),
	D_Forward UMETA(DisplayName = "Forward"),
	D_Backward UMETA(DisplayName = "Backward")
};

UCLASS()
class BEARBROOMSTICK_API AGridMakerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGridMakerActor();

	UPROPERTY(BlueprintReadOnly)
	FVector Direction = FVector(NAN, NAN, NAN);

	UPROPERTY(BlueprintReadOnly)
	AGridActor* StartGrid;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> ActorsToIgnore;

	UPROPERTY(BlueprintReadOnly)
	TArray<AWaypointActor*> CreatedWaypoints;

	UPROPERTY(BlueprintReadOnly)
	FVector SmallestSizeToFit;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> KnownObstacles;

	UPROPERTY(BlueprintReadOnly)
	TArray<AGridActor*> OriginalGrids;

	UPROPERTY(BlueprintReadOnly)
	int MaxRingSize;

	UFUNCTION(BlueprintCallable)
	void MakeGrid(FVector CurrentLocation, FVector TargetLocation, TArray<AActor*> NewActorsToIgnore, FVector NewSmallestSizeToFit, TArray<AActor*> NewKnownObstacles, int NewMaxRingSize = 5);

	UFUNCTION(BlueprintCallable)
	void DestroyGrid(AGridActor* Grid);

	UFUNCTION()
	bool IsValidGrid(AGridActor* GridToValidate);

	UFUNCTION(BlueprintCallable)
	bool HasValidPath();

	UFUNCTION()
	bool HasValidPathHelper(AGridActor* Grid);

	UFUNCTION()
	bool GetValidPath(TArray<AGridActor*>& Grids);

	UFUNCTION()
	bool GetValidPathHelper(AGridActor* Grid, TArray<AGridActor*>& Grids);

	UFUNCTION(BlueprintCallable)
	void MakeValidPath();

	UFUNCTION()
	AGridActor* GetLastGridWithProblem();

	UFUNCTION()
	AGridActor* GetLastGridWithProblemHelper(AGridActor* Grid);

	UFUNCTION()
	void SpawnNewRing(AGridActor* ProblemGrid, int Iteration, bool bIsNestedCall = false);

	UFUNCTION(BlueprintCallable)
	void CreateWaypointsFromValidPath(AWaypointActor* StartingWaypoint);

	UFUNCTION()
	TArray<AGridActor*> AddGridToHeadOfGridArray(AGridActor* GridToAdd, TArray<AGridActor*> GridArray)
	{
		TArray<AGridActor*> NewGridArray;
		NewGridArray.Add(GridToAdd);
		for(AGridActor* FoundGrid : GridArray)
			NewGridArray.Add(FoundGrid);
		return NewGridArray;
	}

	virtual void Destroyed() override;
};
