#ifndef WINDOW_H
#define WINDOW_H

#define MAX_WINDOWS 10
#define WIN_TITLE_HEIGHT 20

typedef struct {
    int x, y;
    int width, height;
    char title[32];
    char visible;
    char dragging;
    int content_id; // 0=terminal, 1=calc, 2=paint
    char active;    
} Window;


void init_windows();
void draw_all_windows();
void create_window(int id, int x, int y, int w, int h, char* title);

void create_window(int id, int x, int y, int w, int h, char* title);
void handle_window_events(int mx, int my, char left_clicked);
#endif