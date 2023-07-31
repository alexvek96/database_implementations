#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "hp_file.h"
#include "record.h"

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

//-----------------------------------------------------------------------------

int HP_CreateFile(char *fileName){

    int fileDesc2;
    BF_Block_Init(&block);
    printf("Creating your HP file...\n");
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
    // initialization of the HP_info block
    hpp_file_info = data;
    hpp_file_info->type = 'H';                        
    hpp_file_info->fileDesc = fileDesc2;
    hpp_file_info->num_of_blocks = 1;

    // create the 1st block...if we need more we will create them
    int blocks_num;
    for (i = 0; i<(hpp_file_info->num_of_blocks); i++){

      // creation...
      error = BF_AllocateBlock(hpp_file_info->fileDesc, block);
      if (error < 0){
          BF_PrintError(error);
          return -1;
      }
      // find the id of the Block (which equals its number in the series)
      BF_GetBlockCounter(hpp_file_info->fileDesc, &blocks_num);
        
      // reading the Block from the disc
      error = BF_GetBlock(hpp_file_info->fileDesc, hpp_file_info->num_of_blocks, block);
      if (error){
          BF_PrintError(error);
          return -1;
      }

      hpp_block_info = (HP_block_info*) BF_Block_GetData(block);
      // data initialization for Block
      hpblock_info.num_of_records = 0;
      hpblock_info.next_block = -1;    // no block afterwards, at the moment...
      
      // copy HP_block_info struct at the beginning of each block
      memcpy(hpp_block_info, &hpblock_info, sizeof(hpblock_info));
      BF_Block_SetDirty(block);
      BF_UnpinBlock(block);
    }

    // copy HP_info struct at the beginning of Block '0'
    BF_Block_SetDirty(block);
    BF_UnpinBlock(block);
    error = BF_CloseFile(hpp_file_info->fileDesc);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    printf("Finished HP file creation.\n");
    return 0;
}

//-----------------------------------------------------------------------------

HP_info* HP_OpenFile(char *fileName){
    
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
    hpp_file_info = data;
    hpp_file_info->fileDesc = fileDesc2;
    return hpp_file_info;
}

//-----------------------------------------------------------------------------

int HP_CloseFile( HP_info* hp_info ){

    error = BF_CloseFile(hpp_file_info->fileDesc);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    // free the struct we passed to the function
    free(hpp_file_info);
    printf("Preparing to end program...\n");
    printf("Closed HP file and freed HP-related memory.\n");
    return 0;
}

//-----------------------------------------------------------------------------

int HP_InsertEntry(HP_info* hp_info, Record record){
    
    printf("---NEW INSERTION in HP---\n");
    error = BF_GetBlock(hpp_file_info->fileDesc, hpp_file_info->num_of_blocks, block);
    if (error){
        BF_PrintError(error);
        return -1;
    }
    data = BF_Block_GetData(block);
    hpp_block_info = data;
    data = hpp_block_info + sizeof(HP_block_info);     
    p_record = data;                                   // points right after the HP_block_info struct in the block
    
    // if Block is not full, insert record
    if ((hpp_block_info->num_of_records) < MAX_RECORDS){

        p_record = data + (hpp_block_info->num_of_records)*sizeof(record);  // point after the last Record of the Block
        memcpy(p_record, &record, sizeof(record));                          // copy Record data in the Block  
        (hpp_block_info->num_of_records)++;                                 // update HP_block_info
        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);
        printf("---FINISHED INSERTION in HP: 'id=%d' -> 'block=%d'---\n", record.id, (hpp_file_info->num_of_blocks));
    }
    else{   // if Block is full, create a new block to insert the record

        hpp_block_info->next_block = (hpp_file_info->num_of_blocks) + 1;
        BF_AllocateBlock(hpp_file_info->fileDesc, block);                 // allocate new Block
        printf("Created a new block.\n");
        hpp_block_info = (HP_block_info*) BF_Block_GetData(block);                         
        (hpp_block_info->num_of_records) = 0;                             // initialize the HP_block_info
        (hpp_block_info->next_block) = -1;
        (hpp_file_info->num_of_blocks)++;                                 // created one more block        
        
        data = hpp_block_info + sizeof(HP_block_info);                    // points to the first AVAILABLE space for a record in the block
        p_record = data;
        memcpy(p_record, &record, sizeof(record));                        // copy Record data in the block
        (hpp_block_info->num_of_records)++;                               // update HP_block_info
        
        printf("Inserting the new record with id '%d' in a new block...\n", p_record->id);  
        BF_Block_SetDirty(block);
        BF_UnpinBlock(block);
        printf("---FINISHED INSERTION in HP: 'id=%d' -> 'block=%d' (chain expansion)---\n", record.id, (hpp_file_info->num_of_blocks));
    }
    return (hpp_file_info->num_of_blocks);
}

