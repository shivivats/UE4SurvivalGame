// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Net/UnrealNetwork.h"
#include "Player/SurvivalCharacter.h"
#include "Components/InventoryComponent.h"

// localisation system for text is implemented by using the LOCTEXT_NAMESPACE macros

// can name the namespace anything
#define LOCTEXT_NAMESPACE "Item"

void UItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This function is where we setup all the replicated values
	// The server will manage all of these values, and the server will send all the values to the clients

	// this is just telling the Quantity value to be replicated
	DOREPLIFETIME(UItem, Quantity);
}

// This is how Unreal Engine checks if we want our UObjects to be networked, bc typically they are not networked
bool UItem::IsSupportedForNetworking() const
{
	return true;
}

#if WITH_EDITOR
void UItem::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	// if we try to change the quantity of the item in the editor
	// and the item is marked as not being stackable, we gotta lock the Quantity to 1
	// or max stack size, respectively
	// UPROPERTY clamping doesnt support this by default, so we do this here

	Super::PostEditChangeProperty(PropertyChangedEvent);

	// if changed property exists, get its name
	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// GET_MEMBER_NAME_CHECKED returns an FText after checking if the property exists for this class
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, Quantity))
	{
		Quantity = FMath::Clamp(Quantity, 1, bStackable? MaxStackSize : 1);
	}

}
#endif

UItem::UItem()
{
	// instead of using text directly, we use LOCTEXT and give it a key and default value
	// then we can go into Localisation Dashboard in UE4 and set translated values for this item key

	ItemDisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
	Weight = 0.f;
	bStackable = true;
	Quantity = 1;
	MaxStackSize = 2;
	RepKey = 0; 
}

void UItem::OnRep_Quantity()
{
	// just broadcast the delegate so we can let the editor know to update the UI to display the quantity
	OnItemModified.Broadcast();
}

void UItem::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, bStackable? MaxStackSize : 1);
		MarkDirtyForReplication();
	}
}

bool UItem::ShouldShowInInventory() const
{
	return true;
}

void UItem::Use(class ASurvivalCharacter* Character)
{

}

void UItem::AddedToInventory(class UInventoryComponent* Inventory)
{

}

void UItem::MarkDirtyForReplication()
{

}

#undef LOCTEXT_NAMESPACE