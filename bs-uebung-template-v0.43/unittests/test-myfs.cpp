//
//  test-myfs.cpp
//  testing
//
//  Created by Oliver Waldhorst on 15.12.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include "catch.hpp"

#include "helper.hpp"
#include "myfs.h"
#define BD1_PATH "/tmp/bd1.bin"
#define BD2_PATH "/tmp/bd2.bin"

TEST_CASE("MyFS-Methods","[methods]") {
	//BEFORE SETUP
	remove(BD1_PATH);
	BlockDevice* bd = new BlockDevice(FILESYSTEM_SIZE);
	bd->create(BD1_PATH);
	MyFS::Instance()->initBlockDevice(bd);

	//Test createSuperBlock();
	SECTION("Test createSuperBlock()"){
		char testBlock[BLOCK_SIZE] = {0};
		MyFS::Instance()->createSuperBlock(bd);		//SuperBlock erstellen und in BlockDevice schreiben
		bd->read(SUPER_BLOCK_START,testBlock);			//SuperBlock aus BlockDevice lesen und in block2 schreiben
		super_block* sb = (super_block*) testBlock;		//SuperBlock Pointer erstellen und auf testblock setzen

		//Vergleiche Superblock Inhalt aus dem BlockDevice mit SollWerten
		REQUIRE(sb->blockSize == BLOCK_SIZE);
		REQUIRE(sb->filesystemSize == FILESYSTEM_SIZE);
		REQUIRE(sb->numbFiles == 0);
		REQUIRE(sb->first_imap_block == INDOE_MAP_START);
		REQUIRE(sb->first_dmap_block == DATA_MAP_START);
		REQUIRE(sb->first_fat_block == FAT_START);
		REQUIRE(sb->first_inode_block == INODE_START);
		REQUIRE(sb->first_data_block == DATA_START);

		//Test superBlockNumFilesIncrease()/superBlockNumFilesDecrease()
		SECTION("Test Increase/Decrease File-Number in SuperBlock") {
			//Lade SuperBlock aus BlockDevice in testBlock und prüfe numbFiles die anzahl der Dateien
			for(int i = 0; i < NUM_DIR_ENTRIES; i++) {
				bd->read(SUPER_BLOCK_START,testBlock);
				REQUIRE(sb->numbFiles == i);
				MyFS::Instance()->superBlockNumFilesIncrease(bd);
			}
			for(int i = NUM_DIR_ENTRIES; i > 0; i--) {
				bd->read(SUPER_BLOCK_START,testBlock);
				REQUIRE(sb->numbFiles == i);
				MyFS::Instance()->superBlockNumFilesDecrease(bd);
			}
			bd->read(SUPER_BLOCK_START,testBlock);
			REQUIRE(sb->numbFiles == 0);
		}

		//Test createInodeBlock()
		SECTION("Test createInodeBlock()") {
			//Benötigte Parameter erstellen
			char filenames[NUM_DIR_ENTRIES][NAME_LENGTH];
			FILE* fd;
			uint32_t fileSize;
			struct stat data;
			u_int32_t dataPointers;
			uint32_t  iNodePointer;
			u_int32_t blocksToWrite;
			for (int i = 0; i < NUM_DIR_ENTRIES; i++) {			//Alle 64 Inodes ausfüllen
				filenames[i][NAME_LENGTH-1] = '\0';
				gen_random(filenames[i], NAME_LENGTH-1);		//Zufälligen Dateinamen erzeugen
				fd = fopen(filenames[i],"w");					//Datei erstellen
				fileSize = rand() % (524288);
				char filecontent[fileSize] = {0};
				gen_random(filecontent,fileSize);				//Zufälligen Inhalt erstellen in "filecontent"
				fprintf(fd,"%s",filecontent);					//"filecontent" mit zufälligen Inhalt in die Datei schreiben.
				fclose(fd);
				stat(filenames[i],&data);						//Schreibe Metadaten der zufälligen Datei für späteren Vergleich in "data".
				blocksToWrite = MyFS::Instance()->getMaxBlocksNeeded(data.st_size);
				u_int32_t dataPointers[blocksToWrite];
				MyFS::Instance()->getFreeDataPointers(bd,dataPointers,blocksToWrite,0);
				MyFS::Instance()->writeFatEntries(bd,dataPointers,blocksToWrite);
				iNodePointer = MyFS::Instance()->getFreeInodePointer(bd);
				REQUIRE(iNodePointer == i);															//getFreeInodePointer mittesten
				MyFS::Instance()->createInodeBlock(bd,filenames[i],dataPointers[0],iNodePointer);	//Schreibe Inode einer zufälligen Datei in das BlockDevice
				bd->read(INODE_START + i,testBlock);										//Schreibe in block 2 den Inhalt der geschriebenen Inode
				inode* node = (inode*) testBlock;

				//Überprüfe geschriebenen Inhalt der Inodes im BlockDevice auf Sollwerte
				REQUIRE(strcmp(node->file_name, filenames[i]) == 0);
				REQUIRE(node->user_id == data.st_uid);
				REQUIRE(node->grp_id == data.st_gid);
				REQUIRE(node->protection == (S_IFREG | 0444));		//Vorgegebener Mode
				REQUIRE(node->st_blocks == blocksToWrite);
				REQUIRE(node->st_size== data.st_size);
				REQUIRE(node->first_data_block == dataPointers[0]);
				REQUIRE(node->mtime == data.st_mtim.tv_sec);
				REQUIRE(node->ctime >= data.st_ctim.tv_sec);
				REQUIRE(node->atime >= data.st_atim.tv_sec);

				//WRITE DATABLOCKS IN BLOCKDEVICE
			    char all_data[data.st_size];

			    FILE* f = fopen(filenames[i], "rb");
			    fread(all_data, data.st_size, 1, f);
			    fclose(f);

			    char singleBlock[BLOCK_SIZE] = {0};

			    for (u_int32_t dataBlock = 0; dataBlock < blocksToWrite - 1; dataBlock++) {
			        memcpy(singleBlock, all_data + BLOCK_SIZE * dataBlock, BLOCK_SIZE);
			        bd->write(dataPointers[dataBlock] + DATA_START, singleBlock);
			    }

			    auto rest = data.st_size % BLOCK_SIZE;
			    char restData[BLOCK_SIZE] = {0};
			    memcpy(restData, all_data + (blocksToWrite - 1) * BLOCK_SIZE, rest);
			    bd->write(dataPointers[blocksToWrite - 1] + DATA_START, restData);
			    MyFS::Instance()->superBlockNumFilesIncrease(bd);
				}

			//Test checkFileExist()
			SECTION("Test checkFileExist") {
				for(int i = 0; i < NUM_DIR_ENTRIES;i++){
				REQUIRE(MyFS::Instance()->checkFileExist(bd,filenames[i]) == i);
				}
				char falseName[] = "notExisting";
				REQUIRE(MyFS::Instance()->checkFileExist(bd,falseName) == -(ENOENT));

			}
			//Test findet größtenteils in CreateInode() statt!
			SECTION("Test getFreeInodePointer") {
				REQUIRE(MyFS::Instance()->getFreeInodePointer(bd) == 0xFFFFFFFF);
			}
			//Test getMaxBlocksNeeded()
			SECTION("Test getMaxBlocksNeeded") {
				REQUIRE(MyFS::Instance()->getMaxBlocksNeeded(0) == 1); //Leere Dateien brauchen einen DataBlock/DataPointer beim erstellen der Inode
				for(uint32_t i = 0; i < 100; i++) {
					uint32_t zufall = rand();
					if(zufall % BLOCK_SIZE == 0) {
						REQUIRE(MyFS::Instance()->getMaxBlocksNeeded(zufall) == zufall / BLOCK_SIZE);
					} else {
						REQUIRE(MyFS::Instance()->getMaxBlocksNeeded(zufall) == (zufall / BLOCK_SIZE) + 1);
					}
				}
			}
			//cleanUp
			//Alle zufällig erstellten Dateien löschen
			for(int i = 0; i < NUM_DIR_ENTRIES; i++) {
				remove(filenames[i]);
			}
		}
	}
	//bd.bin löschen

	bd->close();
	remove(BD1_PATH);
}
// TODO: Implement your helper functions here!
