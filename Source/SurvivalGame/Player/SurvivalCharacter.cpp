// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InteractionComponent.h"
#include "TimerManager.h"

// Sets default values
ASurvivalCharacter::ASurvivalCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(GetMesh(), FName("CameraSocket"));
	CameraComponent->bUsePawnControlRotation = true;

	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HelmetMesh");
	HelmetMesh->SetupAttachment(GetMesh());
	HelmetMesh->SetMasterPoseComponent(GetMesh());

	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ChestMesh");
	ChestMesh->SetupAttachment(GetMesh());
	ChestMesh->SetMasterPoseComponent(GetMesh());

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("LegsMesh");
	LegsMesh->SetupAttachment(GetMesh());
	LegsMesh->SetMasterPoseComponent(GetMesh());

	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>("FeetMesh");
	FeetMesh->SetupAttachment(GetMesh());
	FeetMesh->SetMasterPoseComponent(GetMesh());

	VestMesh = CreateDefaultSubobject<USkeletalMeshComponent>("VestMesh");
	VestMesh->SetupAttachment(GetMesh());
	VestMesh->SetMasterPoseComponent(GetMesh());

	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HandsMesh");
	HandsMesh->SetupAttachment(GetMesh());
	HandsMesh->SetMasterPoseComponent(GetMesh());

	BackpackMesh = CreateDefaultSubobject<USkeletalMeshComponent>("BackpackMesh");
	BackpackMesh->SetupAttachment(GetMesh());
	BackpackMesh->SetMasterPoseComponent(GetMesh());

	GetMesh()->SetOwnerNoSee(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	InteractionCheckFrequency = 0.f;
	InteractionCheckDistance = 1000.f; // in cms, so this is 10 meters max distance
}

// Called when the game starts or when spawned
void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ASurvivalCharacter::IsInteracting() const
{
	// check if the timer is active
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float ASurvivalCharacter::GetRemainingInteractTime() const
{
	// simply return the time remaining
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void ASurvivalCharacter::StartCrouching()
{
	Crouch();
}

void ASurvivalCharacter::StopCrouching()
{
	UnCrouch();
}

void ASurvivalCharacter::MoveForward(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(GetActorForwardVector(), Val);
	}

}


void ASurvivalCharacter::MoveRight(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(GetActorRightVector(), Val);
	}
}

void ASurvivalCharacter::LookUp(float Val)
{
	if (Val != 0.f)
	{
		AddControllerPitchInput(Val);
	}
}

void ASurvivalCharacter::Turn(float Val)
{
	if (Val != 0.f)
	{
		AddControllerYawInput(Val);
	}
}

// Called every frame
void ASurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bIsInteractingOnServer = (HasAuthority() && IsInteracting());

	//UE_LOG(LogTemp, Warning, TEXT("time since last check is %f"), GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime));
	//UE_LOG(LogTemp, Warning, TEXT("interaction check frequency is %f"), InteractionCheckFrequency);

	//UE_LOG(LogTemp, Warning, TEXT("bIsInteractingOnServer is %s"), (bIsInteractingOnServer ? TEXT("true") : TEXT("false")));
	//UE_LOG(LogTemp, Warning, TEXT("HasAuthority is %s"), (HasAuthority() ? TEXT("true") : TEXT("false")));
	//UE_LOG(LogTemp, Warning, TEXT("interaction check frequency is %s"), ());

	// if we're not the server or we're interacting on the server, and only do it according to the interaction frequency
	if ((!HasAuthority() || bIsInteractingOnServer) && (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) >= InteractionCheckFrequency))
	{
		UE_LOG(LogTemp, Warning, TEXT("performing interaction check"));
		PerformInteractionCheck();
	}
}

