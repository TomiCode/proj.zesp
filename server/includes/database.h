#ifndef DATABASE_H
#define DATABASE_H

#include <iostream>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint16_t _version;
  uint16_t users;
} __attribute__((packed)) database_head_t;

typedef struct {
  char username[64];
  uint64_t password;
} __attribute__((packed)) database_segment_t;

class Database {
public:
  Database(const char *filepath);
  ~Database();

  static uint64_t hash(const char *str);

  void check(void);
  bool create(const char *username, const char *password);
  bool exists(const char *username);
  bool authorize(const char *username, const char *password);
private:
  FILE *m_dbfile;
  std::mutex m_mutex;
};

#endif // DATABASE_H
