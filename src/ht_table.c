#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

//-----------------------------------------------------------------------------

// hash function
int hasher(int id){
 
    return ((id + rand()) % NUM_OF_BUCKETS);      // we have NUM_OF_BUCKETS=15 buckets for the hash table
}
//-----------------------------------------------------------------------------

int HT_CreateFile (char *fileName, int buckets){

    int fileDesc2;
    BF_Block_Init(&block);
    int blocks_num = buckets;
    printf("Creating your HT file...\n");
    error = BF_CreateFile(fileName);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    // acquire the File Descriptor
    error = BF_OpenFile(fileName, &fileDesc2);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    // allocate memory for the Block '0' of the File and initialize its fields
    error = BF_AllocateBlock(fileDesc2, block);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    error = BF_GetBlock(fileDesc2, 0, block);
    if (error){
        BF_PrintError(error);
        return -1;
    }

    data = BF_Block_GetData(block);
    // initialization of the HT_info block
    p_file_info = data;
    p_file_info->type = 'H';                        // Hash Table (HT) file
    p_file_info->fileDesc = fileDesc2;
    p_file_info->numBuckets = buckets;
    p_file_info->num_of_blocks = buckets;
    for (i = 0; i<buckets; i++){

        p_file_info->HashTable[i] = i + 1;
        p_file_info->Records_per_Bucket[i] = 0;     // useful for the HashStatistics() function
        p_file_info->Blocks_per_Bucket[i] = 0;      // useful for the HashStatistics() function
        p_file_info->Blocks_with_overflow[i] = 0;   // useful for the HashStatistics() function
    }

    // the other hash buckets...
    for (i = 0; i < buckets; i++){

        // create a block for every hash bucket
        error = BF_AllocateBlock(p_file_info->fileDesc, block);
        if (error < 0){
            BF_PrintError(error);
            return -1;
        }
        // find the id of the Block (which equals its number in the series)
        BF_GetBlockCounter(p_file_info->fileDesc, &blocks_num);
        
        // reading the Block from the disc
        error = BF_GetBlock(p_file_info->fileDesc, p_file_info->HashTable[i], block);
        if (error){
            BF_PrintError(error);
            return -1;
        }

        p_block_info = (HT_block_info*) BF_Block_GetData(block);
        // data initialization for Block
        block_info.num_of_records = 0;
        block_info.next_block = -1;

        // copy HT_block_info struct at the beginning of each block
        memcpy(p_block_info, &block_info, sizeof(block_info));
        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);
    }
    // copy HT_info struct at the beginning of Block '0'
    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);
    error = BF_CloseFile(p_file_info->fileDesc);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    printf("Finished HT file creation.\n");
    return 0;
}

//-----------------------------------------------------------------------------

HT_info* HT_OpenFile (char *fileName){
    
    int fileDesc2;
    error = BF_OpenFile(fileName, &fileDesc2);
    if (error){
        BF_PrintError(error);
        return NULL;
    }
    error = BF_GetBlock(fileDesc2, 0, block);
    if (error){
        BF_PrintError(error);
        return NULL;
    }
    data = BF_Block_GetData(block);
    p_file_info = data;
    p_file_info->fileDesc = fileDesc2;
    p_file_info->num_of_blocks = NUM_OF_BUCKETS;
    return p_file_info;
}

//-----------------------------------------------------------------------------

int HT_CloseFile (HT_info* p_file_info){

    error = BF_CloseFile(p_file_info->fileDesc);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    // free the struct we passed to the function
    free(p_file_info);
    printf("Preparing to end program...\n");
    printf("Closed HT file and freed HT-related memory.\n");
    return 0;
}

//-----------------------------------------------------------------------------

