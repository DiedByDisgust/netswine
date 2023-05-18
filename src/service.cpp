/*
 * netswine service.cpp
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

#include <libnethogs.h>
#include <sqlite3.h>
#include <pwd.h>

#include <iostream>
#include <thread>
#include <map>
#include <sstream>
#include <iomanip>
#include <mutex>

const char version[] = " version " VERSION;

static int monitor_status = NETHOGS_STATUS_OK;

std::mutex access;

struct entry {
  std::string user;
  std::string device;
  std::string process;
  uint64_t tx,rx;

  entry():tx(0),rx(0) {}

  entry(const std::string user,const std::string device,const std::string process,uint64_t tx, uint64_t rx):
    user(user),device(device),process(process),tx(tx),rx(rx){}

  entry operator-(const entry& other) const {
    return entry(user,device,process,tx-other.tx,rx-other.rx);
  }
};

std::map<std::string, entry> lines,l_last;

std::map<std::string,entry> l_this() {
  std::map<std::string,entry> result;

  std::lock_guard<std::mutex> lock(access);
  for(const auto& pair : lines) {
    std::string key = pair.first;
    const entry& e1 = pair.second;

    //check if key exist in l_last
    if(l_last.count(key)>0) {
      const entry& e2 = l_last.at(key);
      result[key] = e1 - e2;
    }else{
      //doesn't exist so take the lines value
      result[key] = e1;
    }
  }

  //set l_last as lines value before continue
  l_last.clear();
  l_last = lines;

  return result;
}

//callback for monitor
static void monitorCallback(int action, NethogsMonitorRecord const* update) {
  entry e = entry(
    getpwuid(update->uid)->pw_name,
    update->device_name,
    update->name,
    update->sent_bytes,
    update->recv_bytes
  );
  std::string key = std::to_string(update->uid) + update->device_name + update->name + std::to_string(update->pid);
  std::lock_guard<std::mutex> lock(access);
  if(lines.count(key)>0) {
    lines[key].tx = e.tx;
    lines[key].rx = e.rx;
  }else lines[key] = e;
}

static void nethogsMonitorThread() {monitor_status = nethogsmonitor_loop(monitorCallback, NULL, 1000);}

int main(){
  std::cout<<PROGRAM_NAME<<version<<std::endl;
  
  const int dir_err = system("mkdir -p /var/lib/netswine");
  if (-1 == dir_err) {
    std::cerr << "Error creating database directory" << std::endl;
    return 1;
  }

  sqlite3* db;
  int rc = sqlite3_open("/var/lib/netswine/registry.db",&db);
  if (rc != SQLITE_OK) {
    std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    return 1;
  }

  const char* create_table_sql = "CREATE TABLE IF NOT EXISTS registry (id INTEGER PRIMARY KEY, timestamp TEXT, user TEXT, device TEXT, process TEXT, tx INTEGER, rx INTEGER);";
  rc = sqlite3_exec(db, create_table_sql, nullptr,nullptr,nullptr);
  if (rc != SQLITE_OK) {
    std::cerr << "Error creating table: " << sqlite3_errmsg(db) << std::endl;
    sqlite3_close(db);
    return 1;
  }

  std::thread monitor(&nethogsMonitorThread);

  while(monitor_status==NETHOGS_STATUS_OK) {
    std::this_thread::sleep_for(std::chrono::minutes(1));

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    std::stringstream stime;
    stime << std::put_time(std::localtime(&timestamp),"%y%m%d%H%M");

    for (const auto& pair : l_this()) {
      if(pair.second.tx == 0 && pair.second.rx == 0)continue;

      const char* insert_sql = "INSERT INTO registry (timestamp,user,device,process,tx,rx) VALUES (?,?,?,?,?,?);";
      sqlite3_stmt* stmt;
      rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
      if (rc != SQLITE_OK) {
        std::cerr << "Error preparing insert statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
      }

      sqlite3_bind_text(stmt, 1, stime.str().c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, pair.second.user.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 3, pair.second.device.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 4, pair.second.process.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_int64(stmt, 5, pair.second.tx);
      sqlite3_bind_int64(stmt, 6, pair.second.rx);
      
      rc = sqlite3_step(stmt);
      if (rc != SQLITE_DONE) {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
      }
      sqlite3_finalize(stmt);
    } 
  }
  
  nethogsmonitor_breakloop();
  monitor.join();
}

