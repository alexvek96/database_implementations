#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int main() {
  int id;
  BF_Init(LRU);
  HT_CreateFile(FILE_NAME, NUM_OF_BUCKETS);
  HT_info* info = HT_OpenFile(FILE_NAME);

  Record record;
  srand(12569874);
  printf("------------------------------------------------------------------------------------------\n");
  printf("\n");
  printf("Inserting Entries...\n");
  printf("\n");
  for (id = 0; id < RECORDS_NUM; id++) {
    record = randomRecord();
    HT_InsertEntry(info, record);
  }
  
  printf("------------------------------------------------------------------------------------------\n");
  printf("\n");
  printf("RUN GetAllEntries\n");
  printf("\n");
  id = rand() % RECORDS_NUM;
  HashStatistics(FILE_NAME);
  HT_CloseFile(info);
  BF_Close();
}
