#ifndef HP_FILE_H
#define HP_FILE_H
#define MAX_RECORDS ((BF_BLOCK_SIZE - sizeof(HP_block_info))/sizeof(Record)) // per block
#include <record.h>

//-----------------------------------------------------------------------------

typedef struct {                                // File meta-data
    int fileDesc;                               // File Descriptor
    int num_of_blocks;
    char type;                                  // file type (H)
} HP_info;

typedef struct {
    int num_of_records;             // number of records in the Block
    int next_block;                 // index to the next Block    
} HP_block_info;

int error, i;
void* data;
HP_block_info* hpp_block_info;
HP_info* hpp_file_info;
HP_info hpfile_info;
HP_block_info hpblock_info;
BF_Block* block;
BF_Block* block_0;                  // the first block of the file storing all the file info
Record* p_record;

//-----------------------------------------------------------------------------

/*Η συνάρτηση HP_CreateFile χρησιμοποιείται για τη δημιουργία και
κατάλληλη αρχικοποίηση ενός άδειου αρχείου σωρού με όνομα fileName.
Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται 0, ενώ σε
διαφορετική περίπτωση -1.*/
int HP_CreateFile(
    char *fileName /*όνομα αρχείου*/);

/* Η συνάρτηση HP_OpenFile ανοίγει το αρχείο με όνομα filename και
διαβάζει από το πρώτο μπλοκ την πληροφορία που αφορά το αρχείο σωρού.
Κατόπιν, ενημερώνεται μια δομή που κρατάτε όσες πληροφορίες κρίνονται
αναγκαίες για το αρχείο αυτό προκειμένου να μπορείτε να επεξεργαστείτε
στη συνέχεια τις εγγραφές του.
*/
HP_info* HP_OpenFile( char *fileName /* όνομα αρχείου */ );



/* Η συνάρτηση HP_CloseFile κλείνει το αρχείο που προσδιορίζεται
μέσα στη δομή header_info. Σε περίπτωση που εκτελεστεί επιτυχώς,
επιστρέφεται 0, ενώ σε διαφορετική περίπτωση -1. Η συνάρτηση είναι
υπεύθυνη και για την αποδέσμευση της μνήμης που καταλαμβάνει η δομή
που περάστηκε ως παράμετρος, στην περίπτωση που το κλείσιμο
πραγματοποιήθηκε επιτυχώς.
*/
int HP_CloseFile( HP_info* header_info );

/* Η συνάρτηση HP_InsertEntry χρησιμοποιείται για την εισαγωγή μιας
εγγραφής στο αρχείο σωρού. Οι πληροφορίες που αφορούν το αρχείο
βρίσκονται στη δομή header_info, ενώ η εγγραφή προς εισαγωγή
προσδιορίζεται από τη δομή record. Σε περίπτωση που εκτελεστεί
επιτυχώς, επιστρέφετε τον αριθμό του block στο οποίο έγινε η εισαγωγή
(blockId) , ενώ σε διαφορετική περίπτωση -1.
*/
int HP_InsertEntry(
    HP_info* header_info, /* επικεφαλίδα του αρχείου*/
    Record record /* δομή που προσδιορίζει την εγγραφή */ );

/*Η συνάρτηση αυτή χρησιμοποιείται για την εκτύπωση όλων των εγγραφών
που υπάρχουν στο αρχείο κατακερματισμού οι οποίες έχουν τιμή στο
πεδίο-κλειδί ίση με value. Η πρώτη δομή δίνει πληροφορία για το αρχείο
κατακερματισμού, όπως αυτή είχε επιστραφεί από την HP_OpenFile.
Για κάθε εγγραφή που υπάρχει στο αρχείο και έχει τιμή στο πεδίο id
ίση με value, εκτυπώνονται τα περιεχόμενά της (συμπεριλαμβανομένου
και του πεδίου-κλειδιού). Να επιστρέφεται επίσης το πλήθος των blocks που
διαβάστηκαν μέχρι να βρεθούν όλες οι εγγραφές. Σε περίπτωση επιτυχίας
επιστρέφει το πλήθος των blocks που διαβάστηκαν, ενώ σε περίπτωση λάθους επιστρέφει -1.
*/
int HP_GetAllEntries(
    HP_info* header_info, /* επικεφαλίδα του αρχείου*/
    int id /* η τιμή id της εγγραφής στην οποία πραγματοποιείται η αναζήτηση*/);

int hpHashStatistics(char* filename);
/* function prints:
1) number of blocks in the file
2) minimum, average and maximum number of records the blocks contain
function returns '0' for success, '-1' for failure
*/
#endif // HP_FILE_H
