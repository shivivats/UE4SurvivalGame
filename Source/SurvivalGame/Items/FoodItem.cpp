// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodItem.h"
#include "Player/SurvivalCharacter.h"

#define LOCTEXT_NAMESPACE "FoodItem"

UFoodItem::UFoodItem()
{
	HealAmount = 20.f;
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");
}

void UFoodItem::Use(class ASurvivalCharacter* Character)
{
	// Here we can heal the character
}

#undef LOCTEXT_NAMESPACE