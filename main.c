#define OLIVEC_IMPLEMENTATION
#include "olive.c"
#undef OLIVEC_IMPLEMENTATION
#include "jsExtern.h"
#include "vec2d.h"
#include "button.c"
// #include "font.h"
#include "olivec_rose_font.c"


#define CELL_AREA_WIDTH 32
typedef struct{
	Vec2D Location;
	int Area[CELL_AREA_WIDTH*CELL_AREA_WIDTH];
	size_t _nonDeadCells;
}CellArea;

#define GetAreaIndex(loc) (((loc.Y % CELL_AREA_WIDTH) * CELL_AREA_WIDTH) + loc.X % CELL_AREA_WIDTH)
Vec2D getAreaLocation(Vec2D absolute){
	return (Vec2D){
		.X=absolute.X/CELL_AREA_WIDTH,
		.Y=absolute.Y/CELL_AREA_WIDTH,
	};
}
Vec2D getAbsolute(Vec2D area, size_t offset){
	return (Vec2D){
		.X=area.X*CELL_AREA_WIDTH+offset%CELL_AREA_WIDTH,
		.Y=area.Y*CELL_AREA_WIDTH+offset/CELL_AREA_WIDTH,
	};
}

bool CellLocCmp(CellArea a, CellArea b){
	return a.Location.X == b.Location.X &&
		a.Location.Y == b.Location.Y;
}
#define MaxCellType 3

uint32_t cellColours[MaxCellType]= {
	0xffff0000,0xffff0ff0,0xff0f0fff
};

#define TYPE CellArea
#include "RJList.h"
#undef TYPE
#define TYPE Vec2D
#include "RJList.h"
#undef TYPE

RJList_CellArea Cells = {0};

Vec2D OffsetLocation={0};

#define BACKGROUND_COLOR 0xFF181818

static uint32_t* Pixels;
Olivec_Canvas oc;
int Width;
int Height;

int mouseState = -1;
int cellPlacingType;

Vec2D MouseLastLoc = {0};

Button autoRunBtn;
bool autoRun = false;

void draw(void)
{
	olivec_fill(oc, BACKGROUND_COLOR);

	for (size_t i = 0; i < Cells.Length; i++){
		// if (Cells.Buffer[i].Location.X == -1 && Cells.Buffer[i].Location.Y == -1){continue;}
		// olivec_frame(
		// 	oc,
		// 	Cells.Buffer[i].Location.X*320+OffsetLocation.X,
		// 	Cells.Buffer[i].Location.Y*320+OffsetLocation.Y,
		// 	320,320,2,0xff0000ff
		// );
		pushToBuffer("(");
		pushToBufferi(Cells.Buffer[i].Location.X);
		pushToBuffer(", ");
		pushToBufferi(Cells.Buffer[i].Location.Y);
		pushToBuffer(")");
		char* buffer = getBuffer();
		clearBuffer();
		olivec_text(
			oc,
			buffer,
			Cells.Buffer[i].Location.X*320+OffsetLocation.X,
			Cells.Buffer[i].Location.Y*320+OffsetLocation.Y,
			olivec_rose_font,
			2,
			0xff0ff0ff
		);
		free(buffer);
		for (size_t j=0; j < CELL_AREA_WIDTH*CELL_AREA_WIDTH; j++){
			// olivec_text(
			// 	oc,
			// 	itoa(neighbours(&Cells.Buffer[i], j)[0]),
			// 	Cells.Buffer[i].Location.X*320,
			// 	Cells.Buffer[i].Location.Y*320,
			// 	MainFont,
			// 	8,
			// 	0xff0ff0ff
			// );
			if (Cells.Buffer[i].Area[j] == 0){continue;} // if its dead dont draw anything
			olivec_rect(
				oc,
				(Cells.Buffer[i].Location.X*320 + j % CELL_AREA_WIDTH * 10)+OffsetLocation.X,
				(Cells.Buffer[i].Location.Y*320 + j / CELL_AREA_WIDTH * 10)+OffsetLocation.Y,
				10,
				10,
				cellColours[Cells.Buffer[i].Area[j]-1]
			);
		}
	}

	drawButton(&autoRunBtn, &oc);

	render(oc);
}

