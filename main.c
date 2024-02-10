#include <stdio.h>
#include "raylib.h"

#define CARD_HEIGHT 120 
#define CARD_WIDTH 80
#define CARD_HALF_HEIGHT  CARD_HEIGHT/2
#define CARD_HALF_WIDTH  CARD_WIDTH/2
#define CARD_COUNT 2

typedef struct Card {
    Rectangle rect;
} Card;

void main(void)
{
    InitWindow(800, 600, "test");
    SetTargetFPS(60);

    Card cards[CARD_COUNT] = {0};
    for (int i = 0; i < CARD_COUNT; i++) 
    {
	Card newCard = (Card){(Rectangle){100 + CARD_WIDTH*i + 50*i, 100, CARD_WIDTH, CARD_HEIGHT}};
	cards[i] = newCard;
    }

    Card* selectedCard = NULL;

    while(!WindowShouldClose())
    {
	Vector2 mousePosition = GetMousePosition();
	if (IsMouseButtonPressed(0))
	{
	    for (int i = 0; i < CARD_COUNT; i++)
	    {
		if (CheckCollisionPointRec(mousePosition, cards[i].rect)) {
		    selectedCard = &cards[i];
		}
	    }
	}
	
	if (IsMouseButtonReleased(0)) {
	    selectedCard = NULL;
	}

	if (selectedCard != NULL) 
	{
	    selectedCard->rect.x = mousePosition.x - CARD_HALF_WIDTH;
	    selectedCard->rect.y = mousePosition.y - CARD_HALF_HEIGHT;
	}

	BeginDrawing();
	ClearBackground(RED);

        for (int i = 0; i < CARD_COUNT; i++)
	{
	    DrawRectangleRec(cards[i].rect, WHITE);
	}

	EndDrawing();
    }

    CloseWindow();
}
