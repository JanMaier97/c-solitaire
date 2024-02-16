#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "raylib.h"

#define CARD_HEIGHT 120 
#define CARD_WIDTH 80
#define CARD_BORDER_WIDTH 2
#define CARD_STACK_OFFSET 30 
#define CARD_HALF_HEIGHT  CARD_HEIGHT/2
#define CARD_HALF_WIDTH  CARD_WIDTH/2
#define CARD_COUNT 52
#define CARD_STACK_COUNT 7
#define TARGET_STACK_COUNT 4

//#define PLATFORM_WEB

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

const int CARD_SPACING = 25;
const int MAX_CARD_VALUE = 13;

typedef enum CardType {
    HEAD_CARD,
    NORMAL_CARD
} CardType;

typedef enum CardSide {
    CS_NONE,
    CS_FRONT,
    CS_BACK
} CardSide;

typedef enum CardHighlight {
    HL_NONE,
    HL_DROP_OK,
    HL_DROP_INVALID,
} CardHighlight;

typedef struct Card {
    CardType type;
    Rectangle rect;
    int value;
    CardHighlight highlight;
    CardSide side;
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

void SuffleCards(Card cards[CARD_COUNT]);

bool CanDropCard(Card* targetCard, Card* cardToDrop)
{
    if (targetCard->value == cardToDrop->value+1)
    {
	return true;
    }

    return false;
}

Card cards[CARD_COUNT] = {0};
Card cardStacks[CARD_STACK_COUNT] = {0};
Card finalCardStacks[TARGET_STACK_COUNT] = {0}; 
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
	Color fillColor = MAROON;

	if (currentCard->side == CS_FRONT) 
	{
	    fillColor = WHITE;
	}

	// select fill color
	if (currentCard->type == HEAD_CARD)
	{
	    Color darkDarkGreen = (Color){ 0, 97, 24, 255 };
	    fillColor = darkDarkGreen;
	    borderColor = darkDarkGreen;
	}

	// select border color 
	switch (currentCard->highlight)
	{
	    case HL_DROP_OK:
		borderColor = BLUE;
		break;
	    case HL_DROP_INVALID:
		borderColor = RED;
		break;
	    case HL_NONE:
		borderColor = BLACK;
		break;
	}

	// draw border and inner rect
	DrawRectangleRec(currentRect, borderColor);
	DrawRectangle(
	    currentRect.x + CARD_BORDER_WIDTH,
	    currentRect.y + CARD_BORDER_WIDTH,
	    currentRect.width - CARD_BORDER_WIDTH*2,
	    currentRect.height - CARD_BORDER_WIDTH*2,
	    fillColor
	);

	// display value for normal cards 
	// and for front cards
	if (currentCard->type != HEAD_CARD &&
	    currentCard->side == CS_FRONT)
	{
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
	}

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

    if (currentCard == NULL)
    {
	return;
    }

    // assuming head cards can only exist at the head
    // manually set the postion of the first child and advance once
    if (currentCard->prev->type == HEAD_CARD)
    {
	currentCard->rect.y = currentCard->prev->rect.y;
	currentCard->rect.x = currentCard->prev->rect.x;
	currentCard = currentCard->next;
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
	Rectangle rect = (Rectangle){CARD_SPACING, CARD_SPACING, CARD_WIDTH, CARD_HEIGHT};
	int value = (i % MAX_CARD_VALUE) + 1;
	Card newCard = (Card){NORMAL_CARD, rect, value, HL_NONE, CS_BACK, NULL, NULL};
	cards[i] = newCard;
    }

    TraceLog(LOG_DEBUG, "Initialize card stacks");
    for (int i = 0; i < CARD_STACK_COUNT; i++)
    {
	int xOffset = CARD_SPACING;
	int yOffset = CARD_HEIGHT + CARD_SPACING * 2;
	Rectangle rect = (Rectangle){xOffset + (CARD_WIDTH + CARD_SPACING) * i, yOffset, CARD_WIDTH, CARD_HEIGHT};
	Card newCard = (Card){HEAD_CARD, rect, 0, false, CS_NONE, NULL, NULL};
	cardStacks[i] = newCard;
    }

    TraceLog(LOG_DEBUG, "Initialize target stacks");
    for (int i = 0; i < TARGET_STACK_COUNT; i++) 
    {
	int xOffset = (CARD_WIDTH + CARD_SPACING) * (CARD_STACK_COUNT - TARGET_STACK_COUNT) + CARD_SPACING;
	int yOffset = CARD_SPACING;
	Rectangle rect = (Rectangle){xOffset + (CARD_WIDTH + CARD_SPACING) * i, yOffset, CARD_WIDTH, CARD_HEIGHT};
	Card newCard = (Card){HEAD_CARD, rect, 0, false, CS_NONE, NULL, NULL};
	finalCardStacks[i] = newCard;
    }


    TraceLog(LOG_DEBUG, "Shuffle cards");
    SuffleCards(cards);

    TraceLog(LOG_DEBUG, "Append cards");
    int lastCardIdx = 0;
    for (int stackIdx = 0; stackIdx < CARD_STACK_COUNT; stackIdx++)
    {
	for (int i = 0; i < stackIdx+1; i++)
	{
	    Card* card = &cards[lastCardIdx]; 
	    AppendCard(&cardStacks[stackIdx],card); 
	    lastCardIdx += 1;
	}

	Card* lastCard = FindLast(&cardStacks[stackIdx]);
	assert(lastCard != NULL);
	lastCard->side = CS_FRONT;
    }

    for (int i = 0; i < CARD_STACK_COUNT; i++)
    {
	ResetCardPosition(&cardStacks[i]);
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
	    if (selectedCard->side == CS_BACK)
	    {
		TraceLog(LOG_DEBUG, "Selected card has not been exposed yet.");
		selectedCard = NULL;
	    }
	}
	else 
	{
	    TraceLog(LOG_DEBUG, "No card under cursor");
	}
    }

    // handle dropping of cards
    if (IsMouseButtonReleased(0)) {
	TraceLog(LOG_DEBUG, "Dropping selected card");

	if (dropTargetCard != NULL)
	{
	    Card* cardToExpose = selectedCard->prev;

	    AppendCard(dropTargetCard, selectedCard);
	    dropTargetCard = NULL;

	    if (cardToExpose != NULL)
	    {
		cardToExpose->side = CS_FRONT;
	    }

	}

	ResetCardPosition(selectedCard);

	for (int i=0; i < CARD_COUNT; i++)
	{
	    cards[i].highlight = HL_NONE;
	}

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
	    Card* topCard = FindLast(&cardStacks[i]);
	    if (topCard == selectedCard)
	    {
		continue;
	    }

	    if (CheckCollisionPointRec(mousePosition, topCard->rect)) 
	    {
		if (!CanDropCard(topCard, selectedCard)) 
		{
		    topCard->highlight = HL_DROP_INVALID;
		    break;
		}

		topCard->highlight = HL_DROP_OK;
		dropTargetCard = topCard;
	    }
	    else 
	    {
		topCard->highlight = HL_NONE;

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
    ClearBackground(DARKGREEN);

    // Draw all cards
    for (int i = 0; i < CARD_STACK_COUNT; i++)
    {
	DrawCards(&cardStacks[i]);
    }

    for (int i = 0; i < TARGET_STACK_COUNT; i++)
    {
	DrawCards(&finalCardStacks[i]);
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

void SuffleCards(Card cards[CARD_COUNT])
{
    // Fisher-Yates shuffle
    // note there is a raylib specific function `LoadRandomSequence`
    // it is not used due to a bug: https://github.com/raysan5/raylib/issues/3612

    srand(time(NULL));

    for (int i = CARD_COUNT-1; i > 0; i--)
    {
	// generate number in [0;i]
	int indexToSwap = rand() % (i+1);

	Card temp = cards[i];
	cards[i] = cards[indexToSwap];
	cards[indexToSwap] = temp;
    }
}