void addCellInArea(RJList_CellArea* cells, size_t index, Vec2D loc, int type){
	int AreaIndex = GetAreaIndex(loc);
	// its already the type so nothing to do
	if (cells->Buffer[index].Area[AreaIndex] == type) {return;}
	
	if (type == 0) {
		if (cells->Buffer[index]._nonDeadCells == 1){
			RJListRemove_CellArea(cells,index);
			return;
		}
		cells->Buffer[index]._nonDeadCells--;
	}else if(cells->Buffer[index].Area[AreaIndex] == 0)
	// we are replacing a dead cell so increment
	{cells->Buffer[index]._nonDeadCells++;}
	// else we are repaceing a non dead cell so nothing else changes

	cells->Buffer[index].Area[AreaIndex] = type;
}
void addCell(RJList_CellArea* cells, Vec2D loc, int type){
	CellArea cell = {0};
	cell.Location.X = loc.X / CELL_AREA_WIDTH;
	cell.Location.Y = loc.Y / CELL_AREA_WIDTH;
	size_t index = RJListFind_CellArea(cells, cell, CellLocCmp);
	// we found a cellarea
	if (index!=(size_t)-1) {addCellInArea(cells, index, loc, type);}
	else if (type != 0){// we cant find a cellarea and we arent trying to clear the cell
		cell._nonDeadCells = 1;
		cell.Area[GetAreaIndex(loc)] = type;
		RJListAppend_CellArea(cells,cell);
	}
}
int toggleCell(RJList_CellArea* cells, Vec2D loc){
	CellArea cell = {0};
	cell.Location.X = loc.X / CELL_AREA_WIDTH;
	cell.Location.Y = loc.Y / CELL_AREA_WIDTH;
	int AreaIndex = GetAreaIndex(loc);
	size_t index = RJListFind_CellArea(cells, cell, CellLocCmp);
	if (index==(size_t)-1) {
		cell._nonDeadCells = 1;
		cell.Area[AreaIndex] = 1;
		RJListAppend_CellArea(cells,cell);
		return 1;
	}

	int type = cells->Buffer[index].Area[AreaIndex];
	if (type >= MaxCellType) {type=0;}else{type++;}

	addCellInArea(cells, index,loc,type);

	return type;
}

