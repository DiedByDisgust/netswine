/*
 * ui.cpp
 *
 * Copyright (c) 2023 Sergio Cabrera Falcon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 *USA.
 *
 */

#include "ui.h"
#include "line.h"

#include <vector>

const char version[] = " version " VERSION;

extern std::vector<Line> lines;
extern int range;
extern bool must_update;

const uint64_t KB_limit = 4096; //4KB
const uint64_t MB_limit = 4194304; //4MB
const uint64_t GB_limit = 1610612736; //1.5GB

uint64_t to_kb(uint64_t bytes){return bytes/1024;}
uint64_t to_mb(uint64_t bytes){return bytes/1024/1024;}
uint64_t to_gb(uint64_t bytes){return bytes/1024/1024/1024;}

const int STARTING_LINE = 4;
static int scroll_index = 0;
static int max_items;
static int highlight = 0;
static int choice;
static bool show_full_paths;

void auto_unit(uint64_t*bytes,std::string*unit) {
  if(*bytes>GB_limit) {
    *bytes=to_gb(*bytes);
    *unit = "GB";
  }else if(*bytes>MB_limit) {
    *bytes=to_mb(*bytes);
    *unit = "MB";
  }else if(*bytes>KB_limit) {
    *bytes = to_kb(*bytes);
    *unit = "KB";
  }
}

std::vector<std::string> range_to_string {
  "Today","Yesterday","This Week","Last Week",
  "This Month","Last Month","Total"
};

void Line::show(int row) {
  uint64_t total = tx+rx;
  uint64_t percent = total * 100 / lines[0].sum();
  mvprintw(row,5,"%s",show_full_paths?path.c_str():name.c_str());
  int bar_length = (COLS - 15) * percent / 100;
  int blank_space = COLS - bar_length - 8;
  std::string string_bar = std::string(bar_length,'/');
  std::string unit = "B";
  auto_unit(&total,&unit);
  mvprintw(row+1, 5, "%s%*lu%s",string_bar.c_str(),blank_space,total,unit.c_str());
}

void ui_init() {
  WINDOW *screen = initscr();
  raw();
  noecho();
  cbreak();
  nodelay(screen, TRUE);
  keypad(screen, TRUE);
}

void ui_exit() {
  clear();
  endwin();
  exit(0);
}

void ui_tick() {
  switch (getch()) {
    case 'q':
      ui_exit();
      break;
    case 'p':
      show_full_paths^=true;
      break;
    case 'r':
      if(range==0)range+=7;
      range = (range-1)%7;
      must_update = true;
      break;
    case 'R':
      range = (range+1)%7;
      must_update = true;
      break;
    case KEY_UP:
      if(highlight==0) {
        highlight=lines.size() - 1;
        scroll_index=lines.size() - max_items;
      }else if(highlight==scroll_index) {
        --scroll_index;
        --highlight;
      }else --highlight;
      break;
    case KEY_DOWN:
      if(highlight==lines.size()-1){
        scroll_index=0;
        highlight=0;
      }else if(highlight==scroll_index+max_items-1) {
        scroll_index++;
        highlight++;
      }else highlight++;
      break;
    case 10:
      choice = highlight;
      break;
    default:
      break;
  }
}

void iterable_list() {
  //list
  int y,i;
  y=STARTING_LINE;
  for (i = scroll_index; i < scroll_index+max_items  && i < lines.size(); ++i) {
    if (highlight == i) {
      attron(A_REVERSE);
      lines[i].show(y);
      attroff(A_REVERSE);
    }else
      lines[i].show(y);
    y+=3;
  }
//  refresh();
}

void ui_refresher() {
  if(COLS < 62 || LINES < 15) {
    erase();
    mvprintw(0, 0, "The terminal is too narrow! Please make it wider.\nI'll wait...");
    return;
  }
  max_items = (LINES - 3) / 3;
  if(highlight>scroll_index+max_items-1)highlight=scroll_index+max_items-1;
  if(scroll_index+max_items>lines.size())scroll_index=lines.size()-max_items;
  if(scroll_index<0)scroll_index=0;

  erase();
  attron(A_REVERSE);
  mvprintw(0,0,"%s%-*s","NetSwine",COLS-8,version);
  attroff(A_REVERSE);

  mvprintw(2,0,"%s: usage %lu%s (r)(R)",range_to_string[range].c_str(),to_mb(Line::get_total(lines.data(), lines.size())),"MB");

  iterable_list();

  //mvprintw(LINES-8,0,"LINES: %i\ntotal_items: %zu\nmax_items: %i\nhighlight: %i\nscroll: %i\nrange: %i",LINES,lines.size(),max_items,highlight,scroll_index,range); //debug
  refresh();
}

