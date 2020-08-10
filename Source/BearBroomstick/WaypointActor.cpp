#include "WaypointActor.h"
#include "Components/ArrowComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AWaypointActor::AWaypointActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>("Scene");
	Scene->Mobility = EComponentMobility::Static;
	RootComponent = Scene;

	static ConstructorHelpers::FObjectFinder<UTexture2D> TargetIconObj(TEXT("/Engine/EditorMaterials/TargetIcon.TargetIcon"));
	if(TargetIconObj.Succeeded())
	{
		Billboard = CreateDefaultSubobject<UBillboardComponent>("Billboard");
		Billboard->SetUsingAbsoluteScale(true);
		Billboard->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.35f));
		Billboard->Mobility = EComponentMobility::Static;
		Billboard->bIsScreenSizeScaled = true;
		Billboard->Sprite = TargetIconObj.Object;
		Billboard->bIsEditorOnly = true;
		Billboard->SetupAttachment(RootComponent);

		Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
		Arrow->SetRelativeScale3D(FVector(2.857143f, 2.857143f, 2.857143f));
		Arrow->ArrowColor = FColor(150, 200, 255, 255);
		Arrow->ArrowSize = 0.5f;
		Arrow->bIsScreenSizeScaled = true;
		Arrow->bTreatAsASprite = true;
		Arrow->SetupAttachment(Billboard);
	}
}