Each part of Tasks 1 & 2 is described separately below:


--------------------------------------------------
| HP Functions |
--------------------------------------------------

DESCRIPTION OF FILES
-----------------

The files of the work we were asked to process are four (4):

1) hp_file.h -> Contains declarations of structs that give information about the whole file (HP_info) and each block
separately (HP_block_info). It also includes variable declarations used in the main
file hp_file.c for various operations and data acquisitions. Finally, in addition to those given by
pronouncing function prototypes, one more is declared that concerns finding and printing the statistics
items requested for HP Functions.

2) hp_main.c -> Task 1 code file for HP Functions, which executes various function calls and
operations on a set of Records. It is the testing code of the file hp_file.c. Beyond those lines
code were given ready with pronunciation, some printouts were added for better visualization of results
in the terminal and visual separation of each testing section.
 

3) Makefile and Run (the commands to compile and run the code are given at the end of the section).

4) hp_file.c -> It is the main file to implement. It includes the implementation of various HP database functions
which are declared in hp_file.h. The number of records can obviously be changed
by the user who owns the source code. In the functions HP_CreateFile(), HP_OpenFile() and
HP_CloseFile() performs basic database file creation, opening, and closing operations.
The various stages of execution of the operations are described both with detailed comments in the code and with prints
in the terminal, so that the user can monitor the execution of the program. Also, with official comments
are also the initializations which are used at the end of Task 1 HP, for the calculation and printing of
of HP approach stats (except that there are no hash buckets here, so some stats are missing
meaning).
In the HP_InsertEntry() implementation section, there are prints that visually separate each Record entry in the terminal, with
marking the START and END of entering the Record into the database. Also, in each call of the BF functions, an error check is performed
so that the program terminates in case of a critical error. The logic of importing Records is simple: Records are imported serially
in each block until it is full (we start from block 1). When a block is full, we create another one and start saving
the next records in it now etc etc.
The oldest overflown block is at the BEGINNING of the file, while the newest is at the END. If the block we are in has not overflowed
and we can enter the Record, then we go to the appropriate memory location of the block and save it. In every
block, the HP_block_info struct is at the beginning of it, followed by the Records.
Schematically, the blocks are as follows:

 
		..... - [HP_block_info][Record id A][Record id B]...[Record id F]-(linkage)-[HP_block_info][Record id K][Record id M]...[Record id S]-(linkage)-.....
 
 
In parallel with the implementation of the Entry function in the base, elements that are later used are kept in some parts of the code
in deriving statistical results for the HP approach.


Makefile and code execution
----------------------------

With the <rm data.db> command, the database that has been created is deleted and we can re-execute the program if
we want to try different parameters (eg different number of buckets, different number of records etc.). The data.db file
we can find it in the same folder as our Makefile. If we don't delete data.db first, then we will get an error that the file
already used, by BF library.

The <make hp> command compiles the heap source files.

The <./build/hp_main> command executes the heap-related code.



--------------------------------------------------
| HT Functions |
--------------------------------------------------

DESCRIPTION OF FILES
-----------------

The files of the work we were asked to process are four (4):

1) ht_table.h -> Contains declarations of structs that give information about the whole file (HT_info) and each block
separately (HT_block_info). It also includes variable declarations used in the main
ht_table.c file for various operations and data acquisitions. Finally, in addition to those given by
pronouncing function prototypes, one more is declared that concerns finding and printing the statistics
items requested for HT Functions.

2) ht_main.c -> Task 1 code file for HT Functions, which executes various function calls and
operations on a set of Records. It is the testing code of the file ht_table.c. Beyond those lines
code were given ready with pronunciation, some printouts were added for better visualization of results
in the terminal and visual separation of each testing section.
 

3) Makefile and Run (the commands to compile and run the code are given at the end of the section).

