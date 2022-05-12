#pragma once

#include "includes.h"


void DrawFilledRect(int x, int y, int w, int h, D3DCOLOR color);

void DrawLine(int x1, int y1, int x2, int y2,  int thickness, D3DCOLOR color);

void DrawEspBox2D(Vector2 top, Vector2 bot, int thickness, D3DCOLOR color);

//bool DrawMessage(LPD3DXFONT font, unsigned int x, unsigned int y, int alpha, unsigned char r, unsigned char g, unsigned char b, LPCSTR Message); //Draw message


