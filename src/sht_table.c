#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "sht_table.h"
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

// string hash function
int shasher(char* name){
    int i;
    int sum = 0;
    char nam[15];
    for (i=0; i<15; i++){
        nam[i]=0;
    } 
    strcpy(nam, name);
    i=0;
    while (nam[i] != 0){
        
      sum = sum + (nam[i] % S_NUM_OF_BUCKETS);     // hashing every letter of the name
      i++;
    }
    sum = sum % S_NUM_OF_BUCKETS;                   // just to be sure that sum>=0 and sum<10
    return sum;                                     // hash arithmetic value of the name
}

//-----------------------------------------------------------------------------

int SHT_CreateSecondaryIndex(char *sfileName,  int buckets, char* fileName){
    
    int sfileDesc2;
    BF_Block_Init(&sblock);
    int blocks_num = buckets;
    printf("Creating your SHT file...\n");
    serror = BF_CreateFile(sfileName);
    if (serror){
        BF_PrintError(serror);
        return -1;
    }
    // acquire the File Descriptor
    serror = BF_OpenFile(sfileName, &sfileDesc2);
    if (serror){
        BF_PrintError(serror);
        return -1;
    }
    // allocate memory for the Block '0' of the File and initialize its fields
    serror = BF_AllocateBlock(sfileDesc2, sblock);
    if (serror){
        BF_PrintError(serror);
        return -1;
    }
    serror = BF_GetBlock(sfileDesc2, 0, sblock);
    if (serror){
        BF_PrintError(serror);
        return -1;
    }

    sdata = BF_Block_GetData(sblock);
    // initialization of the SHT_info block
    sp_file_info = sdata;
    sp_file_info->stype = 'H';
    sp_file_info->sfileDesc = sfileDesc2;
    sp_file_info->snumBuckets = buckets;
    sp_file_info->snum_of_blocks = buckets;
    for (i = 0; i<S_NUM_OF_BUCKETS; i++){

        sp_file_info->sHashTable[i] = i + 1;
        sp_file_info->sRecords_per_Bucket[i] = 0;     // useful for the HashStatistics() function, sRecords are the (SHT_pair pair)
        sp_file_info->sBlocks_per_Bucket[i] = 0;      // useful for the HashStatistics() function
        sp_file_info->sBlocks_with_overflow[i] = 0;   // useful for the HashStatistics() function
    }
    // the other S-hash buckets...
    for (i = 0; i < buckets; i++){

        // create a block for every hash bucket
        serror = BF_AllocateBlock(sp_file_info->sfileDesc, sblock);
        if (serror < 0){
            BF_PrintError(serror);
            return -1;
        }
        // find the id of the Block (which equals its number in the series)
        BF_GetBlockCounter(sp_file_info->sfileDesc, &blocks_num);
        
        // reading the Block from the disc
        serror = BF_GetBlock(sp_file_info->sfileDesc, sp_file_info->sHashTable[i], sblock);
        if (serror){
            BF_PrintError(serror);
            return -1;
        }

        sp_block_info = (SHT_block_info*) BF_Block_GetData(sblock);
        // data initialization for Block
        sp_block_info->snum_of_records = 0;
        sp_block_info->snext_block = -1;

        // copy SHT_block_info struct at the beginning of each block
        memcpy(sp_block_info, &sblock_info, sizeof(sblock_info));
        BF_Block_SetDirty(sblock);
        BF_UnpinBlock(sblock);
    }
    // copy SHT_info struct at the beginning of Block '0'
    BF_Block_SetDirty(sblock);
    BF_UnpinBlock(sblock);
    serror = BF_CloseFile((sp_file_info->sfileDesc));
    if (serror){
        BF_PrintError(serror);
        return -1;
    }
    printf("Finished SHT file creation.\n");
    return 0;

}

//-----------------------------------------------------------------------------

