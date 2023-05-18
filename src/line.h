/*
 * line.h
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

#include <cstdint>
#include <string>
class Line {
public:
  Line();
  Line(std::string full_path,std::string name, uint64_t tx_value, uint64_t rx_value);
  //~Line();
  void show(int row);

  bool operator<(const Line& other) const {
        return rx+tx < other.rx + other.tx;
  }

  std::string get_pname();

  static uint64_t get_total(Line *lines,int size) {
    uint64_t sum = 0;
    for (int i=0; i<size; i++) {
      sum += lines[i].sum();
    }
    return sum;
  }
  uint64_t sum();
private:
  std::string path;
  std::string name;
  uint64_t tx,rx;
};
