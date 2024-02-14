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

//#define PLATFORM_WEB

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

typedef struct Card {
    Rectangle rect;
    int value;
    bool isHighlighted;
    struct Card* prev;
    struct Card* next;
} Card;

void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void UpdateDrawFrame(void);

Card* GetSelectedCard(Card cards[CARD_COUNT]);
Card* FindLast(Card *card);
void AppendCard(Card* card, Card* cardToAppend);
void ResetCardPosition(Card* card);
void DrawCards(Card* head);

Card cards[CARD_COUNT] = {0};
Card* cardStacks[CARD_STACK_COUNT] = {0};
Card* selectedCard = NULL;
Card* dropTargetCard = NULL;

int main(void)
{
    SetTraceLogLevel(LOG_DEBUG);


    InitWindow(800, 600, "Solitaire");
    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);

    while(!WindowShouldClose())
    {
	UpdateDrawFrame();
    }

#endif
    CloseWindow();

    return 0;
}

Card* GetSelectedCard(Card cards[CARD_COUNT]) 
{
    // find any card that is under the cursor
    Vector2 mousePos = GetMousePosition();
    Card* firstCollision = NULL;
    for (int i = 0; i < CARD_COUNT; i++)
    {
	if (CheckCollisionPointRec(mousePos, cards[i].rect))
	{
	    firstCollision = &cards[i];
	    break;
	}
    }

    if (firstCollision == NULL)
    {
	return NULL;
    }

    // find the top most card under the cursor
    Card* currentCard = firstCollision;
    while (true)
    {
	if (currentCard->next == NULL)
	{
	    return currentCard;
	}

	if (CheckCollisionPointRec(mousePos, currentCard->next->rect)) 
	{
	    currentCard = currentCard->next;
	}
	else 
	{
	    return currentCard;
	}
    }
}

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
	Color borderColor = BLACK;
	if (currentCard->isHighlighted)
	{
	    borderColor = BLUE;
	}

	DrawRectangleRec(currentRect, borderColor);
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

    lastCard->next = cardToAppend;

    // Delte child reference of parent 
    if (cardToAppend->prev != NULL)
    {
	cardToAppend->prev->next = NULL;
    }

    cardToAppend->prev = lastCard;

    // LogTrace(LOG_DEBUG, TextFormat("Chidl value: %d"));
}

// Resets the cards position to the parent card
void ResetCardPosition(Card* card)
{
    if (card == NULL)
    {
	return;
    }

    TraceLog(LOG_DEBUG, "Resetting card position");
    Card* currentCard = card;

    // if given card has no parent, use it as the parent instead
    if (currentCard->prev == NULL) 
    {
	TraceLog(LOG_DEBUG, "Using current card as parent");
	currentCard = card->next;
    }

    while(currentCard != NULL)
    {
	TraceLog(LOG_DEBUG, "Setting position for card %d", currentCard->value);
	currentCard->rect.y = currentCard->prev->rect.y + CARD_STACK_OFFSET;
	currentCard->rect.x = currentCard->prev->rect.x;

	currentCard = currentCard->next;
    }
}

void InitGame(void)
{
    TraceLog(LOG_DEBUG, "Initialize cards");
    for (int i = 0; i < CARD_COUNT; i++) 
    {
	Rectangle rect = (Rectangle){100 + CARD_WIDTH*i + 50*i, 100, CARD_WIDTH, CARD_HEIGHT};
	Card newCard = (Card){rect, i, false, NULL, NULL};
	cards[i] = newCard;
    }

    TraceLog(LOG_DEBUG, "Initialize card stacks");

    AppendCard(&cards[0], &cards[1]);
    AppendCard(&cards[0], &cards[2]);
    AppendCard(&cards[3], &cards[4]);

    cardStacks[0] = &cards[0];
    cardStacks[1] = &cards[3];

    ResetCardPosition(cardStacks[0]);
    ResetCardPosition(cardStacks[1]);

    for (int i = 0; i < CARD_COUNT; i++)
    {
	TraceLog(LOG_DEBUG, "Card %d position: (%f, %f)", cards[i].value, cards[i].rect.x, cards[i].rect.y);
    }
}

void UpdateGame(void)
{
    Vector2 mousePosition = GetMousePosition();
    if (IsMouseButtonPressed(0))
    {
	TraceLog(LOG_DEBUG, "Mouse Position: (%f, %f)", mousePosition.x, mousePosition.y);
	selectedCard = GetSelectedCard(cards);

	if (selectedCard != NULL)
	{
	    TraceLog(LOG_DEBUG, "Selected card %d", selectedCard->value);
	}
	else 
	{
	    TraceLog(LOG_DEBUG, "No card under cursor");
	}
    }

    if (IsMouseButtonReleased(0)) {
	TraceLog(LOG_DEBUG, "Dropping selected card");

	if (dropTargetCard != NULL)
	{
	    AppendCard(dropTargetCard, selectedCard);
	    dropTargetCard->isHighlighted = false;
	    dropTargetCard = NULL;
	}
	ResetCardPosition(selectedCard);

	selectedCard = NULL;
    }

    if (selectedCard != NULL) 
    {
	// move card with mouse
	Vector2 delta = GetMouseDelta();
	Card* currentCard = selectedCard;
	while (currentCard != NULL)
	{
	    currentCard->rect.x += delta.x;
	    currentCard->rect.y += delta.y;
	    currentCard = currentCard->next;
	}

	// detect collisions with cards
	for (int i = 0; i < CARD_STACK_COUNT; i++)
	{
	    Card* topCard = FindLast(cardStacks[i]);
	    if (topCard == selectedCard)
	    {
		continue;
	    }

	    if (CheckCollisionPointRec(mousePosition, topCard->rect)) 
	    {
		topCard->isHighlighted = true;
		dropTargetCard = topCard;
	    }
	    else 
	    {
		topCard->isHighlighted = false;

		// reset card for drag and drop 
		if (topCard == dropTargetCard)
		{
		    dropTargetCard = NULL;
		}
	    }
	}
    }
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(RED);

    // Draw all cards
    for (int i = 0; i < CARD_STACK_COUNT; i++)
    {
	DrawCards(cardStacks[i]);
    }

    // Draw selected card stack above all else
    DrawCards(selectedCard);

    EndDrawing();
}

void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}
