# Netswine
## Introduction

This is a small network data metric tool that runs as a daemon. Collects network traffic data, then groups it by process, so can be analized later. This give you the ability to understand which processes are consuming your data and when. Uses [nethogs](https://github.com/raboof/nethogs) for the collection of data grouped by process. Works only on Linux systems.

## Status

A naive solution with a lot of work to do.
Any bug report or suggest will be apreciated

## Installation

### Dependencies

- `libnethogs` (git version needed, libnethogs is broken on 'release' version)
- `sqlite3`
- `ncurses`

### Building from source

Install the development libraries of the dependencies.

#### Archlinux

Archlinux doesn't have developmental packages. All headers and libs are packaged together.
```
paru -S netswine-git
```

#### Other distros

i must remark that nethogs must be build at least from [this nethogs commit](https://github.com/raboof/nethogs/commit/24973112baa1fb521e84489252620b1c0b34a644)

#### clone this repo

```
git clone https://github.com/DiedByDisgust/netswine.git
```

#### Building

simply `make`, in order to register data usage yo need to run `netswined` as root, while `netswine` is the reader.
```
make
sudo ./src/netswined
./src/netswine
```

#### Installing 

just
```
sudo make install
```

#### Use

`netswined` is supposed to run as a daemon, so `enable` and `start` it.
```
systemctl enable netswined.service
systemctl start netswined.service
```
now you can run `netswine` and read data usage statistics.

## Licence

Copyright 2023 Sergio Cabrera Falcon sergio.cabrerafalcon@gmail.com 

see the COPYING file for the license text.
