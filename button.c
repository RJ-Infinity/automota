#include "vec2d.h"
#include <stdint.h>
#include "olive.c"
#include "jsExtern.h"

//TODO: add a toggleable type
typedef struct {
	char* Text;
	Olivec_Font Font;
	size_t FontSize;
	uint32_t FontColour;
	uint32_t BackColour;
	uint32_t HoverColour;
	uint32_t FocusColour;
	size_t BorderWidth;
	uint32_t BorderColour;
	Vec2D Location;
	size_t Padding;
	void (*Callback)(void);
	bool needsRedrawing;
	bool _mouseOverBtn;
	int _mouseDownInBtn;
	Vec2D _lastPos;
} Button;

Button initButton(Button btn){
	if (btn.Text == NULL || btn.Callback == NULL){return (Button){0};}
	btn.needsRedrawing = true;
	btn._mouseOverBtn = false;
	btn._mouseDownInBtn = -1;
	return btn;
}

Vec2D getSizeOfBtn(Button* btn){
	return (Vec2D){
		.X=strlen(btn->Text)*btn->Font.width*btn->FontSize+btn->Padding*2+btn->BorderWidth*2,
		.Y=btn->Font.height*btn->FontSize+btn->Padding*2+btn->BorderWidth*2,
	};
}

bool isPointInBtn(Button* btn, Vec2D point){
	Vec2D size = getSizeOfBtn(btn);
	return point.X > btn->Location.X && point.Y > btn->Location.Y &&
		point.X < btn->Location.X + size.X && point.Y < btn->Location.Y + size.Y;
}

bool mouseMoveButton(Button* btn, MouseEvent* e)
{
	if (isPointInBtn(btn, e->loc)){
		if (!btn->_mouseOverBtn){
			btn->needsRedrawing = true;
			btn->_mouseOverBtn = true;
		}
	}else if (btn->_mouseOverBtn){
		btn->needsRedrawing = true;
		btn->_mouseOverBtn = false;
	}

	btn->_lastPos = e->loc;
	return btn->_mouseDownInBtn > -1;
}

bool mouseUpButton(Button* btn, MouseEvent* e){
	if (isPointInBtn(btn, e->loc) && btn->_mouseDownInBtn > -1){btn->Callback();}
	btn->_mouseDownInBtn = -1;
	btn->needsRedrawing = true;
	return false;
}

bool mouseDownButton(Button* btn, MouseEvent* e){
	bool inBtn = isPointInBtn(btn, e->loc);
	if(inBtn){
		if (btn->_mouseDownInBtn == -1){btn->needsRedrawing = true;}
		btn->_mouseDownInBtn = e->button;
	}
	return inBtn;
}

void drawButton(Button* btn, Olivec_Canvas* oc){
	btn->needsRedrawing = false;
	Vec2D size = getSizeOfBtn(btn);
	olivec_rect(
		*oc,
		btn->Location.X,
		btn->Location.Y,
		size.X,
		size.Y,
		btn->BorderColour
	);
	uint32_t colour = btn->BackColour;
	if (btn->_mouseOverBtn){
		colour = btn->HoverColour;
		if (btn->_mouseDownInBtn > -1){colour = btn->FocusColour;}
	}
	olivec_rect(
		*oc,
		btn->Location.X+btn->BorderWidth,
		btn->Location.Y+btn->BorderWidth,
		size.X-2*btn->BorderWidth,
		size.Y-2*btn->BorderWidth,
		colour
	);
	olivec_text(
		*oc,
		btn->Text,
		btn->Location.X+btn->BorderWidth+btn->Padding,
		btn->Location.Y+btn->BorderWidth+btn->Padding,
		btn->Font,
		btn->FontSize,
		btn->FontColour
	);
}