4) ht_table.c -> It is the main file to implement. It includes the implementation of the various functions of the HT database
which are declared in ht_table.h. Initially, a slightly more complex and pseudo-random hashing function was chosen
which hashes Records ids into 15 buckets [hasher()]. The number of buckets can obviously be changed
by the user who owns the source code. In the functions HT_CreateFile(), HT_OpenFile() and
HT_CloseFile() the basic operations of creating, opening and closing the database file are performed.
The various stages of execution of the operations are described both with detailed comments in the code and with prints
in the terminal, so that the user can monitor the execution of the program. Also, with official comments
are also the initializations which are used at the end of Task 1 HT, for the calculation and printing of
statistics of the HT approach.
In the implementation section of HT_InsertEntry(), there are prints that visually separate each Record entry in the terminal, with
marking the START and END of entering the Record into the database. Also, in each call of the BF functions, an error check is performed
so that the program terminates in case of a critical error. The logic of inserting Records into the corresponding hash bucket
is as follows: we find the hash value of the Record id, then we go to the corresponding index of the HashTable[] (stored in
file info section), and from there we retrieve the block in which the particular Record must be registered. When a block fills up,
we perform hash chaining, i.e. we allocate a new block within the same hash bucket. Then the new block becomes the starting point
of the chain, i.e. the block id of the new block is now stored in the corresponding index of the HashTable[] and after that the rest follow
blocks. The oldest overflown block is at the end of the chain, while the newest at the beginning with its block id stored in
corresponding position of HashTable[]. Linking in hash chaining is achieved by storing the next_block field in the HT_block_info struct,
which stores the block id (18th, 35th or 5th etc of the TOTAL file) of the next one in the chain. If the block in which
we are not overflowed and we can insert the Record, then we go to the appropriate memory location and store it. In every
block (regardless of bucket), the HT_block_info struct is at the beginning of it, followed by the Records, from smallest id to largest.
This also results from the way the ids are generated as is done by the given function randomRecord() [new_id = old_id + 1]. Schematically, in one
bucket where there is a hash chain, the blocks look something like this:
 
		BUCKET #X: [HT_block_info][Record id 5][Record id 12]...[Record id 30]-(linkage)-[HT_block_info][Record id 37][Record id 44]...[Record id 65 ]-(linkage)-.....
 
In parallel with the implementation of the Entry function in the base, elements that are later used are kept in some parts of the code
in deriving statistical results for the HT approach. Some arrays or variables also serve this purpose (eg int Blocks_per_Bucket[])
which are declared in struct HT_info (info of the total file).



Makefile and code execution
----------------------------

With the <rm data.db> command, the database that has been created is deleted and we can re-execute the program if
we want to try different parameters (eg different number of buckets, different number of records etc.). The data.db file
we can find it in the same folder as our Makefile. If we don't delete data.db first, then we will get an error that the file
already used, by BF library.

With the <make ht> command, the source files related to the simple hash tables are compiled.

The command <./build/ht_main> executes the code related to simple hash tables.

-------------------------------------------------------------------------------------------------------------------------------------------

--------------------------------------------------
| HT-SHT Functions |
--------------------------------------------------

DESCRIPTION OF FILES
-----------------

The files of the work we were asked to process are four (4):

1) sht_table.h -> Contains declarations of structs that give information about the whole file (SHT_info) and each block
separately (SHT_block_info). It also includes variable declarations used in the main
sht_table.c file for various operations and data acquisitions. Finally, in addition to those given by
calling function prototypes. The sHashStatistics() function shows partially correct results
for the reasons that will be mentioned later.

2) sht_main.c -> Task 2 code file for SHT & HT Functions, which executes various function calls and
operations on a set of Records. It is the testing code of the sht_table.c file. Beyond those lines
code were given ready with pronunciation, some printouts were added for better visualization of results
in the terminal and visual separation of each testing section.
 
3) Makefile and Run (the commands to compile and run the code are given at the end of the section).

