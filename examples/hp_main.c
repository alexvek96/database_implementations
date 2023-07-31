#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "hp_file.h"
#define FILE_NAME "data.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
  BF_Init(LRU);

  HP_CreateFile(FILE_NAME);
  HP_info* info = HP_OpenFile(FILE_NAME);

  Record record;
  srand(12569874);
  printf("------------------------------------------------------------------------------------------\n");
  printf("\n");
  printf("Inserting Entries...\n");
  printf("\n");
  for (int id = 0; id < RECORDS_NUM; id++) {
    record = randomRecord();
    HP_InsertEntry(info, record);
  }

  printf("------------------------------------------------------------------------------------------\n");
  printf("\n");
  printf("RUN GetAllEntries\n");
  printf("\n");
  int id = rand() % RECORDS_NUM;
  printf("\nSearching for: %d\n",id);
  HP_GetAllEntries(info, id);
  hpHashStatistics(FILE_NAME);
  HP_CloseFile(info);
  BF_Close();
}
