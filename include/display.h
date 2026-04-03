#pragma once

class DateTime;

void drawDatePanelStyle1(const DateTime& now, double padRight);
void drawDatePanelStyle2(const DateTime& now, double padRight, int rowDate = 0);

void drawClock(int x, int y);
void drawClock24h(int x, int y);
void drawHome(int layout = 1);
void drawHomeLayout2();
void drawMenu();
void drawSubMenu(const char* title);
void drawTakwim();
void handleTakwimInput();