4) sht_table.c -> It is the main file to implement. It includes the implementation of the various functions of the SHT database
which are declared in sht_table.h. The hash function shasher() produces the hash value from name,
processing each character of the name string serially. The number of buckets can obviously be changed
by the user who owns the source code. In the functions SHT_CreateSecondaryIndex(), SHT_OpenSecondaryIndex() and
SHT_CloseSecondaryIndex() the basic operations of creating, opening and closing the database file are performed.
The various stages of execution of the operations are described both with detailed comments in the code and with prints
in the terminal, so that the user can monitor the execution of the program. Also, with official comments
are also the initializations that are used at the end of Task 2 SHT, to calculate and print the
statistics of the SHT approach. It should be emphasized that line 173 of sht_table.c is checked
for the value of the file descriptor because during the initial execution of the code, the value of fd changed inexplicably (still
and whether the correct initialization of the sfileDesc field in the SHT_CreateFile() and SHT_OpenFile() functions was followed. As long as
I searched the internet, this is related to more than how many files are active, even to the fact that the I/O channels or the terminal
obtain file descriptors from the operating system. In any case, the particular value handle ensures that
we are referring to the correct file descriptor.
Here, what is inserted into index.db are not entire records, but pairs of the form (name, HT block_id), where HT block_id is
the number of the block where the record was stored in the HT FILE (data.db).
In the implementation section of SHT_InsertEntry(), there are prints that visually separate in the terminal each input pair, with
marking the START and END of inserting the pair into the database. Also, in each call of the BF functions, an error check is performed
so that the program terminates in case of a critical error. The logic of inserting the pairs into the corresponding SHT hash bucket
is as follows: first sht_main.c calls the HT_InsertEntry function, from which we get the input block_id.
Then we find the hash value of the Record name inside the SHT_SecondaryInsertEntry, and then we go to the corresponding index of
sHashTable[] (stored in file info section SHT_info). From there we retrieve the SHT block in which the specific name must be registered.
Thus, we insert in this block the pair (name, HT block_id).
When a block is full, we perform hash chaining, i.e. we allocate a new block within the same hash bucket. Subsequently,
the new block becomes the starting point of the chain, i.e. the block id of the new block is now stored in the corresponding index of the sHashTable[]
and after that the rest of the blocks follow. The oldest overflown block is at the end of the chain, while the newest is at the beginning
with its block id stored in the corresponding position of the sHashTable[]. Linking in hash chaining is achieved by storing in struct
SHT_block_info the next_block field, which stores the block id (18th, 35th or 5th etc of the TOTAL file) of its immediate next in
chain. If the block we are in has not overflowed and we can insert the pair, then we move to the appropriate position
memory and save it. In each block (regardless of bucket), the SHT_block_info struct is at the beginning of it, followed by the pairs,
in chronological order of introduction.
Schematically, in a bucket where there is a hash chain, the blocks look like this:
 
		BUCKET #X: [SHT_block_info][name X, block_id Y][name K, block_id T]...[name N, block_id D]-(linkage)-[SHT_block_info][name X, block_id R][name A, block_id B]...[name J, block_id W]-(linkage)-.....
 
In parallel with the implementation of the Entry function in the base, elements that are later used are kept in some parts of the code
in deriving statistical results for the SHT approach. Some arrays or variables also serve this purpose (eg int sBlocks_per_Bucket[])
which are declared in struct SHT_info (info of the global file index.db).
 
(!!!!) While running and debugging the sht_table.c code the following was observed: in lines 190 and 197 some print tests of the value have been inserted
which has the variable '(sp_block_info->snum_of_records)' (= the number of pairs stored at any time in a SHT block). I noticed
that when entering the FIRST pair, this value changes unexpectedly from line 190 to 197. This, with print tests, happens exactly on line 191.
Specifically, from the value 0 that (sp_block_info->snum_of_records)' has (correctly) for the FIRST input pair, it suddenly becomes 1. So also in line 202
then this pair is not stored immediately after SHT_block_info, but an empty pair is interposed and then stored:
 
		BLOCK X: | SHT_block_info | ! | pair | pair | pair |.....

Trying to give a correct value to the offset (=0) to write to the correct point, I was getting an error from the BF library (BF ERROR something unexpected occurred).
The insertion of pair is indeed done, just in a "more right" position than expected. After that, in every subsequent storage of another pair, either in the same block
or else (even empty with theoretically '(sp_block_info->snum_of_records) = 0'), the program enters the SHT_SecondaryInsertEntry() function with
"locked" value '(sp_block_info->snum_of_records) = 1'), which if changed inside the function causes errors with file descriptors (!!!), since
messages were displayed 'BF error The file has not been opened. It returns on the next call again with a value of 1 (!).
This results in each new pair in each block (empty or with a pair already inside) being inserted in the same position and overwriting another pair in
that position! Eventually, due to this 'lock', the only record that can be found in SHT_SecondaryGetAllEntries() is the last record (pair)
in the SHT case. Indeed, I have introduced a 2nd test case in SHT_SecondaryGetAllEntries(): given a random Record name to find (Vagelis), 
we also enter the search for the last name entered (in this case with 60 records, it is 'Giorgos').
And indeed, the code gets it right in both SHT and HT afterwards:
 
		Inserting Entries...

		---NEW INSERTION in HT---
		Record with id '1' goes to bucket '7'.
		####### 8
		---FINISHED INSERTION in HT: 'id=1' -> 'bucket=7'---
		---NEW INSERTION in SHT---
		Record with name 'Maria' (id = 1) goes to bucket '10'.
		------> 1/ snum_of_records = 0
		------> 2/ snum_of_records = 1
		---FINISHED INSERTION in SHT: name = 'Maria' -> 'bucket=10'---
		---NEW INSERTION in HT---
		Record with id '2' goes to bucket '8'.
		####### 9
		---FINISHED INSERTION in HT: 'id=2' -> 'bucket=8'---
 
 
