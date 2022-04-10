# MapReduce

[![wakatime](https://wakatime.com/badge/user/efb4fc44-7e85-40bc-a691-86644bbc5864/project/d62567ce-07f6-4d7b-acec-3178e0150fb9.svg)](https://wakatime.com/badge/user/efb4fc44-7e85-40bc-a691-86644bbc5864/project/d62567ce-07f6-4d7b-acec-3178e0150fb9)

Basic MapReduce System

Libraries:
- [Asio](https://think-async.com/Asio/)
- [CLI11](https://github.com/CLIUtils/CLI11)
- [spdlog](https://github.com/gabime/spdlog)
- [Nlohoman JSON](https://github.com/nlohmann/json)

For version history see [Changelog](https://github.com/Kleinrad/kleinrad_project/blob/main/CHANGELOG.org)

## Installation 
```
git clone git@github.com:Kleinrad/kleinrad_project.git
```

## Build
```
cd build && meson ..
ninja
```
## Usage
Start MapReduce master

```
./mr_master
```

Start MapReduce workers

```
./mr_worker
```

Start MapReduce client

```
./mr_client
```

### Client commands

```
send <jobType> [-f] <data>
print [-s] <printType>
quit, q
help, h
```

### Client parameters

1. jobType (0 - count characters, 1 - count words)

2. printType (0 - print plain, 1 - print histogram)
 
* -f ... use file as data
* -s ... print results sorted by value
