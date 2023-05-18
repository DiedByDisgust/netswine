/*
 * reader.cpp
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

#include <chrono>
#include <sqlite3.h>
#include <iostream>

#include "reader.h"

const char* DB_PATH = "/var/lib/netswine/registry.db";

extern int range;

std::string range_string() {
  auto now = std::chrono::system_clock::now();
  std::time_t currentTimeT = std::chrono::system_clock::to_time_t(now);
  std::tm*localTime = std::localtime(&currentTimeT);
  uint64_t year = localTime->tm_year-100;
  uint64_t month = year * 100 + localTime->tm_mon+1;
  uint64_t today = month * 100 + localTime->tm_mday;
  std::string from = "timestamp>";
  std::string to = " and timestamp<";

  switch (range) {
    case TODAY:
      from += std::to_string(today*10000);
      to = "";
      break;
    case YESTERDAY:
      from += std::to_string((today-1)*10000);
      to += std::to_string(today*10000);
      break;
    case THISWEEK:
      from += std::to_string((today-localTime->tm_wday)*10000);
      to = "";
      break;
    case LASTWEEK: {
      uint64_t last_sunday = today - localTime->tm_wday;
      from += std::to_string((last_sunday-7)*10000);
      to += std::to_string(last_sunday*10000);
      break;
    }
    case THISMONTH:
      from += std::to_string(month*1000000);
      to = "";
      break;
    case LASTMONTH:
      from += std::to_string((month-1)*1000000);
      to += std::to_string(month*1000000);
      break;
    case TOTAL_EVER:
      from += std::to_string(0);
      to = "";
      break;
    default:
      break;
  }
  return from + to;
}

std::vector<std::string> get_processes_list() {
  sqlite3* db;
  int rc = sqlite3_open_v2(DB_PATH, &db, SQLITE_OPEN_READONLY, NULL);
  if (rc != SQLITE_OK) {
    std::cerr << "cannot open database: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    exit(EXIT_FAILURE);
  }

  sqlite3_stmt* stmt;
  std::string request = "SELECT DISTINCT process FROM registry WHERE "+range_string()+";";
  rc = sqlite3_prepare_v2(db, request.c_str(), -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    std::cerr << "get_processes_list(): cannot prepare sql statement: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    exit(EXIT_FAILURE);
  }

  std::vector<std::string> processes;
  while(sqlite3_step(stmt) == SQLITE_ROW) {
    int col_type = sqlite3_column_type(stmt,0);
    switch (col_type) {
      case SQLITE_BLOB:continue;
      case SQLITE_TEXT:
        const unsigned char *value = sqlite3_column_text(stmt,0);
        processes.emplace_back(reinterpret_cast<const char*>(value));
    }
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);
  return processes;
}

Line get_total_line(sqlite3* db, std::string process_string) {
  sqlite3_stmt* stmt;
  std::string request = "SELECT sum(tx),sum(rx) FROM registry WHERE process = '"+process_string+"' and "+range_string()+";";
  int rc = sqlite3_prepare_v2(db, request.c_str(), -1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    std::cerr << "get_total_line(): cannot prepare sql statement: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    exit(EXIT_FAILURE);
  }
  uint64_t tx,rx;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    tx = sqlite3_column_int64(stmt,0);
    rx = sqlite3_column_int64(stmt,1);
  }

  std::string app_name = process_string;
  if(process_string.find('\\') != std::string::npos){
    app_name = "wine::";
    std::string short_name = process_string.substr(process_string.find_last_of('\\')+1);
    app_name += short_name;
  }else if (process_string.find('/') != std::string::npos) {
    app_name = process_string.substr(process_string.find_last_of('/')+1);
  }
  sqlite3_finalize(stmt);
  return Line(process_string,app_name,tx,rx);
}

std::vector<Line> get_total_list() {
  static sqlite3* db;
  int rc = sqlite3_open_v2(DB_PATH, &db, SQLITE_OPEN_READONLY, NULL);
  if (rc != SQLITE_OK) {
    std::cerr << "cannot open database: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    exit(EXIT_FAILURE);
  }

  std::vector<Line> lines;
  for ( std::string p : get_processes_list()) {
    Line l = get_total_line(db,p);
    lines.emplace_back(l);
  }
  sqlite3_close(db);
  return lines;
}