// Main function that is going to handle if we're looking at an interactable object or not
void ASurvivalCharacter::PerformInteractionCheck()
{

	// safety check incase there is no controller
	if (GetController() == nullptr)
	{
		return;
	}

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyesLoc;
	FRotator EyesRot;

	// We get the location and rotation of the player's camera
	GetController()->GetPlayerViewPoint(EyesLoc, EyesRot);

	// We need points for where the raycast will start and end
	FVector TraceStart = EyesLoc;
	FVector TraceEnd = (EyesRot.Vector() * InteractionCheckDistance) + TraceStart;
	FHitResult TraceHit;

	// Making sure the raycast doesnt hit the player themselves
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(true);

	// Run a line trace where we are looking
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		// Check if we hit something
		if (TraceHit.GetActor())
		{
			UE_LOG(LogTemp, Warning, TEXT("saw an object"));
			// Check if the thing we hit is an interactable
			if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass())))
			{
				UE_LOG(LogTemp, Warning, TEXT("saw an interactable object"));

				// Get how far away we are from the object
				float Distance = (TraceStart - TraceHit.ImpactPoint).Size();

				// we found interactable and we are within distance
				if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance)
				{
					
					FoundNewInteractable(InteractionComponent);
				}
				// we found interactable but we are not within distance
				else if (GetInteractable() && Distance > InteractionComponent->InteractionDistance)
				{
					CouldntFindInteractable();
				}

				return;
			}
		}
	}

	// if we get here then we either didnt hit an actor or we didnt hit an interactable
	CouldntFindInteractable();
}

void ASurvivalCharacter::CouldntFindInteractable()
{
	// We've lost the interactable. clear the timer
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}

	// Tell the interactable we've stopped focusing on it and clear the current interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
		{
			EndInteract();
		}
	}

	// arent viewing any interactable anymore
	InteractionData.ViewedInteractionComponent = nullptr;
}

void ASurvivalCharacter::FoundNewInteractable(UInteractionComponent* Interactable)
{
	// when we find a new interactable, first stop interacting with any current interactable
	EndInteract();

	// remove focus from the old interactable if any
	if (UInteractionComponent* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(this);
	}

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);
}

void ASurvivalCharacter::BeginInteract()
{
	// if we're not the server
	if (!HasAuthority())
	{
		// we wanna call begin interact on the server
		ServerBeginInteract();
	}

	InteractionData.bInteractHeld = true;

	// if it has an interactable component
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		UE_LOG(LogTemp, Warning, TEXT("try to interact with an object"));

		Interactable->BeginInteract(this);

		// if interaction time is smol
		if (FMath::IsNearlyZero(Interactable->InteractionTime))
		{
			Interact();
		}
		else
		{
			// now we gotta wait to interact. set a timer to call interact function after the interaction time has elapsed
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ASurvivalCharacter::Interact, Interactable->InteractionTime, false);
		}
	}
}

void ASurvivalCharacter::EndInteract()
{
	// if we're not the server
	if (!HasAuthority())
	{
		// we wanna call end interact on the server
		ServerEndInteract();
	}

	InteractionData.bInteractHeld = false;

	// clear the interaction duration timer if the timer was running
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndInteract(this);
	}
}

void ASurvivalCharacter::ServerBeginInteract_Implementation()
{
	// we just call the begin interact function on the server
	BeginInteract();
}

bool ASurvivalCharacter::ServerBeginInteract_Validate()
{
	// go ahead with the RPC, assume everything will work properly
	return true;
}

void ASurvivalCharacter::ServerEndInteract_Implementation()
{
	// we just call the end interact function on the server
	EndInteract();
}

bool ASurvivalCharacter::ServerEndInteract_Validate()
{
	// go ahead with the RPC, assume everything will work properly
	return true;
}

void ASurvivalCharacter::Interact()
{
	// clear the interaction duration timer if the timer was running
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	// we call the interactable's interact function
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		UE_LOG(LogTemp, Warning, TEXT("INTERACTED WITH AN OBJECT"));
		Interactable->Interact(this);
	}
}

// Called to bind functionality to input
void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &ASurvivalCharacter::EndInteract);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASurvivalCharacter::StopCrouching);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASurvivalCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASurvivalCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ASurvivalCharacter::Turn);

}

