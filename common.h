#pragma once
typedef struct{
	int X;
	int Y;
}Vec2D;
bool Vec2DComp(Vec2D a,Vec2D b){
	return a.X == b.X && a.Y == b.Y;
}
