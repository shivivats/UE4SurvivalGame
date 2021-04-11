// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"

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
	DrawSize = FIntPoint(400, 100);
	bDrawAtDesiredSize = true;

	SetActive(true);
	// The reason we're hiding it in game is bc we dont want it to show up by default
	// It only shows up when player gets closer to it
	SetHiddenInGame(true);
}