size_t* neighbours(CellArea* area, size_t index){
	size_t* array = malloc(sizeof(size_t) * MaxCellType);
	for (size_t i = 0; i < MaxCellType; i++){array[i] = 0;}
	for (int offsetX = -1; offsetX < 2; offsetX++)
	{for (int offsetY = -1; offsetY < 2; offsetY++){
		if (offsetX==0 && offsetY==0){continue;}

		Vec2D AbsoluteLoc = getAbsolute(area->Location, index);
		AbsoluteLoc.X += offsetX;
		AbsoluteLoc.Y += offsetY;

		Vec2D AreaLoc = getAreaLocation(AbsoluteLoc);

		if (
			area->Location.X != AreaLoc.X ||
			area->Location.Y != AreaLoc.Y
		){
			CellArea cell = {0};
			cell.Location.X = AreaLoc.X;
			cell.Location.Y = AreaLoc.Y;
			size_t areaIndex = RJListFind_CellArea(&Cells, cell, CellLocCmp);
			if (areaIndex != (size_t)-1){
				int value = Cells.Buffer[areaIndex].Area[GetAreaIndex(AbsoluteLoc)];
				if (value > 0){array[value-1] = array[value-1] + 1;}
			}
		}else{
			int value = area->Area[GetAreaIndex(AbsoluteLoc)];
			if (value > 0){array[value-1] = array[value-1] + 1;}
		}
	}}
	return array;
}
int gol(int currState, size_t* nbrs){
	if (currState == 1){
		if (nbrs[0] == 2 || nbrs[0] == 3){return 1;}
	}else if(nbrs[0] ==  3) {return 1;}
	return 0;
}
int seeds(int currState, size_t* nbrs){
	if (currState == 0 && nbrs[0] == 2){return 1;}
	return 0;
}
int briansBrain(int currState, size_t* nbrs){
	#define OFF 0
	#define DYING 1
	#define ON 2

	if (currState == ON){return DYING;}
	if (currState == OFF && nbrs[ON-1] == 2){return ON;}
	return OFF;
}
int WireWorld(int currState, size_t* nbrs){
	#define OFF 0
	#define ELECTRONHEAD 1
	#define ELECTRONTAIL 2
	#define CONDUCTOR 3

	if (currState == ELECTRONHEAD){return ELECTRONTAIL;}
	if (currState == ELECTRONTAIL){return CONDUCTOR;}
	if (currState == CONDUCTOR && (
		nbrs[ELECTRONHEAD-1] == 1 ||
		nbrs[ELECTRONHEAD-1] == 2
	)){return ELECTRONHEAD;}
	if (currState == CONDUCTOR){return CONDUCTOR;}
	return OFF;
}
void nextItteration(void){
	// as the next itteration will have a similar amount of cells to the current
	// itteration it is better to copy the list and empty it than to perform lots
	// of mallocs on a new list
	RJList_CellArea newCells = RJListCopy_CellArea(&Cells);
	newCells.Length = 0;

	RJList_Vec2D done = {0};
	size_t cellCount = 0;
	for (size_t iCellArea = 0; iCellArea < Cells.Length; iCellArea++)
	{for (size_t iCellIndex=0; iCellIndex < CELL_AREA_WIDTH*CELL_AREA_WIDTH; iCellIndex++)
	{if (Cells.Buffer[iCellArea].Area[iCellIndex] != 0)
	{cellCount++;for (int offsetY = -1; offsetY < 2; offsetY++)
	{for (int offsetX = -1; offsetX < 2; offsetX++){
		Vec2D loc = getAbsolute(Cells.Buffer[iCellArea].Location,iCellIndex);
		loc.X+=offsetX;
		loc.Y+=offsetY;

		if (RJListFind_Vec2D(&done, loc, Vec2DComp) != (size_t)-1){continue;}

		Vec2D AreaLoc = getAreaLocation(loc);

		CellArea* cellArea = &Cells.Buffer[iCellArea];
		CellArea CA;
		if (
			Cells.Buffer[iCellArea].Location.X != AreaLoc.X ||
			Cells.Buffer[iCellArea].Location.Y != AreaLoc.Y
		){
			CellArea find = {0};
			find.Location = AreaLoc;
			size_t CellAreaI = RJListFind_CellArea(&Cells, find, CellLocCmp);
			if (CellAreaI == (size_t)-1){
				CA = (CellArea){0};
				CA.Location = AreaLoc;
				cellArea = &CA;
			}else{cellArea = &Cells.Buffer[CellAreaI];}
		}

		size_t* nbrs = neighbours(cellArea, GetAreaIndex(loc));

		int nextState = seeds (cellArea->Area[GetAreaIndex(loc)], nbrs);


		// no need to set dead cells as they all start dead
		if (nextState > 0){addCell(&newCells, loc, nextState);}

		free(nbrs);
		RJListAppend_Vec2D(&done, loc);
	}}}}}
	RJListDelete_Vec2D(&done);
	pushToBuffer("number of alive cells is ");
	pushToBufferi(cellCount);
	logBuffer();
	clearBuffer();
	RJListDelete_CellArea(&Cells);
	Cells = newCells;
}
bool moved = false;
void mouseDown(MouseEvent* e){
	moved = false;
	bool buttonHandled = mouseDownButton(&autoRunBtn, e);
	if (autoRunBtn.needsRedrawing){draw();}
	if (buttonHandled){return;}
	Vec2D cellLoc = {
		.X = (e->loc.X-OffsetLocation.X) / 10,
		.Y = (e->loc.Y-OffsetLocation.Y) / 10,
	};
	if (e->button == 0){
		cellPlacingType = toggleCell(&Cells, cellLoc);
	}else if(e->button == 2){
		CellArea find = {0};
		find.Location = getAreaLocation(cellLoc);
		size_t CellAreaI = RJListFind_CellArea(&Cells, find, CellLocCmp);
		if (CellAreaI == (size_t)-1){cellPlacingType = 0;}
		else{cellPlacingType = Cells.Buffer[CellAreaI].Area[GetAreaIndex(cellLoc)];}
	}
	mouseState = e->button;
	draw();
	MouseLastLoc = (Vec2D){
		.X = e->loc.X,
		.Y = e->loc.Y,
	};
}
void mouseUp(MouseEvent* e){
	bool buttonHandled = mouseUpButton(&autoRunBtn, e);
	if (autoRunBtn.needsRedrawing){draw();}
	if (buttonHandled){return;}
	mouseState = -1;
}
void mouseMove(MouseEvent* e){
	bool buttonHandled = mouseMoveButton(&autoRunBtn, e);
	if (autoRunBtn.needsRedrawing){draw();}
	if (buttonHandled){return;}
	moved = true;
	if (mouseState == 0 || mouseState == 2){
		addCell(&Cells, (Vec2D){
			.X = (e->loc.X-OffsetLocation.X) / 10,
			.Y = (e->loc.Y-OffsetLocation.Y) / 10,
		}, cellPlacingType);
		draw();
	}
	if (mouseState == 1){
		OffsetLocation.X-=MouseLastLoc.X-e->loc.X;
		OffsetLocation.Y-=MouseLastLoc.Y-e->loc.Y;
		draw();
	}
	MouseLastLoc = (Vec2D){
		.X = e->loc.X,
		.Y = e->loc.Y,
	};
}
void keyDown(KeyboardEvent* e){
	if (strcmp(e->key, "Space")==0){
		nextItteration();
		draw();
	}
	if (strcmp(e->key, "Enter")==0 && Cells.Length > 0){
		OffsetLocation.X = -Cells.Buffer[0].Location.X * CELL_AREA_WIDTH * 10;
		OffsetLocation.Y = -Cells.Buffer[0].Location.Y * CELL_AREA_WIDTH * 10;
		draw();
	}
}
bool contextMenu(void){return moved;}

void autoNextIttr(void){
	nextItteration();
	draw();
	if (autoRun){runAfterTime(autoNextIttr, 50);}
}
void autoRunBtnCallback(void){
	autoRun = !autoRun;
	if (autoRun){
		autoNextIttr();
		autoRunBtn.Text = "Toggle Auto Run #";
	}else{autoRunBtn.Text = "Toggle Auto Run -";}
	draw();
}

void resize(int width, int height){
	if (Pixels){free(Pixels);}
	Pixels = malloc(width * height * sizeof *Pixels);
	Width = width;
	Height = height;
	oc = olivec_canvas(Pixels, Width, Height, Width);
}

int main(void){
	autoRunBtn = initButton((Button){
		.Text = "Toggle Auto Run -",
		.Font = olivec_rose_font,
		.FontSize = 1,
		.FontColour = 0xffffffff,
		.BackColour = 0xff000000,
		.HoverColour = 0xff252525,
		.FocusColour = 0xff454545,
		.BorderWidth = 2,
		.BorderColour = 0xffc1a05b,
		.Location = (Vec2D){.X=5,.Y=5},
		.Padding = 5,
		.Callback = autoRunBtnCallback,
	});
}
