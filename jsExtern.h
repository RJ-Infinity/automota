#pragma once
#include "olive.c"

#include "vec2d.h"

float render(Olivec_Canvas oc);
void pushToBuffer(char* x);
void pushToBufferi(int x);
void pushToBufferb(bool x);
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
size_t strlen(const char *str);
void runAfterTime(void(*callback)(void), int timeMS);
int getUnixTimeStamp(void);

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