SHT_info* SHT_OpenSecondaryIndex(char *indexName){

    int sfileDesc2;
    serror = BF_OpenFile(indexName, &sfileDesc2);
    if (serror){
        BF_PrintError(serror);
        return NULL;
    }
    serror = BF_GetBlock(sfileDesc2, 0, sblock);
    if (serror){
        BF_PrintError(serror);
        return NULL;
    }
    sdata = BF_Block_GetData(sblock);
    sp_file_info = sdata;
    sp_file_info->sfileDesc = sfileDesc2;
    sp_file_info->snum_of_blocks = S_NUM_OF_BUCKETS;
    return sp_file_info;

}

//-----------------------------------------------------------------------------

int SHT_CloseSecondaryIndex(SHT_info* sp_file_info){

    serror = BF_CloseFile((sp_file_info->sfileDesc)-1);
    if (serror){
        BF_PrintError(serror);
        return -1;
    }
    printf("Preparing to end program...\n");
    printf("Closed SHT file and freed SHT-related memory.\n");
    return 0;
}

//-----------------------------------------------------------------------------

int SHT_SecondaryInsertEntry(SHT_info* sp_file_info, Record record, int block_id){
    
    // find the Hash place for the Record through its 'name'
    int sindex = shasher(record.name);
    printf("---NEW INSERTION in SHT---\n");
    printf("Record with name '%s' (id = %d) goes to bucket '%d'.\n", record.name, record.id, sindex);

    // 'if' to handle unexpected error with file descriptors
    if ((sp_file_info->sfileDesc) == 2){
        (sp_file_info->sfileDesc)--;
    }
    // acquire the right hash block to insert the pair
    serror = BF_GetBlock(sp_file_info->sfileDesc, sp_file_info->sHashTable[sindex], sblock);
    if (serror){
        BF_PrintError(serror);
        return -1;
    }

    // SHT pair initialization
    SHT_pair pair;
    strcpy(pair.name, record.name);
    pair.sblock_id = block_id;

    // data acquisitions
    sdata = BF_Block_GetData(sblock);
    printf("1/ snum_of_records = %d\n", (sp_block_info->snum_of_records));
    sp_block_info = sdata;
    // HERE, after line 191, the value of '(sp_block_info->snum_of_records)' changes unexpectedly
    sdata = sp_block_info + sizeof(SHT_block_info);

    // 'p_pair' points right after the SHT_block_info struct in the block
    p_pair = sdata;                                                
    printf("2/ snum_of_records = %d\n", (sp_block_info->snum_of_records));

    // if Block is not full, insert pair
    if ((sp_block_info->snum_of_records) < S_MAX_NUM_OF_RECORDS){

        sdata = sdata + (sp_block_info->snum_of_records)*sizeof(SHT_pair);     // points after the last pair of the Block
        memcpy(sdata, &pair, sizeof(SHT_pair));                                // copy pair in the Block  
        (sp_block_info->snum_of_records)++;
        (sp_file_info->sRecords_per_Bucket[sindex])++;
        BF_Block_SetDirty(sblock);
        BF_UnpinBlock(sblock);
        printf("---FINISHED INSERTION in SHT: name = '%s' -> 'bucket=%d'---\n", record.name, sindex);
    }
    else{   // if Block is full, create a new block to insert the record

        (sp_file_info->sBlocks_with_overflow[sindex])++;                   // one more block has overflown
        BF_AllocateBlock(sp_file_info->sfileDesc, sblock);                 // allocate new Block
        sp_block_info = (SHT_block_info*) BF_Block_GetData(sblock);                         
        (sp_block_info->snum_of_records) = 0;                              // initialize the HT_block_info
        (sp_block_info->snext_block) = -1;
        (sp_file_info->snum_of_blocks)++;                                  // created one more block
        /* things changed because we added one more block for this Record
           so the block id where the Record potentially and finally goes, is updated*/
        (sp_file_info->sBlocks_per_Bucket[sindex])++;                        // one more block in this hash bucket chain
        (sp_block_info->snext_block) = (sp_file_info->sHashTable[sindex]);   // the new block of the chain points to its previous
        (sp_file_info->sHashTable[sindex]) = (sp_file_info->snum_of_blocks); // now the first block of the chain is the new one
        sdata = sp_block_info + sizeof(SHT_block_info);                      // points to the first AVAILABLE space for a record in the block
        p_pair = sdata;
        memcpy(p_pair, &pair, sizeof(pair));                      // copy Record data in the block
        (sp_block_info->snum_of_records)++;                       // update HT_block_info
        (sp_file_info->sRecords_per_Bucket[sindex])++;
        printf("Inserting the new record with name '%s' in a new block...\n", record.name);  
        BF_Block_SetDirty(sblock);
        BF_UnpinBlock(sblock);
        printf("---FINISHED INSERTION in SHT: name = '%s' -> 'bucket=%d' (chain expansion)---\n", record.name, sindex);

    }
    return 0;     // SUCCESS
}