//-----------------------------------------------------------------------------

int HP_GetAllEntries(HP_info* hp_info, int value){
   
    // we iterate through all blocks of the file to find all
    // possible entries with id='value'
    int i = 0;
    int blockCounter = 1;         // counts the number of blocks we iterate through (at least 1)
    int found = 0;                // flag-> if '1' then we found something, if '0' we didn't find anything 
    int first_block = 1;

    error = BF_GetBlock(hpp_file_info->fileDesc, first_block, block);    // first block of the file
    if (error){
        BF_PrintError(error);
        return -1;
    }

    data = BF_Block_GetData(block);                 // points to the 'HP_block_info' section
    hpp_block_info = (HP_block_info*) data;
    data = hpp_block_info + sizeof(HP_block_info);  // points to the 1st record in the 1st block of the file
    p_record = (Record*) data;
    // iterate through every record in every block of the file
    printf("Searching for records with value (id) '%d' in the file block '%d'...\n", value, blockCounter);

    while (i < hpp_block_info->num_of_records){

        if ((p_record->id) == value){
            
            printf("Found value '%d' in block '%d'. It corresponds to Record: \n", value, blockCounter);
            printRecord(*p_record);
            found = 1;
            i++;
            if (i != (hpp_block_info->num_of_records-1)){
                p_record = p_record + sizeof(Record);               // go to next Record inside the block
            }
        }
        else if (i == (hpp_block_info->num_of_records-1)){          // we are at the last Record of the Block
            
            if (hpp_block_info->next_block != -1){                  // means there is another block in the hash chain
                
                printf("Hopping to block '%d' of the file...\n", hpp_block_info->next_block);
                error = BF_GetBlock(hpp_file_info->fileDesc, hpp_block_info->next_block, block);    // get pointer to next block of the file
                data = BF_Block_GetData(block);                             
                hpp_block_info = (HP_block_info*) data;           // points to the 'HP_block_info' section
                data = hpp_block_info + sizeof(HP_block_info);
                p_record = (Record*) data;                        // points to the 1st record in the block
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
    printf("Iterated through %d block(s) to find all records with Record.id = %d'.\n", blockCounter, value);
    
    if (found==0){
        printf("The given 'value' does not exist in the file.\n");
    }
    printf("------------------------------------------------------------------------------------------\n");
    printf("\n");
    return blockCounter;
}

//-----------------------------------------------------------------------------

int hpHashStatistics(char* filename){
    
    (HP_info*) hpp_file_info;
    hpp_file_info = HP_OpenFile (filename);      // acquiring the file info
    if (hpp_file_info == NULL){
        return -1;
    }
    printf("\n");
    printf("---Heap File STATISTICS---\n");
    printf("\n");
    printf("Total number of blocks in the file: %d\n", hpp_file_info->num_of_blocks);
    
    int min, avg, max;
    int sum_records = 0;
    int i;
    for (i=1; i<=(hpp_file_info->num_of_blocks); i++){
      
      // for every block of the file, acquire the HP_block_info section
      error = BF_GetBlock(hpp_file_info->fileDesc, i, block);
      if (error){
          BF_PrintError(error);
          return -1;
      }
      data = BF_Block_GetData(block);                 
      hpp_block_info = (HP_block_info*) data;                       // points to the 'HP_block_info' section
      sum_records = sum_records + hpp_block_info->num_of_records;  

      if (i==1){
          max = hpp_block_info->num_of_records;
          min = hpp_block_info->num_of_records;
      }
      else{
          if (max < hpp_block_info->num_of_records){
              max = hpp_block_info->num_of_records;       // upgrade max Records per block
          }
          if (min > hpp_block_info->num_of_records){
              min = hpp_block_info->num_of_records;       // upgrade min Records per block
          }
      }
    }
    printf("(HP) Minimum number of records existing in a block: %d\n", min);
    printf("(HP) Average number of records existing in a block: %d\n", (int) sum_records/(hpp_file_info->num_of_blocks));
    printf("(HP) Maximum number of records existing in a block: %d\n", max);
    
    return 0;     // SUCCESS
}