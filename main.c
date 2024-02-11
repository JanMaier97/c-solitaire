#include <stdio.h>
#include <assert.h>
#include "raylib.h"

#define CARD_HEIGHT 120 
#define CARD_WIDTH 80
#define CARD_BORDER_WIDTH 2
#define CARD_STACK_OFFSET 30 
#define CARD_HALF_HEIGHT  CARD_HEIGHT/2
#define CARD_HALF_WIDTH  CARD_WIDTH/2
#define CARD_COUNT 5
#define CARD_STACK_COUNT 2

typedef struct Card {
    Rectangle rect;
    int value;
    struct Card* next;
} Card;

void DrawCards(Card* head) 
{
    if (head == NULL)
    {
	return;
    }

    Card* currentCard = head;
    while (currentCard != NULL) 
    {
	Rectangle currentRect = currentCard->rect;
	DrawRectangleRec(currentRect, BLACK);
	DrawRectangle(
	    currentRect.x + CARD_BORDER_WIDTH,
	    currentRect.y + CARD_BORDER_WIDTH,
	    currentRect.width - CARD_BORDER_WIDTH*2,
	    currentRect.height - CARD_BORDER_WIDTH*2,
	    WHITE);

	const char* valueText = TextFormat("%d", currentCard->value);
	int fontSize = 12;
	int yOffset = 12;
	int textWidth = MeasureText(valueText, fontSize);
	DrawText(
	    valueText,
	    currentRect.x + CARD_HALF_WIDTH - textWidth/2,
	    currentRect.y + yOffset,
	    fontSize,
	    BLACK
	);

	currentCard = currentCard->next;
    }
}

Card* FindLast(Card *card) 
{
    if (card == NULL)
    {
	return NULL;
    }

    Card* current = card;
    while (current->next != NULL) 
    {
	current = current->next;
    }

    return current;
}

void AppendCard(Card* card, Card* cardToAppend) 
{
    assert(card != NULL);
    assert(cardToAppend != NULL);

    Card* lastCard = FindLast(card);
    assert(lastCard != NULL);

    cardToAppend->rect.y = lastCard->rect.y + CARD_STACK_OFFSET;
    cardToAppend->rect.x = lastCard->rect.x;

    lastCard->next = cardToAppend;
}

void main(void)
{
    SetTraceLogLevel(LOG_DEBUG);
    Card* selectedCard = NULL;

    Card cards[CARD_COUNT] = {0};
    Card* cardStacks[CARD_STACK_COUNT] = {0};
    for (int i = 0; i < CARD_COUNT; i++) 
    {
	Rectangle rect = (Rectangle){100 + CARD_WIDTH*i + 50*i, 100, CARD_WIDTH, CARD_HEIGHT};
	Card newCard = (Card){rect,i};
	cards[i] = newCard;
    }

    printf("initialized cards");

    AppendCard(&cards[0], &cards[1]);
    AppendCard(&cards[0], &cards[2]);

    AppendCard(&cards[3], &cards[4]);

    cardStacks[0] = &cards[0];
    cardStacks[1] = &cards[3];

    InitWindow(800, 600, "Solitair");
    SetTargetFPS(60);
    while(!WindowShouldClose())
    {
	Vector2 mousePosition = GetMousePosition();
	if (IsMouseButtonPressed(0))
	{
	    for (int i = 0; i < CARD_COUNT; i++)
	    {
		if (CheckCollisionPointRec(mousePosition, cards[i].rect)) {
		    selectedCard = &cards[i];
		    TraceLog(LOG_DEBUG, "Selected card at index %d", i);
		}
	    }
	}
	
	if (IsMouseButtonReleased(0)) {
	    TraceLog(LOG_DEBUG, "Dropping selected card");
	    selectedCard = NULL;
	}

	if (selectedCard != NULL) 
	{
	    Vector2 delta = GetMouseDelta();
	    Card* currentCard = selectedCard;
	    while (currentCard != NULL)
	    {
		currentCard->rect.x += delta.x;
		currentCard->rect.y += delta.y;
		currentCard = currentCard->next;
	    }
	}

	BeginDrawing();
	ClearBackground(RED);
        for (int i = 0; i < CARD_STACK_COUNT; i++)
	{
	    DrawCards(cardStacks[i]);
	}

	EndDrawing();
    }

    CloseWindow();
}
