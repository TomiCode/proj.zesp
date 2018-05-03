#include "database.h"

Database::Database(const char *filepath)
{
  this->dbfile = fopen(filepath, "r+b");
  if (this->dbfile == NULL) {
    this->dbfile = fopen(filepath, "w+b");
  }
}

Database::~Database()
{
  if (this->dbfile)
    fclose(this->dbfile);
}

uint64_t Database::hash(const char *str)
{
  uint64_t _hash = 0xBCA95B;
  for(; *str != '\0'; str++)
    _hash = (_hash ^ *str) << 4;
  return _hash;
}

void Database::check(void)
{
  std::lock_guard<std::mutex> lock(this->mtx);

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
}

bool Database::create(const char *username, const char *password)
{
  std::lock_guard<std::mutex> lock(this->mtx);
  database_head_t header = {0};

  // Check database header.
  fseek(this->dbfile, 0, SEEK_SET);
  if (fread(&header, sizeof(database_head_t), 1, this->dbfile) != 1) {
    printf("Invalid database header!\n");
    return false;
  }

  // Create new user segment
  database_segment_t new_segment = {{}};
  new_segment.password = this->hash(password);
  strcpy(new_segment.username, username);

  // Write the segment into the file
  fseek(this->dbfile, sizeof(database_segment_t) * header.users, SEEK_CUR);
  if (fwrite(&new_segment, sizeof(database_segment_t), 1, this->dbfile) != 1) {
    printf("Can not write new user segment.\n");
    return false;
  }

  // Increment users count in the database file.
  header.users++;
  fseek(this->dbfile, 0, SEEK_SET);
  bool valid = fwrite(&header, sizeof(database_head_t), 1, this->dbfile) == 1;
  fflush(this->dbfile);

  return valid;
}

bool Database::exists(const char *username)
{
  std::lock_guard<std::mutex> lock(this->mtx);
  database_head_t header = {0};

  // Read database header
  fseek(this->dbfile, 0, SEEK_SET);
  if (fread(&header, sizeof(database_head_t), 1, this->dbfile) != 1) {
    printf("Invalid database header!\n");
    return false;
  }

  database_segment_t segment = {{}};
  // Find the user segment
  for (uint16_t i = 0; i < header.users; i++) {
    if (fread(&segment, sizeof(database_segment_t), 1, this->dbfile) != 1) {
      printf("Can not read %d segment from user database.\n", i);
      break;
    }
    if (strcmp(segment.username, username) == 0)
      return true;
  }

  // No segment found - user does not exists.
  return false;
}

bool Database::authorize(const char *username, const char *password)
{
  std::lock_guard<std::mutex> lock(this->mtx);
  database_head_t header = {0};

  fseek(this->dbfile, 0, SEEK_SET);
  if (fread(&header, sizeof(database_head_t), 1, this->dbfile) != 1) {
    printf("Invalid database header!\n");
    return false;
  }

  uint64_t passwd = this->hash(password);
  database_segment_t segment = {{}};

  for (uint16_t i = 0; i < header.users; i++) {
    if (fread(&segment, sizeof(database_segment_t), 1, this->dbfile) != 1) {
      printf("Can not read %d segment from user database.\n", i);
      break;
    }
    if (strcmp(segment.username, username) == 0) {
      return (segment.password == passwd);
    }
  }

  return false;
}
