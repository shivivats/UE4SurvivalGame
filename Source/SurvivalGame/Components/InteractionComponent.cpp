// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "Player/SurvivalCharacter.h"

UInteractionComponent::UInteractionComponent()
{
	SetComponentTickEnabled(false);

	InteractionTime = 0.f;
	InteractionDistance = 200.f; // Distance is in cms
	InteractableNameText = FText::FromString("Interactable Object");
	InteractableActionText = FText::FromString("Interact");
	bAllowMultipleInteractors = true;

	// Draw widget in worldspace or screen space? We draw in screen space
	Space = EWidgetSpace::Screen; 
	// How big the UI is going to be on the screen
	DrawSize = FIntPoint(600, 100);
	bDrawAtDesiredSize = true;

	SetActive(true);
	// The reason we're hiding it in game is bc we dont want it to show up by default
	// It only shows up when player gets closer to it
	SetHiddenInGame(true);
}

void UInteractionComponent::BeginFocus(class ASurvivalCharacter* Character)
{
	if (!IsActive() || !GetOwner() || !Character)
	{
		return;
	}

	OnBeginFocus.Broadcast(Character);

	// show this interactable in game
	SetHiddenInGame(false);

	// if not the server
	if (!GetOwner()->HasAuthority())
	{
		// grab any visual components (they are called primitive components in ue4)
		for (auto& VisualComp : GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass()))
		{
			// this enables the outline around the visual object
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(true);
			}
		}
	}

}

void UInteractionComponent::EndFocus(class ASurvivalCharacter* Character)
{

}

void UInteractionComponent::BeginInteract(class ASurvivalCharacter* Character)
{

}

void UInteractionComponent::EndInteract(class ASurvivalCharacter* Character)
{

}

void UInteractionComponent::Interact(class ASurvivalCharacter* Character)
{
	OnInteract.Broadcast(Character);
}
