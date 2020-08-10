#include "GridMakerActor.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GridActor.h"
#include "WaypointActor.h"

// Sets default values
AGridMakerActor::AGridMakerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AGridMakerActor::MakeGrid(FVector CurrentLocation, FVector TargetLocation, TArray<AActor*> NewActorsToIgnore, FVector NewSmallestSizeToFit, TArray<AActor*> NewKnownObstacles, int NewMaxRingSize)
{
	if(IsValid(StartGrid))
		DestroyGrid(nullptr);
	SmallestSizeToFit = NewSmallestSizeToFit;
	KnownObstacles = NewKnownObstacles;
	MaxRingSize = NewMaxRingSize;
	AGridActor* FirstGrid = GetWorld()->SpawnActorDeferred<AGridActor>(AGridActor::StaticClass(), FTransform(CurrentLocation), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if(IsValid(FirstGrid))
	{
		FirstGrid->GetGridCollision()->SetBoxExtent(SmallestSizeToFit, true);
		FirstGrid = Cast<AGridActor>(UGameplayStatics::FinishSpawningActor(FirstGrid, FTransform(CurrentLocation)));
	}
	if(IsValid(FirstGrid))
	{
		NewActorsToIgnore.Add(FirstGrid);
		OriginalGrids.Add(FirstGrid);
		StartGrid = FirstGrid;
		FVector StepVector = CurrentLocation;
		if(UKismetMathLibrary::IsPointInBox(StepVector, TargetLocation, SmallestSizeToFit))
		{
			ActorsToIgnore = NewActorsToIgnore;
			return;
		}
		else
		{
			AGridActor* CurrentGrid = StartGrid;
			while(!UKismetMathLibrary::IsPointInBox(StepVector, TargetLocation, SmallestSizeToFit))
			{
				FVector NewGridLocation = (UKismetMathLibrary::GetDirectionUnitVector(CurrentLocation, TargetLocation) * (SmallestSizeToFit.X * 2.0f)) + StepVector;
				AGridActor* NewGrid = GetWorld()->SpawnActorDeferred<AGridActor>(AGridActor::StaticClass(), FTransform(NewGridLocation), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if(IsValid(NewGrid))
				{
					NewGrid->PreviousGrid = CurrentGrid;
					NewGrid->GetGridCollision()->SetBoxExtent(SmallestSizeToFit, true);
					NewGrid = Cast<AGridActor>(UGameplayStatics::FinishSpawningActor(NewGrid, FTransform(NewGridLocation)));
				}
				if(IsValid(NewGrid))
				{
					NewActorsToIgnore.Add(NewGrid);
					OriginalGrids.Add(NewGrid);
					CurrentGrid->NextGrids.Add(NewGrid);
					CurrentGrid = NewGrid;
					StepVector = NewGridLocation;
				}
			}
			ActorsToIgnore = NewActorsToIgnore;
			return;
		}
	}
}

void AGridMakerActor::DestroyGrid(AGridActor* Grid)
{
	if(IsValid(Grid) && !Grid->IsPendingKillPending())
	{
		if(Grid->NextGrids.Num() > 0)
			for(AGridActor* NextGrid: Grid->NextGrids)
				if(IsValid(NextGrid) && !NextGrid->IsPendingKillPending())
					DestroyGrid(NextGrid);
		Grid->Destroy();
	}
	else
	{
		if(IsValid(StartGrid) && !StartGrid->IsPendingKillPending())
		{
			if(StartGrid->NextGrids.Num() > 0)
				for(AGridActor* NextGrid: StartGrid->NextGrids)
					if(IsValid(NextGrid) && !NextGrid->IsPendingKillPending())
						DestroyGrid(NextGrid);
			StartGrid->Destroy();
		}
		StartGrid = nullptr;
		for(AWaypointActor* CreatedWaypoint : CreatedWaypoints)
			CreatedWaypoint->Destroy();
		CreatedWaypoints.Empty();
	}
}

bool AGridMakerActor::IsValidGrid(AGridActor* GridToValidate)
{
	if(!IsValid(GridToValidate))
		return false;
	for(AActor* KnownObstacle : KnownObstacles)
	{
		FVector Origin = FVector(NAN, NAN, NAN);
		FVector BoxExtent = FVector(NAN, NAN, NAN);
		KnownObstacle->GetActorBounds(true, Origin, BoxExtent, true);
		if(!Origin.ContainsNaN() && !BoxExtent.ContainsNaN() && UKismetMathLibrary::IsPointInBox(GridToValidate->GetActorLocation(), Origin, BoxExtent))
			return false;
	}
	TArray<AActor*> OverlappingActors;
	GridToValidate->GetGridCollision()->GetOverlappingActors(OverlappingActors, AActor::StaticClass());
	if(OverlappingActors.Num() > 0)
		for(AActor* OverlappingActor : OverlappingActors)
			if(!ActorsToIgnore.Contains(OverlappingActor))
				return false;
	return true;
}

bool AGridMakerActor::HasValidPath()
{
	return IsValid(StartGrid) ? HasValidPathHelper(StartGrid) : false;
}

bool AGridMakerActor::HasValidPathHelper(AGridActor* Grid)
{
	if(!IsValidGrid(Grid))
		return false;
	if(Grid->NextGrids.Num() > 0)
	{
		for(int i = 0; i < Grid->NextGrids.Num(); i++)
			if(HasValidPathHelper(Grid->NextGrids[i]))
				return true;
		return false;
	}
	return true;
}

bool AGridMakerActor::GetValidPath(TArray<AGridActor*>& Grids)
{
	if(!IsValidGrid(StartGrid))
		return false;
	if(StartGrid->NextGrids.Num() > 0)
	{
		for(AGridActor* NextGrid : StartGrid->NextGrids)
		{
			TArray<AGridActor*> MadeGrid;
			if(GetValidPathHelper(NextGrid, MadeGrid))
			{
				Grids = AddGridToHeadOfGridArray(StartGrid, MadeGrid);
				return true;
			}
		}
		return false;
	}
	TArray<AGridActor*> OneArray;
	OneArray.Add(StartGrid);
	Grids = OneArray;
	return true;
}

bool AGridMakerActor::GetValidPathHelper(AGridActor* Grid, TArray<AGridActor*>& Grids)
{
	if(!IsValidGrid(Grid) || Grid->Iteration > -1)
		return false;
	if(Grid->NextGrids.Num() > 0)
	{
		for(AGridActor* NextGrid : Grid->NextGrids)
		{
			TArray<AGridActor*> MadeGrid;
			if(GetValidPathHelper(NextGrid, MadeGrid))
			{
				Grids = AddGridToHeadOfGridArray(Grid, MadeGrid);
				return true;
			}
		}
		return false;
	}
	TArray<AGridActor*> OneArray;
	OneArray.Add(Grid);
	Grids = OneArray;
	return true;
}

void AGridMakerActor::MakeValidPath()
{
	AGridActor* ProblemGrid = nullptr;
	int Iteration = 0;
	while(!HasValidPath())
	{
		if((Iteration + 1) >= MaxRingSize)
			break;
		AGridActor* NewProblemGrid = GetLastGridWithProblem();
		if(IsValid(NewProblemGrid))
		{
			if(IsValid(ProblemGrid) && ProblemGrid == NewProblemGrid)
				Iteration++;
			else
			{
				ProblemGrid = NewProblemGrid;
				Iteration = 0;
			}
			SpawnNewRing(ProblemGrid, Iteration);
		}
		else
			break;
	}
	// Average Rings (Change from Outside == i affects i + 1 && i - 1)
	for(int  i = 0; i < OriginalGrids.Num(); i++)
		if(IsValid(OriginalGrids[i]) && OriginalGrids[i]->Iteration >= 1)
		{
			if(i == 0)
			{
				if(IsValid(OriginalGrids[i + 1]) && OriginalGrids[i + 1]->Iteration == -1)
					SpawnNewRing(OriginalGrids[i + 1], OriginalGrids[i]-> Iteration);
			}
			else if(i == OriginalGrids.Num() - 1)
			{
				if(IsValid(OriginalGrids[i - 1]) && OriginalGrids[i - 1]->Iteration == -1)
					SpawnNewRing(OriginalGrids[i - 1], OriginalGrids[i]-> Iteration);
			}
			else
			{
				if(IsValid(OriginalGrids[i - 1]) && OriginalGrids[i - 1]->Iteration == -1)
					SpawnNewRing(OriginalGrids[i - 1], OriginalGrids[i]-> Iteration);
				if(IsValid(OriginalGrids[i + 1]) && OriginalGrids[i + 1]->Iteration == -1)
					SpawnNewRing(OriginalGrids[i + 1], OriginalGrids[i]-> Iteration);
			}
		}
	// Average Rings (Change from Inside == i affects i from i + 1 && i - 1)
}

AGridActor* AGridMakerActor::GetLastGridWithProblem()
{
	return IsValid(StartGrid) ? GetLastGridWithProblemHelper(StartGrid) : nullptr;
}

AGridActor* AGridMakerActor::GetLastGridWithProblemHelper(AGridActor* Grid)
{
	if(!IsValid(Grid))
		return nullptr;
	if(Grid->NextGrids.Num() > 0)
	{
		TArray<AGridActor*> ProblemGrids;
		for(AGridActor* NextGrid : Grid->NextGrids)
			ProblemGrids.Add(GetLastGridWithProblemHelper(NextGrid));
		if(!ProblemGrids.Contains(nullptr))
			for(AGridActor* ProblemGrid : ProblemGrids)
				if(IsValid(ProblemGrid))
					return ProblemGrid;
	}
	return IsValidGrid(Grid) ? nullptr : Grid;
}

void AGridMakerActor::SpawnNewRing(AGridActor* ProblemGrid, int Iteration, bool bIsNestedCall)
{
	if(Iteration < 0)
		return;
	if(!IsValid(ProblemGrid) || ProblemGrid->IsPendingKillPending())
		return;
	AGridActor* PreviousGrid = IsValid(ProblemGrid->PreviousGrid) ? ProblemGrid->PreviousGrid : ProblemGrid;
	TArray<AGridActor*> NextGrids;
	if(IsValid(ProblemGrid->PreviousGrid))
	{
		if(ProblemGrid->NextGrids.Num() > 0)
			NextGrids = ProblemGrid->NextGrids;
	}
	else
	{
		for(AGridActor* TempGrid : ProblemGrid->NextGrids)
			NextGrids.Add(TempGrid);
		ProblemGrid->NextGrids.Empty();
	}
	ProblemGrid->Iteration = Iteration;
	FVector StartLocation = ProblemGrid->GetActorLocation();
	int NumberToSpawn = 3 + (Iteration * 2);
	int FirstIndex = 0 - (NumberToSpawn / 2);
	int LastIndex = NumberToSpawn / 2;
	for(int Z = FirstIndex; Z <= LastIndex; Z++)
		for(int X = FirstIndex; X <= LastIndex; X++)
		{
			if(Z == FirstIndex || Z == LastIndex)
			{
				FVector NewGridLocation = FVector(StartLocation.X + (X * (SmallestSizeToFit.X * 2.0f)), StartLocation.Y, StartLocation.Z + (Z * (SmallestSizeToFit.X * 2.0f)));
				AGridActor* NewGrid = GetWorld()->SpawnActorDeferred<AGridActor>(AGridActor::StaticClass(), FTransform(NewGridLocation), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if(IsValid(NewGrid))
				{
					NewGrid->PreviousGrid = PreviousGrid;
					NewGrid->NextGrids = NextGrids;
					NewGrid->GetGridCollision()->SetBoxExtent(SmallestSizeToFit, true);
					NewGrid = Cast<AGridActor>(UGameplayStatics::FinishSpawningActor(NewGrid, FTransform(NewGridLocation)));
				}
				if(IsValid(NewGrid))
				{
					PreviousGrid->NextGrids.Add(NewGrid);

					if(ProblemGrid != PreviousGrid && PreviousGrid->Rings.Num() > 0)
						for(AGridActor* PreviousRingGrid : PreviousGrid->Rings)
							PreviousRingGrid->NextGrids.Add(NewGrid);

					ProblemGrid->Rings.Add(NewGrid);
					ActorsToIgnore.Add(NewGrid);
				}
			}
			else if(X == FirstIndex || X == LastIndex)
			{
				FVector NewGridLocation = FVector(StartLocation.X + (X * (SmallestSizeToFit.X * 2.0f)), StartLocation.Y, StartLocation.Z + (Z * (SmallestSizeToFit.X * 2.0f)));
				AGridActor* NewGrid = GetWorld()->SpawnActorDeferred<AGridActor>(AGridActor::StaticClass(), FTransform(NewGridLocation), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				if(IsValid(NewGrid))
				{
					NewGrid->PreviousGrid = PreviousGrid;
					NewGrid->NextGrids = NextGrids;
					NewGrid->GetGridCollision()->SetBoxExtent(SmallestSizeToFit, true);
					NewGrid = Cast<AGridActor>(UGameplayStatics::FinishSpawningActor(NewGrid, FTransform(NewGridLocation)));
				}
				if(IsValid(NewGrid))
				{
					PreviousGrid->NextGrids.Add(NewGrid);

					if(ProblemGrid != PreviousGrid && PreviousGrid->Rings.Num() > 0)
						for(AGridActor* PreviousRingGrid : PreviousGrid->Rings)
							PreviousRingGrid->NextGrids.Add(NewGrid);

					ProblemGrid->Rings.Add(NewGrid);
					ActorsToIgnore.Add(NewGrid);
				}
			}
		}
}

void AGridMakerActor::CreateWaypointsFromValidPath(AWaypointActor* StartingWaypoint)
{
	if(!IsValid(StartingWaypoint) || !IsValid(StartingWaypoint->NextWaypoint))
		return;
	AWaypointActor* EndWaypoint = StartingWaypoint->NextWaypoint;
	AWaypointActor* CurrentWaypoint = StartingWaypoint;
	TArray<AGridActor*> ValidPath;
	if(GetValidPath(ValidPath))
	{
		for(AGridActor* ValidGrid : ValidPath)
		{
			AWaypointActor* NewWaypoint = GetWorld()->SpawnActorDeferred<AWaypointActor>(AWaypointActor::StaticClass(), FTransform(ValidGrid->GetActorLocation()), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			if(IsValid(NewWaypoint))
			{
				NewWaypoint->PreviousWaypoint = CurrentWaypoint;
				NewWaypoint = Cast<AWaypointActor>(UGameplayStatics::FinishSpawningActor(NewWaypoint, FTransform(ValidGrid->GetActorLocation())));
			}
			if(IsValid(NewWaypoint))
			{
				CurrentWaypoint->NextWaypoint = NewWaypoint;
				CreatedWaypoints.Add(NewWaypoint);
				CurrentWaypoint = NewWaypoint;
			}
		}
		CurrentWaypoint->NextWaypoint = EndWaypoint;
		EndWaypoint->PreviousWaypoint = CurrentWaypoint;
	}
}

void AGridMakerActor::Destroyed()
{
	DestroyGrid(nullptr);
	Super::Destroyed();
}