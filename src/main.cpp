/*
 * netswine main.cpp
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
#include "reader.h"
#include <vector>
#include <algorithm>

std::vector<Line> lines;
int range = TODAY;
bool must_update;

void update_lines() {
  lines = get_total_list();

  std::sort(lines.rbegin(),lines.rend());
  must_update = false;
}

int main() {
  update_lines();

  ui_init();
  while (true) {
    ui_tick();
    if(must_update)update_lines();
    ui_refresher();
  }
  ui_exit();

} 
