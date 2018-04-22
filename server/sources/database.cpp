#include "database.h"

Database::Database(const char *filepath)
{
  this->dbfile = fopen(filepath, "a+b");
}

Database::~Database()
{
  if (this->dbfile)
    fclose(this->dbfile);
}

void Database::check(void)
{
  this->mtx.lock();
  fseek(this->dbfile, 0, SEEK_SET);
  database_head_t header = {0};

  size_t read = fread(&header, sizeof(database_head_t), 1, this->dbfile);
  if (read != 1) {
    printf("Database %zu bytes. Inserting new header..\n", read);
    fseek(this->dbfile, 0, SEEK_SET);
    header._version = 0x0DB0;
    header.users = 0;
    fwrite(&header, sizeof(database_head_t), 1, this->dbfile);
    fflush(this->dbfile);
  }
  printf("Database loaded, %d users.\n", header.users);
  this->mtx.unlock();
}

bool Database::create(const char *username, const char *password)
{
  return true;
}

bool Database::exists(const char *username)
{
  return true;
}

bool Database::authorize(const char *username, const char *password)
{
  return true;
}

