#pragma once
#define OLIVEC_IMPLEMENTATION
#include "olive.c"

#include "common.h"

float render(Olivec_Canvas oc);
void pushToBuffer(char* x);
void pushToBufferi(int x);
void logBuffer(void);
void clearBuffer(void);
char* getBuffer(void);
void consoleLog(char* x);
void consoleLogi(int x);
void consoleLogb(bool x);
void* malloc(size_t size);
void free(void* ptr);
char* itoa(int x);
int strcmp(char* a, char* b);
void debug(int a);

// although not technicly a js function it is needed for memcpy to work
void* memcpy(void* dest, void* src, size_t count)
{return __builtin_memcpy(dest, src, count);}

typedef struct{
	Vec2D loc;
	int button;
	bool altKey;
	bool ctrlKey;
	bool shiftKey;
} MouseEvent;

typedef struct{
	char* key;
	int keycode;
	bool altKey;
	bool ctrlKey;
	bool shiftKey;
} KeyboardEvent;