//-----------------------------------------------------------------------------

int SHT_SecondaryGetAllEntries(HT_info* p_file_info, SHT_info* sp_file_info, char* name){

    // find the bucket of the SHT file where the 'name' is stored
    // then we iterate through the blocks and the chain (if any) to find all 'name' instances
    int sindex = shasher(name);
    int i=0;
    int HT_block_counter = 0;       // blocks we iterated through in the HT file (at least 1 at the beginning)
    int SHT_block_counter = 1;      // blocks we iterated through in the SHT file (at least 1 at the beginning)
    int total_blocks;               // total blocks we iterated through (HT file + SHT file);
    
    // get the first block of the right hash bucket in the SHT file
    serror = BF_GetBlock(sp_file_info->sfileDesc-1, sp_file_info->sHashTable[sindex], sblock);    // first block of the hash bucket
    if (serror){
        BF_PrintError(serror);
        return -1;
    }
    sdata = BF_Block_GetData(sblock);                  // points to the 'HT_block_info' section
    sp_block_info = (SHT_block_info*) sdata;
    sdata = sp_block_info + sizeof(SHT_block_info);    // points to the 1st record in the 1st block of the hash bucket chain
    p_pair = (SHT_pair*) sdata;

    // iterate through every record in every block of the hash bucket
    printf("\n");
    printf("---SEARCHING for pairs with name '%s' in SHT hash bucket '%d'...---\n", name, sindex);

    while (i < S_MAX_NUM_OF_RECORDS){        
        
        p_pair = sdata + i*sizeof(SHT_pair);
        if (strcmp(p_pair->name, name) == 0){ 

            // found one instance of 'name' in the SHT file, we will move on to the next later
            printf("The name '%s' corresponds to HT block '%d'.\n", name, p_pair->sblock_id);
            // get the specific block of the HT file
            error = BF_GetBlock(p_file_info->fileDesc, p_pair->sblock_id, block);
            if (error){
                BF_PrintError(error);
                return -1;
            }
            data = BF_Block_GetData(block);                 // points to the 'HT_block_info' section of the specific block
            p_block_info = (HT_block_info*) data;
            data = p_block_info + sizeof(HT_block_info);    // points to the 1st record in the specific block of the HT file
            p_record = (Record*) data;
            HT_block_counter++;
            int j=0;
            while (j < (p_block_info->num_of_records)){

                p_record = p_record + 1;
                if (strcmp(p_record->name, name) == 0){
                    //found the 'name' in the HT file, in the specific block
                    printf("Found name '%s' at SHT bucket '%d' -> Full record is at HT file/block '%d' and is:\n", name, sindex, p_pair->sblock_id);
                    printRecord(*p_record);
                    printf("\n");
                }
                j++;
                
            }
            // finished with the whole specific HT block
        }
        // going for the next pair in the SHT hash bucket chain
        i++;
        // going for the next pair in the SHT block
        p_pair = p_pair + 1;
        if (sp_block_info->snum_of_records > S_MAX_NUM_OF_RECORDS){   // if we are beyond the end of the block, so go to the next one in the chain
            
            if (sp_block_info->snext_block != -1){  
                
                // means there is another block in the hash chain
                printf("Hopping to next block in the SHT hash bucket chain...\n");
                // one more SHT block to search in the chain
                SHT_block_counter++;
                serror = BF_GetBlock(sp_file_info->sfileDesc-1, sp_block_info->snext_block, sblock);    // get pointer to next block of hash chain
                if (serror){
                    BF_PrintError(serror);
                    return -1;
                }
                sdata = BF_Block_GetData(sblock);                             
                sp_block_info = (SHT_block_info*) sdata;           // points to the 'HT_block_info' section
                sdata = sp_block_info + sizeof(SHT_block_info);
                p_pair = (SHT_pair*) sdata;                      // points to the 1st record in the block
                i = 0;
            }
            else{break;}        // means we have no more blocks in the hash chain
        }
    }

    total_blocks = SHT_block_counter + HT_block_counter;
    printf("Iterated through %d SHT block(s) to find all pairs with name '%s'.\n", SHT_block_counter, name);
    printf("Iterated through %d HT block(s) to find all Records with SHT pair instance (%s, %d).\n", HT_block_counter, name, p_pair->sblock_id);
    printf("Iterated through %d block(s) in total (SHT + HT) to find all the Records.\n", total_blocks);

    return total_blocks;

}

