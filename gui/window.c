#include <include/gui/window.h>
#include <include/gui/vga.h> 

Window windows[MAX_WINDOWS];

void init_windows() {
    for(int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].visible = 0;
        windows[i].dragging = 0;
    }
}

void draw_window_frame(Window* win) {
    if (!win->visible) return;

  
    fill_rect_backbuffer(win->x, win->y, win->width, win->height, GUI_GREY);

   
    draw_line_backbuffer(win->x, win->y, win->x + win->width, win->y, GUI_WHITE); 
    draw_line_backbuffer(win->x, win->y, win->x, win->y + win->height, GUI_WHITE);
    draw_line_backbuffer(win->x, win->y + win->height, win->x + win->width, win->y + win->height, GUI_DARK_GREY); 
    draw_line_backbuffer(win->x + win->width, win->y, win->x + win->width, win->y + win->height, GUI_DARK_GREY); 


    char title_color = win->active ? GUI_BLUE : GUI_DARK_GREY;
    fill_rect_backbuffer(win->x + 3, win->y + 3, win->width - 6, 16, title_color);


    draw_string_backbuffer(win->x + 8, win->y + 6, GUI_WHITE, win->title);

    fill_rect_backbuffer(win->x + win->width - 18, win->y + 5, 12, 12, GUI_RED);
    draw_char_backbuffer(win->x + win->width - 15, win->y + 7, GUI_WHITE, 'X');

    fill_rect_backbuffer(win->x + 3, win->y + 22, win->width - 6, win->height - 25, GUI_WHITE);
    draw_rect_backbuffer(win->x + 3, win->y + 22, win->width - 6, win->height - 25, GUI_BLACK);
}

void draw_all_windows() {
    for(int i = 0; i < MAX_WINDOWS; i++) {
        if(windows[i].visible) {
            draw_window_frame(&windows[i]);
        
        }
    }
}

void create_window(int id, int x, int y, int w, int h, char* title) {
    
    if (id < 0 || id >= MAX_WINDOWS) return;

    windows[id].x = x;
    windows[id].y = y;
    windows[id].width = w;
    windows[id].height = h;
    windows[id].visible = 1;     
    windows[id].dragging = 0;    
    windows[id].active = 1;     
    windows[id].content_id = id; 


    int i = 0;
    while (title[i] != '\0' && i < 31) {
        windows[id].title[i] = title[i];
        i++;
    }
    windows[id].title[i] = '\0'; 


    for (int j = 0; j < MAX_WINDOWS; j++) {
        if (j != id) {
            windows[j].active = 0;
        }
    }
}

void handle_window_events(int mx, int my, char left_clicked) {
    static int dragged_win = -1;

    if (left_clicked) {
      
        if (dragged_win != -1) {
            windows[dragged_win].x = mx - (windows[dragged_win].width / 2);
            windows[dragged_win].y = my - 10;
            return;
        }
        for (int i = MAX_WINDOWS - 1; i >= 0; i--) {
            if (windows[i].visible) {
               
                if (mx >= windows[i].x && mx <= (windows[i].x + windows[i].width) &&
                    my >= windows[i].y && my <= (windows[i].y + 20)) {
                    
                    dragged_win = i;
                    windows[i].active = 1;
                    
          
                    for(int j = 0; j < MAX_WINDOWS; j++) 
                        if(j != i) windows[j].active = 0;
                    
                    break; 
                }
            }
        }
    } else {
        dragged_win = -1; 
    }
}