// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SurvivalCharacter.generated.h"

USTRUCT()
struct FInteractionData
{
	// GENERATED_BODY is needed to be added to any struct
	GENERATED_BODY()

	// Make a constructor
	FInteractionData()
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}

	// The current interactable component we're viewing, if there is one
	UPROPERTY()
	class UInteractionComponent* ViewedInteractionComponent;

	// The time when we last checked for an interactable
	UPROPERTY()
	float LastInteractionCheckTime;

	// Whether the player is holding the interact key
	UPROPERTY()
	bool bInteractHeld;
};

UCLASS()
class SURVIVALGAME_API ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASurvivalCharacter();

	UPROPERTY(EditAnywhere, Category="Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, Category="Components")
	class USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(EditAnywhere, Category="Components")
	class USkeletalMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, Category="Components")
	class USkeletalMeshComponent* LegsMesh;

	UPROPERTY(EditAnywhere, Category="Components")
	class USkeletalMeshComponent* FeetMesh;

	UPROPERTY(EditAnywhere, Category="Components")
	class USkeletalMeshComponent* VestMesh;

	UPROPERTY(EditAnywhere, Category="Components")
	class USkeletalMeshComponent* HandsMesh;

	UPROPERTY(EditAnywhere, Category="Components")
	class USkeletalMeshComponent* BackpackMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// How often in seconds to check for an interactable object. Set this to zero if you want to check every tick
	// This is for optimisation
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckFrequency;

	// How far we'll trace when we check if the player is looking at an interactable object
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckDistance;

	void PerformInteractionCheck();

	void CouldntFindInteractable();
	void FoundNewInteractable(UInteractionComponent* Interactable);

	void BeginInteract();
	void EndInteract();

	// We make some Remote Procedure Calls (RPCs) for our functions

	// Reliable means it will always try to call the function even if connection is laggy
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndInteract();

	void Interact();

	// Information about the current state of the players interaction
	UPROPERTY()
	FInteractionData InteractionData;

	// Helper function to make grabbing interatable faster
	FORCEINLINE class UInteractionComponent* GetInteractable() const { return InteractionData.ViewedInteractionComponent;  }

	FTimerHandle TimerHandle_Interact;

	void StartCrouching();
	void StopCrouching();

	void MoveForward(float Val);
	void MoveRight(float Val);

	void LookUp(float Val);
	void Turn(float Val);


public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