int HT_InsertEntry (HT_info* p_file_info, Record record){
    
    // find the Hash place for the Record through its id
    int index = hasher(record.id);
    int block_id;
    block_id = p_file_info->HashTable[index];           // this is the initial block id where the Record shall go
    printf("---NEW INSERTION in HT---\n");
    printf("Record with id '%d' goes to bucket '%d'.\n", record.id, index);
    // acquire the right hash block to insert the Record
    printf("####### %d\n", p_file_info->HashTable[index]);
    error = BF_GetBlock(p_file_info->fileDesc, p_file_info->HashTable[index], block);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    data = BF_Block_GetData(block);
    p_block_info = data;
    data = p_block_info + sizeof(HT_block_info);     
    p_record = data;                                                // points right after the HT_block_info struct in the block
    // if Block is not full, insert record
    if ((p_block_info->num_of_records) < MAX_NUM_OF_RECORDS){

        p_record = data + (p_block_info->num_of_records)*sizeof(record);  // point after the last Record of the Block
        memcpy(p_record, &record, sizeof(record));                        // copy Record data in the Block  
        (p_block_info->num_of_records)++;                                 // update HT_block_info
        (p_file_info->Records_per_Bucket[index])++;
        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);
        printf("---FINISHED INSERTION in HT: 'id=%d' -> 'bucket=%d'---\n", record.id, index);
    }
    else{   // if Block is full, create a new block to insert the record

        (p_file_info->Blocks_with_overflow[index])++;                   // one more block has overflown
        BF_AllocateBlock(p_file_info->fileDesc, block);                 // allocate new Block
        printf("Created a new block in bucket '%d'.\n", index);
        p_block_info = (HT_block_info*) BF_Block_GetData(block);                         
        (p_block_info->num_of_records) = 0;                             // initialize the HT_block_info
        (p_block_info->next_block) = -1;
        (p_file_info->num_of_blocks)++;                                 // created one more block
        /* things changed because we added one more block for this Record
           so the block id where the Record potentially and finally goes, is updated*/
        block_id = (p_file_info->num_of_blocks);                                                                                                
        (p_file_info->Blocks_per_Bucket[index])++;                      // one more block in this hash bucket chain
        (p_block_info->next_block) = (p_file_info->HashTable[index]);   // the new block of the chain points to its previous
        (p_file_info->HashTable[index]) = (p_file_info->num_of_blocks); // now the first block of the chain is the new one
        data = p_block_info + sizeof(HT_block_info);                    // points to the first AVAILABLE space for a record in the block
        p_record = data;
        memcpy(p_record, &record, sizeof(record));                      // copy Record data in the block
        (p_block_info->num_of_records)++;                               // update HT_block_info
        (p_file_info->Records_per_Bucket[index])++;
        printf("Inserting the new record with id '%d' in a new block...\n", p_record->id);  
        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);
        printf("---FINISHED INSERTION in HT: 'id=%d' -> 'bucket=%d' (chain expansion)---\n", record.id, index);
    }
    return block_id;     // return block id where Record was finally inserted
}

//-----------------------------------------------------------------------------