int sHashStatistics(char* sfilename){

    (SHT_info*) sp_file_info;
    sp_file_info = SHT_OpenSecondaryIndex(sfilename);      // acquiring the file info
    if (sp_file_info == NULL){
        return -1;
    }
    printf("\n");
    printf("---Secondary HashTable HASH STATISTICS---\n");
    printf("\n");
    printf("Total number of blocks in the file: %d\n", sp_file_info->snum_of_blocks);
    int min, avg, max;
    int buckets_with_overflown_blocks = 0;
    int sum_records = 0;
    int sum_blocks = 0;
    for (int i=0; i<S_NUM_OF_BUCKETS; i++){

        sum_records = sum_records + sp_file_info->sRecords_per_Bucket[i];  
        sum_blocks = sum_blocks + sp_file_info->sBlocks_with_overflow[i];

        if ((sp_file_info->sBlocks_with_overflow[i]) > 0){
            buckets_with_overflown_blocks++;
        }
        if (i==0){
            max = sp_file_info->sRecords_per_Bucket[i];
            min = sp_file_info->sRecords_per_Bucket[i];
        }
        else{
            if (max < sp_file_info->sRecords_per_Bucket[i]){
                max = sp_file_info->sRecords_per_Bucket[i];       // upgrade max Records per bucket
            }
            if (min > sp_file_info->sRecords_per_Bucket[i]){
                min = sp_file_info->sRecords_per_Bucket[i];       // upgrade min Records per bucket
            }
        }
    }
    printf("(SHT) Minimum number of records existing in a hash bucket: %d\n", min);
    printf("(SHT) Average number of records existing in a hash bucket: %d (WRONG -> see README)\n", (int) sum_records/S_NUM_OF_BUCKETS);
    printf("(SHT) Maximum number of records existing in a hash bucket: %d (WRONG -> see README)\n", max);
    printf("(SHT) Average number of blocks existing in a hash bucket: %d (WRONG -> see README)\n", (int) sum_blocks/S_NUM_OF_BUCKETS);
    printf("(SHT) Number of buckets containing at least 1 overflown block: %d\n", buckets_with_overflown_blocks);
    
    for (int i=0; i<S_NUM_OF_BUCKETS; i++){
        printf("(SHT) Bucket '%d': %d overflown blocks\n", i, sp_file_info->sBlocks_with_overflow[i]);
    }
    printf("\n");
    printf("------------------------------------------------------------------------------------------\n");
    printf("\n");
    return 0;
}