-Case 'Vagelis' (entered with some series X and shows errors):
 
		RUN GetAllEntries for name Vagelis


		---SEARCHING for pairs with name 'Vagelis' in SHT hash bucket '10'...---
		Iterated through 1 SHT block(s) to find all pairs with name 'Vagelis'.
		Iterated through 0 HT block(s) to find all Records with SHT pair instance (Vagelis, 0).
		Iterated through 1 block(s) in total (SHT + HT) to find all the Records.
		-------------------------------------------------- ---------------------------------------
		 
		-Case 'Giorgos' (inserted last and actually found after the record):
		 
		RUN GetAllEntries for name Giorgos


		---SEARCHING for pairs with name 'Giorgos' in SHT hash bucket '10'...---
		The name 'Giorgos' corresponds to HT block '4'.
		Found name 'Giorgos' at SHT bucket '10' -> Full record is at HT file/block '4' and is:
		(60, Giorgos, Michas, Miami)

		Iterated through 1 SHT block(s) to find all pairs with name 'Giorgos'.
		Iterated through 1 HT block(s) to find all Records with SHT pair instance (Giorgos, 0). <-----here '0' is false and ALSO 'locked' (!?) value of block_id
		without affecting the correct finding of the record
		Iterated through 2 block(s) in total (SHT + HT) to find all the Records.
 
 
 
 
Because of this problem, sHashStatistics() returns some results correctly and some incorrectly. But with many different approaches to
manipulate the problem and overcome it (e.g. with an external table whose each cell will be a bucket and store the number of its pairs), I did not succeed
to find why this is happening on an 'invalid' line of code. I don't have any variable conflict, because I searched a lot. Even with valgrind I tried to find some leak,
but nothing, beyond the standard leaks due to the BF library.
The above can be seen with the prints I have put in the appropriate places, and in the terminal output. If this 'lock' is overcome, then everything get inserted
and work properly, after some other tests I did (but it is not possible to include them all here).

		|
		|
		V
		BLOCK X: | SHT_block_info | ! | pair |
 

Makefile and code execution
----------------------------

With the commands <rm data.db>, <rm index.db> the record base that has been created is deleted and we can re-execute the program if
we want to try different parameters (eg different number of buckets, different number of records etc.). The data.db and index.db files
we can find it in the same folder as our Makefile. If we don't delete data.db first, then we will get an error that the file
already used, by BF library.

The <make sht> command compiles the source files related to the SHT-HT implementation.

The command <./build/sht_main> executes the code related to the SHT-HT implementation.