int HT_GetAllEntries(HT_info* p_file_info, int value ){  

    // we must first find the hash bucket where the 'value' (id) has been stored
    // then, we iterate through the hash chain until we find the 'Record record' with 'record.id=value'
    int i = 0;
    int blockCounter = 1;         // counts the number of blocks we iterate through
    int index;
    index = hasher(value);        // find the hashed value of 'value', so we know in which bucket to search
    int found = 0;                // flag-> if '1' then we found something, if '0' we didn't find anything 
    
    error = BF_GetBlock(p_file_info->fileDesc, p_file_info->HashTable[index], block);    // first block of the hash bucket
    if (error){
        BF_PrintError(error);
        return -1;
    }

    data = BF_Block_GetData(block);                 // points to the 'HT_block_info' section
    p_block_info = (HT_block_info*) data;
    data = p_block_info + sizeof(HT_block_info);    // points to the 1st record in the 1st block of the hash bucket chain
    p_record = (Record*) data;
    // iterate through every record in every block of the hash bucket
    printf("Searching for records with value (id) '%d' in hash bucket '%d'...\n", value, index);
    
    while (i < p_block_info->num_of_records){
        if ((p_record->id) == value){
            
            printf("The value '%d' corresponds to Record: ", value);
            printRecord(*p_record);
            found = 1;
            i++;
            if (i != (p_block_info->num_of_records-1)){
                p_record = p_record + sizeof(Record);               // go to next Record
            }
        }
        else if (i == (p_block_info->num_of_records-1)){            // didn't find the 'value' and we are at the last Record of the Block
            
            if (p_block_info->next_block != -1){                                                // means there is another block in the hash chain
                printf("Hopping to next block in the hash bucket chain...\n");
                error = BF_GetBlock(p_file_info->fileDesc, p_block_info->next_block, block);    // get pointer to next block of hash chain
                data = BF_Block_GetData(block);                             
                p_block_info = (HT_block_info*) data;           // points to the 'HT_block_info' section
                data = p_block_info + sizeof(HT_block_info);
                p_record = (Record*) data;                      // points to the 1st record in the block
                i = 0;
                if (error){
                    BF_PrintError(error);
                    return -1;
                }
                blockCounter++;             // one more block to search for Records
            }
            else {break;}                   // means we have no more blocks in the hash chain
        }
        else{
            i++;
            p_record = p_record + 1;        // go to next Record 
        }  
    }
    printf("Iterated through %d block(s) to find all records with 'Record id = %d'.\n", blockCounter, value);
    
    if (found==0){
        printf("The given 'value' does not exist in the file.\n");
    }
    printf("------------------------------------------------------------------------------------------\n");
    return blockCounter;
}

//-----------------------------------------------------------------------------

int HashStatistics(char* filename){

    (HT_info*) p_file_info;
    p_file_info = HT_OpenFile (filename);      // acquiring the file info
    if (p_file_info == NULL){
        return -1;
    }
    printf("\n");
    printf("---Simple HashTable HASH STATISTICS---\n");
    printf("\n");
    printf("Total number of blocks in the file: %d\n", p_file_info->num_of_blocks);
    int min, avg, max;
    int buckets_with_overflown_blocks = 0;
    int sum_records = 0;
    int sum_blocks = 0;
    for (int i=0; i<NUM_OF_BUCKETS; i++){

        sum_records = sum_records + p_file_info->Records_per_Bucket[i];  
        sum_blocks = sum_blocks + p_file_info->Blocks_with_overflow[i];

        if ((p_file_info->Blocks_with_overflow[i]) > 0){
            buckets_with_overflown_blocks++;
        }
        if (i==0){
            max = p_file_info->Records_per_Bucket[i];
            min = p_file_info->Records_per_Bucket[i];
        }
        else{
            if (max < p_file_info->Records_per_Bucket[i]){
                max = p_file_info->Records_per_Bucket[i];       // upgrade max Records per bucket
            }
            if (min > p_file_info->Records_per_Bucket[i]){
                min = p_file_info->Records_per_Bucket[i];       // upgrade min Records per bucket
            }
        }
    }
    printf("(HT) Minimum number of records existing in a hash bucket: %d\n", min);
    printf("(HT) Average number of records existing in a hash bucket: %d\n", (int) sum_records/NUM_OF_BUCKETS);
    printf("(HT) Maximum number of records existing in a hash bucket: %d\n", max);
    printf("(HT) Average number of blocks existing in a hash bucket: %d\n", (int) sum_blocks/NUM_OF_BUCKETS);
    printf("(HT) Number of buckets containing at least 1 overflown block: %d\n", buckets_with_overflown_blocks);
    
    for (int i=0; i<NUM_OF_BUCKETS; i++){
        printf("(HT) Bucket '%d': %d overflown blocks\n", i, p_file_info->Blocks_with_overflow[i]);
    }
    printf("\n");
    printf("------------------------------------------------------------------------------------------\n");
    printf("\n");
    return 0;
}