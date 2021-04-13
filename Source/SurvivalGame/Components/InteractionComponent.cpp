// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "Player/SurvivalCharacter.h"
#include "Widgets/InteractionWidget.h"

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

void UInteractionComponent::SetInteractableNameText(const FText& NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}

void UInteractionComponent::SetInteractableActionText(const FText& NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
}

void UInteractionComponent::Deactivate()
{
	Super::Deactivate();

	// loop over all the interactors for this object
	for (int32 i = Interactors.Num() - 1; i >= 0; --i)
	{
		if (ASurvivalCharacter* Interactor = Interactors[i])
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}

	// clear all the interactors out
	Interactors.Empty();
}

bool UInteractionComponent::CanInteract(class ASurvivalCharacter* Character) const
{
	const bool bPlayerAlreadyInteracting = !bAllowMultipleInteractors && Interactors.Num() >= 1;

	// check some conditions for if we can interact
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && Character != nullptr;
}

void UInteractionComponent::RefreshWidget()
{
	if (!bHiddenInGame && GetOwner()->GetNetMode() != NM_DedicatedServer)
	{
		// Make sure the widget is initialised and that we are displaying the right values (these may have changed)
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
		{
			InteractionWidget->UpdateInteractionWidget(this);
		}
	}
}

void UInteractionComponent::BeginFocus(class ASurvivalCharacter* Character)
{
	if (!IsActive() || !GetOwner() || !Character)
	{
		return;
	}

	// call the delegate
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

	RefreshWidget();
}

void UInteractionComponent::EndFocus(class ASurvivalCharacter* Character)
{
	// call the delegate
	OnEndFocus.Broadcast(Character);

	// hide this interactable in game
	SetHiddenInGame(true);

	// if not the server
	if (!GetOwner()->HasAuthority())
	{
		// grab any visual components (they are called primitive components in ue4)
		for (auto& VisualComp : GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass()))
		{
			// this disables the outline around the visual object
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{
				Prim->SetRenderCustomDepth(false);
			}
		}
	}
}

void UInteractionComponent::BeginInteract(class ASurvivalCharacter* Character)
{
	if (CanInteract(Character))
	{
		Interactors.AddUnique(Character);
		OnBeginInteract.Broadcast(Character);
	}
}

void UInteractionComponent::EndInteract(class ASurvivalCharacter* Character)
{
	Interactors.RemoveSingle(Character);
	OnEndInteract.Broadcast(Character);
}

void UInteractionComponent::Interact(class ASurvivalCharacter* Character)
{
	if (CanInteract(Character))
	{
		// call the delegate
		OnInteract.Broadcast(Character);
	}

}

float UInteractionComponent::GetInteractPercentage()
{
	// the first interactor is us always on our local machine
	if (Interactors.IsValidIndex(0))
	{
		if (ASurvivalCharacter* Interactor = Interactors[0])
		{
			if (Interactor && Interactor->IsInteracting())
			{
				return 1.f - FMath::Abs(Interactor->GetRemainingInteractTime() / InteractionTime);
			}
		}
	}

	return 0.f;
}
