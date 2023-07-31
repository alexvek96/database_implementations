#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "sht_table.h"
#define S_RECORDS_NUM 60            // you can change it if you want
#define FILE_NAME "data.db"
#define INDEX_NAME "index.db"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }


int main() {
    srand(12569874);
    BF_Init(LRU);
    // Αρχικοποιήσεις
    HT_CreateFile(FILE_NAME, S_NUM_OF_BUCKETS);
    SHT_CreateSecondaryIndex(INDEX_NAME, S_NUM_OF_BUCKETS, FILE_NAME);
    HT_info* info = HT_OpenFile(FILE_NAME);
    SHT_info* index_info = SHT_OpenSecondaryIndex(INDEX_NAME);

    // Θα ψάξουμε στην συνέχεια το όνομα searchName
    Record record=randomRecord();
    char searchName[15];
    strcpy(searchName, record.name);

    // Κάνουμε εισαγωγή τυχαίων εγγραφών τόσο στο αρχείο κατακερματισμού
    // τις οποίες προσθέτουμε και στο δευτερεύον ευρετήριο
    printf("------------------------------------------------------------------------------------------\n");
    printf("\n");
    printf("Inserting Entries...\n");
    printf("\n");    
    for (int id = 0; id < S_RECORDS_NUM; id++) {
        record = randomRecord();
        int block_id;
        block_id = HT_InsertEntry(info, record);
        SHT_SecondaryInsertEntry(index_info, record, block_id);
    }
    // Τυπώνουμε όλες τις εγγραφές με όνομα searchName
    printf("------------------------------------------------------------------------------------------\n");
    printf("\n");
    printf("RUN GetAllEntries for name %s\n", searchName);
    printf("\n");
    SHT_SecondaryGetAllEntries(info, index_info, searchName);

    // 2ο test case
    printf("------------------------------------------------------------------------------------------\n");
    printf("\n");
    printf("RUN GetAllEntries for name %s\n", "Giorgos");
    printf("\n");
    SHT_SecondaryGetAllEntries(info, index_info, "Giorgos");
    
    sHashStatistics(INDEX_NAME);

    // Κλείνουμε το αρχείο κατακερματισμού και το δευτερεύον ευρετήριο
    SHT_CloseSecondaryIndex(index_info);
    HT_CloseFile(info);
    BF_Close();
}
