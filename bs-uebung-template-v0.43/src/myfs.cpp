//
//  myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 02.08.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

// For documentation of FUSE methods see https://libfuse.github.io/doxygen/structfuse__operations.html

#undef DEBUG

//Comment this to reduce debug messages
#define DEBUG
#define DEBUG_METHODS
#define DEBUG_RETURN_VALUES

#ifndef _DARWIN_FEATURE_64_BIT_INODE
#define st_ctimespec st_ctim
#define st_birthtimespec st_ctim
#define st_atimespec st_atim
#define st_mtimespec st_mtim
#endif

#include "macros.h"
#include <errno.h>
#include "myfs.h"
#include "myfs-info.h"

MyFS* MyFS::_instance = NULL;

MyFS* MyFS::Instance() {
    if(_instance == NULL) {
        _instance = new MyFS();
    }
    return _instance;
}

MyFS::MyFS() {
    this->logFile= stderr;
}

MyFS::~MyFS() {
    
}

/**
 * Metadaten für Datei lesen.
 */
int MyFS::fuseGetattr(const char *path, struct stat *statbuf) {
    LOGM();
    statbuf->st_uid = getuid();     /* The owner of the file/directory is the user who mountned the filesystem */
    statbuf->st_gid = getgid();     /* The group of the file/directory is the same as the group of the user who mounted
                                     the filesystem */
    if (statbuf->st_birthtimespec.tv_sec == 0) {        /* If birthtime 01.01.1970 01:00 */
        statbuf->st_birthtimespec.tv_sec = time(0);
    }

    statbuf->st_atimespec.tv_sec = time(0);     /* The last access of the file/directory is right now */
    statbuf->st_mtimespec.tv_sec = time(0);     /* The last modification of the file/directory is right now */

    if (strcmp(path, "/") == 0) {
        LOGF("\n<--- Write st_mode and st_nlink for: %s --->\n", path);
        statbuf->st_mode = S_IFDIR | 0555;
        statbuf->st_nlink = 2;
        return 0;
    }

    int count = getNumbOfFiles();

    if (count > 0 && (strcmp(path, "/") != 0)) {
    	inode node[count];
    	getInodesOfFiles(count,node);
        for (int i = 0; i < count; i++) {
            string filename = "/";
            filename += node[i].file_name;
            const char* path2 = filename.c_str();
            if (strcmp(path, path2) == 0) {
                LOGF("\n<--- Write statbuf for: %s %s --->\n", path, path2);
                statbuf->st_uid = node[i].user_id;
                statbuf->st_gid = node[i].grp_id;
                statbuf->st_ctimespec.tv_sec = node[i].ctime;
                statbuf->st_mode = node[i].protection;
                statbuf->st_nlink = 1;
                statbuf->st_size = node[i].st_size;
                statbuf->st_blocks = node[i].st_blocks;
                statbuf->st_blksize = BLOCK_SIZE;
                RETURN(0);
            }
        }
    }
    return -(ENOENT);

}


int MyFS::fuseReadlink(const char *path, char *link, size_t size) {
    //LOGM();
    return 0;
}

int MyFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGM();
    LOGF("path: %s, mode: %ld , dev %ld",path,mode,dev);
    int err;
    u_int32_t iNodePointer = getFreeInodePointer(&bd_fuse);
    if (iNodePointer == 0xFFFFFFFF) {
    	RETURN(-ENOSPC);
    }
    u_int32_t dataPointers[1];
    err = getFreeDataPointers(&bd_fuse, dataPointers,1,0);
    if(err) {
    	//TODO: benutzten InodePointer freigeben!
    	RETURN(-EFBIG);
    }
    char emptyBlock[BLOCK_SIZE] = {0};
    bd_fuse.write(DATA_START + dataPointers[0],emptyBlock);			//clean Block(it could have been used from deleted files)
    createNewInode(&bd_fuse, path+1, dataPointers[0], iNodePointer);
    writeFatEntries(&bd_fuse, dataPointers, 1);
    RETURN(0);
}

int MyFS::fuseMkdir(const char *path, mode_t mode) {
    //LOGM();
    return 0;
}

int MyFS::fuseUnlink(const char *path) {
    LOGM();
    path++;
    int iNodeNumber = checkFileExist(&bd_fuse,path);
    LOGF("iNodeNumber = %d , filename = %s",iNodeNumber,path);
    if(iNodeNumber >= 0) {									//Existiert die Datei?
    	char block[BLOCK_SIZE];
    	superBlockNumFilesDecrease(&bd_fuse);				//FileNumber im SuperBlock verringern
    	bd_fuse.read(INDOE_MAP_START,block);				//iMap bit der Datei finden
    	uint32_t byteOffset = iNodeNumber / 8;
    	uint32_t bitOffset = iNodeNumber - byteOffset * 8;
    	uint8_t bitMask = 0x80 >> bitOffset;
    	bitMask = ~bitMask;
    	block[byteOffset] &= bitMask;						//bit auf 0 setzen
    	bd_fuse.write(INDOE_MAP_START, block);
    	bd_fuse.read(INODE_START + iNodeNumber,block);
    	inode* node = (inode*)block;
    	uint32_t numberOfDataPointers = node->st_blocks;
    	uint32_t* allDataPointers = (uint32_t*)malloc(numberOfDataPointers * 4);
    	allDataPointers[0] = node->first_data_block;
    	getAllUsedDataPointers(allDataPointers,numberOfDataPointers);
    	deleteDMapEntries(allDataPointers,numberOfDataPointers);
    	free(allDataPointers);
    	RETURN(0);
    }
    RETURN(-(ENOENT));
}


int MyFS::fuseRmdir(const char *path) {
    //LOGM();
    return 0;
}

int MyFS::fuseSymlink(const char *path, const char *link) {
    //LOGM();
    return 0;
}

int MyFS::fuseRename(const char *path, const char *newpath) {
    //LOGM();
	//TODO
    return 0;
}

int MyFS::fuseLink(const char *path, const char *newpath) {
    //LOGM();
    return 0;
}

int MyFS::fuseChmod(const char *path, mode_t mode) {
    //LOGM();
    return 0;
}

int MyFS::fuseChown(const char *path, uid_t uid, gid_t gid) {
    //LOGM();
    return 0;
}

int MyFS::fuseTruncate(const char *path, off_t newSize) {
    //LOGM();
    return 0;
}

int MyFS::fuseUtime(const char *path, struct utimbuf *ubuf) {
    //LOGM();
    return 0;
}
/* Erstellt für eine geöffnete Datei ein FileHandleBuffer.
 * Der FileHandleBuffer beinhaltet Informationen über die Datei und ein DataBuffer/FatBuffer für das Lesen und schreiben.
 *
 * @param path - Dateiname der geöffneten Datei.
 * @param fileInfo - In der fileInfo wird der Pointer zum FileHandleBuffer gespeichert.
 * @return - 0 bei erfolgreichen öffnen, -ENOENT wenn Datei nicht gefunden wurde.
 */
int MyFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    uint32_t inodeNumber = checkFileExist(&bd_fuse,path+1);
    if(inodeNumber < 0) {
    	RETURN(-(ENOENT))
    }
    char block [BLOCK_SIZE];
    FileHandleBuffer* fileBuffer = new FileHandleBuffer;
    LOGF("%s Buffer auf Adresse %ld erstellt.\n",path,fileBuffer);
    fileBuffer->inodeNumber = inodeNumber;											//inodeNumber in fileBuffer festlegen
    bd_fuse.read(INODE_START + inodeNumber,block);
    inode* node = (inode*)block;
    fileBuffer->fileSize = node->st_size;											//dateiGröße in fileBuffer schreiben
    fileBuffer->absoluteDataBlockNumber = node->first_data_block + DATA_START;		//Absolute DataBlock-Nummer eintragen
    fileBuffer->relativeDataBlockNumber = 0;
    fileBuffer->startDataPointer = node->first_data_block;							//relative DataBlock-Nummer eintragen
    fileBuffer->currentDataPointer = node->first_data_block;						//ersten DataPointer in fileBuffer schreiben
    fileBuffer->numDataPointers = node->st_blocks;
    bd_fuse.read(DATA_START + node->first_data_block,fileBuffer->dataBlockBuffer);	//ersten dataBlock in fileBuffer schreiben
    uint32_t fatBlockOffset = node->first_data_block / (BLOCK_SIZE/POINTER_SIZE);
    bd_fuse.read(FAT_START + fatBlockOffset,fileBuffer->fatBuffer);					//fatBlock mit ersten DataPointer Eintrag in Buffer schreiben
    fileBuffer->fatBlockNumber = fatBlockOffset;
    fileInfo->fh = (uint64_t)fileBuffer;
    RETURN(0);
}
    
/* Liest aus einer Datei ab offset bis offset + size. Die zu lesenden Bytes müssen aus den DataBlocks der Datei in den buf
 * geschrieben werden. Sollte fuseRead() über Dateiende hinaus lesen wollen muss size gekürzt werden.
 *
 * @param path - Name der Datei aus der gelesen wird.
 * @param buf - Buffer in dem die zu lesenden Bytes reingeschrieben werden.
 * @param size - Die Anzahl der Bytes die das Programm das fuseRead() aufgerufen hat, lesen will.
 * Muss verkleinert werden falls das Programm über Dateiende hinaus lesen will
 * @param offset - Position ab der gelesen wird in der Datei in Bytes.
 * @param fileInfo - beinhaltet den Pointer zum FileBuffer.
 * @return - die Menge der Bytes die in buf reingeschrieben wurde.
 */
int MyFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
	LOGM();
	FileHandleBuffer* fileBuffer = (FileHandleBuffer*)fileInfo->fh;
	LOGF("fileSize = %d \nrequestedSize = %d \noffset = %d",fileBuffer->fileSize,size,offset);
	if(offset < fileBuffer->fileSize) {
		 uint32_t blockOffset = offset / BLOCK_SIZE;
		 uint32_t byteOffset = offset - (blockOffset * BLOCK_SIZE);
		 if(offset + size > fileBuffer->fileSize) {
			 LOGF("Size reduced from %d",size);
			 size = fileBuffer->fileSize - offset;
			 LOGF(" to %d \n",size);
		 }
		 char requestedData[size];
		 size_t copiedBytes = 0;
		 if(fileBuffer->relativeDataBlockNumber != blockOffset) {			//Angefragte Block befindet sich nicht im Buffer, dataPointer suchen und Block laden
			 if(fileBuffer->relativeDataBlockNumber + 1 == blockOffset) {
				 fileBuffer->currentDataPointer = nextDataPointer(fileBuffer, fileBuffer->currentDataPointer);
			 }else {
				 LOGF("RelativeBlockNumber != blockOffset!: %d != %d \n",fileBuffer->relativeDataBlockNumber,blockOffset);
				 fileBuffer->currentDataPointer = getRequestedDataPointer(fileBuffer,blockOffset);
			 }
			 fileBuffer->relativeDataBlockNumber = blockOffset;
			 readFromFileBuffer(fileBuffer,&bd_fuse);
		 }
		 while(copiedBytes < size) {
			 if(byteOffset + size - copiedBytes > BLOCK_SIZE && nextDataPointer(fileBuffer, fileBuffer->currentDataPointer) != 0xFFFFFFFF){
				 memcpy(requestedData + copiedBytes, fileBuffer->dataBlockBuffer + byteOffset, BLOCK_SIZE - byteOffset);
				 copiedBytes += BLOCK_SIZE - byteOffset;
				 byteOffset = 0;
				 fileBuffer->currentDataPointer = nextDataPointer(fileBuffer,fileBuffer->currentDataPointer);
				 fileBuffer->relativeDataBlockNumber += 1;
				 readFromFileBuffer(fileBuffer,&bd_fuse);
			 } else {
				 memcpy(requestedData + copiedBytes, fileBuffer->dataBlockBuffer + byteOffset, size - copiedBytes);
				 copiedBytes += size;
			 }
		 }
		 memcpy(buf,requestedData,size);
	}else {
        size = 0;
    }
    RETURN(size);
}
/* Schreibt von offset bis offset + size mit Inhalt aus buf in die Datei path.
 *
 * @param path - Datei in die geschrieben wird.
 * @param buf - Der Inhalt der geschrieben werden soll.
 * @param size - Die Menge an Bytes die geschrieben werden soll.
 * @param offset - Die Stelle ab der geschrieben werden soll  in Bytes.
 * @param fuse_file_info - Informationen über die Datei die vorher in fuseOpen() gefüllt wurde.
 * @return - Die Anzahl der geschriebenen Bytes.
 */
int MyFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("Path: %s , size : %ld , offset : %ld \n",path,size,offset);
    FileHandleBuffer* fileBuffer = (FileHandleBuffer*)fileInfo->fh;
    uint32_t blockOffsetFirst = offset / BLOCK_SIZE;
    uint32_t blockOffsetLast = getMaxBlocksNeeded(size+offset);
    uint32_t byteOffsetFirst = offset - (blockOffsetFirst * BLOCK_SIZE);
    uint32_t writtenBytes = 0;
	uint32_t lastPointer;

	if(fileBuffer->positionOfLastFoundPointer == 0) {		//letzten Pointer holen
		lastPointer = getRequestedDataPointer(fileBuffer,fileBuffer->numDataPointers - 1);
		fileBuffer->positionOfLastFoundPointer = lastPointer;
	}else {
		lastPointer = fileBuffer->positionOfLastFoundPointer;
	}
    if(fileBuffer->numDataPointers < blockOffsetLast) {													//Prüfen ob genug DataBlocks vorhanden sind.
    		LOGF("Wir brauchen mehr Pointer! %d neue Pointer \n",blockOffsetLast - fileBuffer->numDataPointers);
    		int err;
    		uint32_t amountOfPointersNeeded = blockOffsetLast - fileBuffer->numDataPointers;
    		uint32_t newNeededPointers[amountOfPointersNeeded];
    		err = getFreeDataPointers(&bd_fuse,newNeededPointers,amountOfPointersNeeded,fileBuffer->positionOfLastFoundPointer);
    		if(err) {
    			//TODO: Alle Pointer freigeben die in anspruch genommen worden sind, wenn Datei zu groß ist!.
    			RETURN(EFBIG);
    		}																							 //Suche letzten Pointer bevor datei vergrößert wurde
    		fileBuffer->numDataPointers += amountOfPointersNeeded;										//Erhöhe Anzahl der DataPointers der Datei.
    		writeAdditionalFatEntries(&bd_fuse,lastPointer,newNeededPointers,amountOfPointersNeeded);	//Neue Pointer Eintragen in BlockDevice
    		uint32_t fatBlockOffset = fileBuffer->startDataPointer / (BLOCK_SIZE/POINTER_SIZE);
    		bd_fuse.read(FAT_START + fatBlockOffset,fileBuffer->fatBuffer);								//update buffer, sonst sind veraltete falsche Werte im Buffer
    		fileBuffer->fatBlockNumber = fatBlockOffset;
    		for(uint32_t i = 0; i < amountOfPointersNeeded;i++) {
    			lastPointer = nextDataPointer(fileBuffer,lastPointer);
    		}
    		char emptyBlock[BLOCK_SIZE] = {0};
    		bd_fuse.write(DATA_START + lastPointer,emptyBlock);
    		fileBuffer->positionOfLastFoundPointer = lastPointer;
     }
	 if(fileBuffer->relativeDataBlockNumber != blockOffsetFirst) {			//Angefragte Block befindet sich nicht im Buffer, dataPointer suchen und Block laden
		 if(fileBuffer->relativeDataBlockNumber + 1 == blockOffsetFirst) {
			 fileBuffer->currentDataPointer = nextDataPointer(fileBuffer, fileBuffer->currentDataPointer);
		 }else {
			 LOGF("RelativeBlockNumber != blockOffset!: %d != %d \n",fileBuffer->relativeDataBlockNumber,blockOffsetFirst);
			 fileBuffer->currentDataPointer = getRequestedDataPointer(fileBuffer,blockOffsetFirst);
		 }
		 fileBuffer->relativeDataBlockNumber = blockOffsetFirst;
		 readFromFileBuffer(fileBuffer,&bd_fuse);
	 }

	 while(writtenBytes < size) {											//Der eigentliche Schreibvorgang
		 if(byteOffsetFirst + size - writtenBytes > BLOCK_SIZE && nextDataPointer(fileBuffer, fileBuffer->currentDataPointer) != 0xFFFFFFFF){
				 memcpy(fileBuffer->dataBlockBuffer + byteOffsetFirst, buf + writtenBytes, BLOCK_SIZE - byteOffsetFirst);
				 writeFromDataBufferToBlockDevice(fileBuffer,&bd_fuse);
				 writtenBytes += BLOCK_SIZE - byteOffsetFirst;
				 byteOffsetFirst = 0;
				 fileBuffer->currentDataPointer = nextDataPointer(fileBuffer,fileBuffer->currentDataPointer);
				 fileBuffer->relativeDataBlockNumber += 1;
				 readFromFileBuffer(fileBuffer,&bd_fuse);
			 } else {
				 memcpy(fileBuffer->dataBlockBuffer + byteOffsetFirst, buf + writtenBytes,  size - writtenBytes);
				 writtenBytes += size;
				 writeFromDataBufferToBlockDevice(fileBuffer,&bd_fuse);
			 }
    }
	 LOGF("numDataPointers : %ld , posOfLastPointer: %ld", fileBuffer->numDataPointers, fileBuffer->positionOfLastFoundPointer);
	uint32_t newFileSize = fileBuffer->numDataPointers * BLOCK_SIZE - unusedBytesInDataBlock(fileBuffer->positionOfLastFoundPointer);
	LOG("Test 2");
	char inodeBlock[BLOCK_SIZE];
	readFromBuffer(INODE_START + fileBuffer->inodeNumber,inodeBlock,&bd_fuse);
	inode* node = (inode*)inodeBlock;
	node->st_blocks = fileBuffer->numDataPointers;
	node->st_size = newFileSize;
	writeToBuffer(INODE_START + fileBuffer->inodeNumber,inodeBlock,&bd_fuse);
	writeBufferToBlockDevice(&bd_fuse);	//Inode mit neuen Werten der Dateigröße und anzahl der benutzten DataBlöcke speichern.
	RETURN(size);
}

int MyFS::fuseStatfs(const char *path, struct statvfs *statInfo) {
    //LOGM();
    return 0;
}

int MyFS::fuseFlush(const char *path, struct fuse_file_info *fileInfo) {
    //LOGM();
    return 0;
}

int MyFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    FileHandleBuffer* fileBuffer = (FileHandleBuffer*)fileInfo->fh;
    delete fileBuffer;
    RETURN(0);
}

int MyFS::fuseFsync(const char *path, int datasync, struct fuse_file_info *fi) {
    //LOGM();
    return 0;
}

int MyFS::fuseListxattr(const char *path, char *list, size_t size) {
    //LOGM();
    return 0;
}

int MyFS::fuseRemovexattr(const char *path, const char *name) {
    //LOGM();
    return 0;
}

int MyFS::fuseOpendir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    
    
    RETURN(0);
}

int MyFS::fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    LOGF("\n<--- Getting the list of files of %s --->\n", path);
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    
    int count = getNumbOfFiles();
    LOGF("\n<---- Getting the number of files %d --->\n", count);
    
    if (count > 0) {
    	inode node[count];
    	getInodesOfFiles(count,node);
        for (int i = 0; i < count; i++) {
            const char* name = node[i].file_name;
            filler(buf, name, NULL, 0);
            LOGF("filler: %s \n", name);
        }
        return 0;
    }
    RETURN(0);
    
    // <<< My new code
    
    return -(ENOTDIR);
}

int MyFS::fuseReleasedir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseFsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
    //LOGM();
    return 0;
}

int MyFS::fuseTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
    //LOGM();
    return 0;
}

int MyFS::fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fileInfo) {
    LOGM();
    
    RETURN(0);
}

void MyFS::fuseDestroy() {
    LOGM();
}


/*
 Zum Initialisieren, BlockDevice anlegen um Containerdatei zu lesen
*/
void* MyFS::fuseInit(struct fuse_conn_info *conn) {
    // Open logfile
    this->logFile= fopen(((MyFsInfo *) fuse_get_context()->private_data)->logFile, "w+");
    if(this->logFile == NULL) {
        fprintf(stderr, "ERROR: Cannot open logfile %s\n", ((MyFsInfo *) fuse_get_context()->private_data)->logFile);
    } else {
        //    this->logFile= ((MyFsInfo *) fuse_get_context()->private_data)->logFile;
        
        // turn of logfile buffering
        setvbuf(this->logFile, NULL, _IOLBF, 0);
        
        LOG("Starting logging...\n");
        LOGM();
        
        // you can get the containfer file name here:
        LOGF("Container file name: %s", ((MyFsInfo *) fuse_get_context()->private_data)->contFile);
        
        bd_fuse.open(((MyFsInfo *) fuse_get_context()->private_data)->contFile);
        
    }
    
    RETURN(0);
}

#ifdef __APPLE__
int MyFS::fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags, uint32_t x) {
#else
int MyFS::fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
#endif
    //LOGM();
    return 0;
}
    
#ifdef __APPLE__
int MyFS::fuseGetxattr(const char *path, const char *name, char *value, size_t size, uint x) {
#else
int MyFS::fuseGetxattr(const char *path, const char *name, char *value, size_t size) {
#endif
    //LOGM();
    return 0;
}
        
//---------------Methoden die nur für mount.myfs benötigt werden---------------//

/* List aus dem SuperBlock die Anzahl der vorhandenden Dateien im BlockDevice
 *
 * @return - Anzahl der Dateien im BlockDevice.
 */
int MyFS::getNumbOfFiles() {
    char superblock[BLOCK_SIZE] = {0};
    bd_fuse.read(SUPER_BLOCK_START, superblock);
    super_block* sb = (super_block*) superblock;
    return sb->numbFiles;
}

/* Schreibt alle vorhandenen Inodes in das inodes-Array.
 *
 * @param numbOfFiles - Anzahl der zu lesenden Inodes.
 * @param inodes - Array in dem die Inodes gespeichert werden.
 */
void MyFS::getInodesOfFiles(int numbOfFiles, inode* inodes) {
    char iMap[BLOCK_SIZE] = {0};
    char buffer[BLOCK_SIZE] = {0};

    bd_fuse.read(INDOE_MAP_START, iMap);
    u_int8_t bitMask;
    int count = 0;

    for (int byte = 0; (byte < NUM_DIR_ENTRIES / 8) && (count < numbOfFiles); byte++) {
        bitMask = 0x80;

        for (int bitOffset = 0; (bitOffset < 8) && (count < numbOfFiles); bitOffset++) {
            if ((iMap[byte] & bitMask) == bitMask) {
                bd_fuse.read(INODE_START + bitOffset + byte * 8, buffer);
                inode* node = (inode*) buffer;
                inodes[count].operator=(*node);
                count++;
            }
            bitMask >>= 1;
        }
    }
}

/* Gibt den gesuchten DataPointer zurück der auf dataBlockNr zeigt.
 * Beispiel für fuseRead: aktuelle Pointer im fhb zeigt auf Block 12.
 * fuseRead() will aber ab Block 09 lesen. Dann wird der StartPointer der Datei genommen,
 * und der Pointer auf Block 09 zeigt wird aus der Table geholt.
 *
 * @param fhb - fileBuffer der start Informationen über die geöffnete Datei beinhaltet.
 * @param dataBlockNr - Datablock aus dem gelesen werden soll.
 * @return - Pointer der auf dataBlockNr zeigt
 */
u_int32_t MyFS::getRequestedDataPointer(FileHandleBuffer* fhb, u_int32_t dataBlockNr) {
    u_int32_t counter = 0;
    uint32_t startPointer = fhb->startDataPointer;
    u_int32_t blockOffset = startPointer / (BLOCK_SIZE /POINTER_SIZE);
    u_int32_t pointerOffset = startPointer - blockOffset * (BLOCK_SIZE / POINTER_SIZE);
    fat* s_fat = (fat*) fhb->fatBuffer;
    while(counter < dataBlockNr) {
        readFromFatBuffer(fhb,blockOffset, &bd_fuse);
        startPointer = s_fat->table[pointerOffset];
        blockOffset = startPointer / (BLOCK_SIZE / POINTER_SIZE);
        pointerOffset = startPointer - blockOffset * (BLOCK_SIZE / POINTER_SIZE);
        counter++;
    }
    return startPointer;
}

/* Gibt den Wert des nächsten Pointers zurück.
 *
 * @param fhb - fileHanleBuffer der geöffneten Datei.
 * @param dataPointer - Pointer bei dem der nächste Eintrag zurückgegeben werden soll.
 * @return - Wert des nächten Pointers.
 */
u_int32_t MyFS::nextDataPointer(FileHandleBuffer* fhb, u_int32_t dataPointer) {
    u_int32_t blockOffset = dataPointer / (BLOCK_SIZE / POINTER_SIZE);
    u_int32_t pointerOffset = dataPointer - blockOffset * (BLOCK_SIZE / POINTER_SIZE);
    fat* s_fat = (fat*) fhb->fatBuffer;
    readFromFatBuffer(fhb,blockOffset,&bd_fuse);
    return s_fat->table[pointerOffset];
}

/* Verringert den Wert der vorhandenen Dateien im Filesystem um 1.
 *
 * @param bd - BlockDevice in dem der Wert erhöht werden soll.
 */
void MyFS::superBlockNumFilesDecrease(BlockDevice* bd) {
    char buffer[BLOCK_SIZE] = {0};
    bd->read(SUPER_BLOCK_START, buffer);
    super_block* sb = (super_block*) buffer;
    sb->numbFiles--;
    bd->write(SUPER_BLOCK_START, buffer);
}

/* Liest aus dem dataBuffer des FileHandleBuffers. Wenn nicht der richtige Block sich im dataBuffer befindet,
 * wird aus dem BlockDeviceder richtige in den dataBuffer geladen.
 *
 * @param fhb - beinhaltet den dataBlockBuffer.
 * @param bd - BlockDevice aus dem gelesen werden soll.
 */
int MyFS::readFromFileBuffer(FileHandleBuffer* fhb, BlockDevice* bd) {
		if(fhb->currentDataPointer + DATA_START == fhb->absoluteDataBlockNumber) {	//is Block already loaded?
			return 0;
		}
		bd->read(DATA_START+fhb->currentDataPointer,fhb->dataBlockBuffer);		//get the Block and update absoluteBlockNumber.
		fhb->absoluteDataBlockNumber = fhb->currentDataPointer + DATA_START;
		return 0;
}

/* Schreibt den aktuellen Block im DataBuffer der geöffneten Datei in das BlockDevice.
 *
 * @param fhb - fileHandleBuffer der geöffneten Datei.
 * @param bd - BlockDevice in welches der DataBlock geschrieben werden soll.
 */
void MyFS::writeFromDataBufferToBlockDevice(FileHandleBuffer* fhb, BlockDevice* bd) {
	bd->write(fhb->currentDataPointer + DATA_START,fhb->dataBlockBuffer);
}

/* Liest aus dem FatBuffer. Wenn nicht der richtige FatBlock im Buffer ist, wird aus dem BlockDevice der benötigte geladen und im Buffer gespeichert.
 *
 * @param fhb - fileHandleBuffer in dem der FatBuffer für die geöffnete Datei gespeichert ist.
 * @param fatBlockOffset - Position des FatBlocks der gelesen werden soll.
 * @param bd - BlockDevice aus dem die Fat-Blöcke gelesen werden.
 */
int MyFS::readFromFatBuffer(FileHandleBuffer* fhb, uint32_t fatBlockOffset, BlockDevice* bd) {
    if (fhb->fatBlockNumber == fatBlockOffset) {
        return 0;
    }
    bd->read(FAT_START + fatBlockOffset, fhb->fatBuffer);
    fhb->fatBlockNumber = fatBlockOffset;
    return 0;
}

/* Schreibt alle benutzten DataPointer einer Datei in ein Array.
 *
 * @param dataPointerArray - Array in dem alle benutzten DataPointer gespeicher werden.
 * @param sizeOfArray - Anzahl der zu speichernden Pointer.
 */
void MyFS::getAllUsedDataPointers(uint32_t* dataPointerArray, uint32_t sizeOfArray) {
    char table[BLOCK_SIZE] = {0};
    fat* s_fat = (fat*) table;
    u_int32_t blockOffset = dataPointerArray[0] / (BLOCK_SIZE / POINTER_SIZE);
    uint32_t pointerOffset = dataPointerArray[0] - blockOffset * (BLOCK_SIZE/POINTER_SIZE);
    readFromBuffer(FAT_START + blockOffset, table, &bd_fuse);
	for(uint32_t i = 0 ; i < sizeOfArray - 1;i++) {
		readFromBuffer(FAT_START + blockOffset,table,&bd_fuse);
		dataPointerArray[i + 1] = s_fat->table[pointerOffset];
		blockOffset = dataPointerArray[i + 1] / (BLOCK_SIZE /POINTER_SIZE);
		pointerOffset = dataPointerArray[i + 1] - blockOffset * (BLOCK_SIZE / POINTER_SIZE);
	}
}
/* Setzt alle DMap Einträge Frei die im dataPointerArray sind.
 *
 * @param dataPointerArray - Array mit allen besetzten Data-Pointern die freigesetzt werden sollen.
 * @param sizeOfArray - Anzahl der Einträge die Freigesetzt werden sollen.
 */
void MyFS::deleteDMapEntries(uint32_t* dataPointerArray, uint32_t sizeOfArray) {
    char dataDMap[BLOCK_SIZE] = {0};
    u_int8_t bitMask;
    uint32_t blockOffset = dataPointerArray[0] / (BLOCK_SIZE * 8);
    uint32_t byteOffset;
    uint32_t bitOffset;
    readFromBuffer(DATA_MAP_START + blockOffset,dataDMap,&bd_fuse);
    for(uint32_t i = 0; i < sizeOfArray;i++) {
    	if(blockOffset != dataPointerArray[i] / (BLOCK_SIZE * 8)) {
    		blockOffset = dataPointerArray[i] / (BLOCK_SIZE * 8);
    		writeBufferToBlockDevice(&bd_fuse);
    		readFromBuffer(DATA_MAP_START + blockOffset,dataDMap,&bd_fuse);
    	}
    	byteOffset = (dataPointerArray[i] - (blockOffset * BLOCK_SIZE * 8)) / 8;
    	bitOffset = dataPointerArray[i] - ((blockOffset * BLOCK_SIZE * 8) + (byteOffset * 8));
    	bitMask = 0x80 >> bitOffset;
    	bitMask = ~bitMask;
    	dataDMap[byteOffset] &= bitMask;
    	writeToBuffer(DATA_MAP_START + blockOffset,dataDMap,&bd_fuse);
    }
    writeBufferToBlockDevice(&bd_fuse);
}

/* Gibt die Anzahl der nicht benutzen Bytes in einem Block zurück.
 * Wird benötigt für fuseWrite(), um die neue Dateigröße genau zu bestimmen.
 * TODO:: Wenn Block Komplett leer ist, vorherige Block aufrufen und da nachgucken.
 * Alle leeren Blocks müssen rausgeschmissen werden. Vllt fuseTruncate() aufrufen hierfür?
 *
 * @param dataPointer - Letzter geschriebener Block.
 * @return - Anzahl nicht benutzer Bytes im Block.
 */
uint32_t MyFS::unusedBytesInDataBlock(uint32_t dataPointer) {
	uint32_t emptyBytes = 0;
	uint32_t counter = BLOCK_SIZE - 1;
	char dataBlock[BLOCK_SIZE];
	readFromBuffer(DATA_START + dataPointer,dataBlock,&bd_fuse);
	while(counter >= 0 && dataBlock[counter] == 0) {
		emptyBytes++;
		counter--;
	}
	return emptyBytes;
}

/* Schreibt zu einer existierenden Datei im BlockDevice zusätzliche neue Einträge in die FAT.
 * Wird aufgerufen wenn z.b eine Datei in das BlockDevice kopiert wird bzw. größer wird.
 * Der alte letzte Pointer zeigt nicht mehr auf 0xFFFFFFFF(Dateiende) sondern auf den neuen ersten Eintrag im Pointer-Array.
 * Der letzte Eintrag im Pointer-Array wird zum neuen Dateiende.
 *
 * @param bd - BlockDevice in dem die FAT-Einträge reinkommen.
 * @param lastPointer - Der letzte Pointer vor dem vergrößern der Datei.
 * @param pointers - Array mit den neuen benötigten DataPointers.
 * @param sizeOfArray - Anzahl der neuen DataPointers.
 */
void MyFS::writeAdditionalFatEntries(BlockDevice *bd, uint32_t lastPointer, u_int32_t *pointers, u_int32_t sizeOfArray) {
    char table[BLOCK_SIZE] = {0};
    fat* s_fat = (fat*) table;
    u_int32_t blockOffset = lastPointer / (BLOCK_SIZE / POINTER_SIZE);
    readFromBuffer(FAT_START + blockOffset, table, bd);
    s_fat->table[lastPointer - blockOffset * (BLOCK_SIZE/POINTER_SIZE)] = pointers[0];		//Last Block now refers to the new pointer in pointers[0]
    writeToBuffer(FAT_START + blockOffset, table, bd);
    writeBufferToBlockDevice(bd);
    blockOffset = pointers[0] / (BLOCK_SIZE / POINTER_SIZE);
    readFromBuffer(FAT_START + blockOffset, table, bd);
    for (u_int32_t entries = 0; entries < sizeOfArray - 1; entries++) {
        if (blockOffset != pointers[entries] / (BLOCK_SIZE / POINTER_SIZE)) {
            writeToBuffer(FAT_START + blockOffset, table, bd);
            writeBufferToBlockDevice(bd);
            blockOffset = pointers[entries] / (BLOCK_SIZE / POINTER_SIZE);
            readFromBuffer(FAT_START + blockOffset, table, bd);
        }
        s_fat->table[pointers[entries] - blockOffset * (BLOCK_SIZE / POINTER_SIZE)] = pointers[entries + 1];
    }
    blockOffset = pointers[sizeOfArray - 1] / (BLOCK_SIZE / POINTER_SIZE);
    s_fat->table[pointers[sizeOfArray - 1] - blockOffset * (BLOCK_SIZE / POINTER_SIZE)] = 0xFFFFFFFF;
    writeToBuffer(FAT_START + blockOffset, table, bd);
    writeBufferToBlockDevice(bd);
}

/* Estellt eine neue Inode für eine neue Datei im BlockDevice.
 * TODO:: Es ist noch nicht ganz klar wie auf Dateiinformationen von einer exisitierenden Datei die in das BlockDevice kopiert werden soll
 * zugegriffen werden kann, falls man das überhaupt machen soll.
 *
 * @param bd - BlockDevice in dem die Inode erstellt wird.
 * @param path - Dateiname der neuen Datei.
 * @param dataPointer - Erster DataPointer der Datei.
 * @param inodePointer - iNodePointer der auf die Stelle im BlockDevice zeigt in dem die Inode erstellt wird.
 */
void MyFS::createNewInode(BlockDevice* bd, const char* path, u_int32_t dataPointer, u_int32_t iNodePointer) {
    char structure[BLOCK_SIZE] = {0};
    inode* node = (inode*) structure;

    strncpy(node->file_name, path, sizeof(node->file_name));/* File name */
    node->st_size = 0;										/* total size, in bytes */
    node->user_id = getuid();								/* user ID of owner */
    node->grp_id = getgid();								/* group ID of owner */
    node->protection = (S_IFREG | 0777);                    /* protection lesen und schreiben für alle, TODO: soll das so sein?*/
    node->atime = time(0);                                  /* time of last access */
    node->ctime = time(0);                                  /* time of last status change */
    node->mtime = time(0);                      	        /* time of last modification */
    node->first_data_block = dataPointer;                   /* number of first file data-block */
    node->st_blocks = 1;            						/* number of 512 byte blocks allocated */
    bd->write(INODE_START + iNodePointer, (char*) node);
    superBlockNumFilesIncrease(bd);
}

/* Trägt die DataPointers im pointers Array in die FAT ein.
 * Die aktuellen FAT-Einträge werden zuerst in einem Char array "table" geladen um bereits vorhandene Einträge zu
 * behalten. (Es können
 * keine einzelnen Einträge geschrieben werden, sondern immer nur ein ganzer FATBlock.)
 * Es werden alle DataPointers, die in das geladene Block müssen, in den Buffer reingeschrieben. Sind DataPointers
 * vorhanden im DataPointer Array, die in einen
 * anderen FAT-Block reingeschrieben werden müssen, so wird der Buffer mit allen Änderungen in das BlockDevice
 * geschrieben und der benötigte Block wird in den
 * Buffer geladen. Der letzte Eintrag im DataPointer Array wird an der Stelle im FAT mit 0xFFFF FFFF
 * eingetragen(Dateiende).
 *
 * In einem FATBlock sind BLOCK_SIZE/POINTER_SIZE Einträge drin.
 * Ein Eintrag im FAT bedeutet, dass die Datei mehrere DatenBlöcke hat aus denen gelesen werden muss, oder dass das
 * Dateiende erreicht wurde(0xFFFF FFFF).
 * Beispiel: An der Stelle 18 steht im FAT der Wert 21. Das heißt, im DataBlock 21 ist der nächste Teil der Datei
 * geschrieben.
 * außerdem heißt das auch, das an der Stelle 21 im FAT der Eintrag geprüft werden muss. Ist im Eintrag 21 im FAT
 * der Wert 0xFFFF FFFF
 * drin, so bedeutet dies dass das Dateiende erreicht wurde. Sollte eine andere Zahl wie 0xFFFF FFFF drin stehen,
 * bedeutet das die Datei
 * noch größer ist und weiter geguckt werden muss.
 *
 * @param bd - BlockDevice in das geschrieben wird.
 * @param pointers - Das Array welches alle benötigten DataPointers enthält.
 * @param sizeOfArray - Anzahl der Einträge im pointers Array.
*/
void MyFS::writeFatEntries(BlockDevice *bd, u_int32_t *pointers, u_int32_t sizeOfArray) {
    char table[BLOCK_SIZE] = {0};
    fat* s_fat = (fat*) table;
    u_int32_t blockOffset = pointers[0] / (BLOCK_SIZE / POINTER_SIZE);
    readFromBuffer(FAT_START + blockOffset, table, bd);

    for (u_int32_t entries = 0; entries < sizeOfArray - 1; entries++) {
        if (blockOffset != pointers[entries] / (BLOCK_SIZE / POINTER_SIZE)) {
            writeToBuffer(FAT_START + blockOffset, table, bd);
            writeBufferToBlockDevice(bd);
            blockOffset = pointers[entries] / (BLOCK_SIZE / POINTER_SIZE);
            readFromBuffer(FAT_START + blockOffset, table, bd);
        }
        s_fat->table[pointers[entries] - blockOffset * (BLOCK_SIZE / POINTER_SIZE)] = pointers[entries + 1];
    }
    blockOffset = pointers[sizeOfArray - 1] / (BLOCK_SIZE / POINTER_SIZE);
    s_fat->table[pointers[sizeOfArray - 1] - blockOffset * (BLOCK_SIZE / POINTER_SIZE)] = 0xFFFFFFFF;
    writeToBuffer(FAT_START + blockOffset, table, bd);
    writeBufferToBlockDevice(bd);
}

//---------------Methoden die für mount.myfs und mkfs.myfs benötigt werden---------------//

/* Geht durch die IMap und sucht nach einer freien Stelle.
 * Wurde eine freie Stelle gefunden (0 = freie Stelle) so wird diese belegt
 * (1 = belegt) und der neue Eintrag wird in das BlockDevice geschrieben.
 * Die Stelle an der die neue Inode geschrieben weden soll, wird als Wert zurückgegeben.
 *
 * @param bd - BlockDevice in dem eine leere Inode gesucht wird.
 * @return - InodeBlock der Leer ist.(Wenn kein freier Platzt vorhanden ist, wird 0xFFFF FFFF zurückgegeben.)
*/
u_int32_t MyFS::getFreeInodePointer(BlockDevice *bd) {
    char dataIMap[BLOCK_SIZE] = {0};
    u_int8_t bitMask;
    bd->read(INDOE_MAP_START, dataIMap);

    for (int byte = 0; byte < NUM_DIR_ENTRIES / 8; byte++) {
        bitMask = 0x80;

        for (int bit = 0; bit < 8; bit++) {
            if ((dataIMap[byte] & bitMask) == 0) {
                dataIMap[byte] |= bitMask;
                bd->write(INDOE_MAP_START, dataIMap);
                return bit + byte * 8;
            }
            bitMask >>= 1;
        }
    }
    return ~0;
}

/* Liest die Daten aus dem Buffer und schreibt diese in das übergebene char Array.
 * Sollen Daten aus einen Block gelesen werden der nicht im Buffer ist, so werden
 * diese vom BlockDevice in den Buffer geschreiben.
 *
 * @param position - Block der gelesen werden soll.
 * @param data - Char Array in dem die Daten geschrieben werden soll.
 * @param BlockDevice - BlockDevice von den die Blöcke gelesen werden.
 * @return - Immer 0
*/
int MyFS::readFromBuffer(u_int32_t position, char* data, BlockDevice* bd) {
    if (bdBuffer.blockNumber == position) {
        memcpy(data, bdBuffer.blockContent, BLOCK_SIZE);
        return 0;
    }
    bd->read(position, bdBuffer.blockContent);
    bdBuffer.blockNumber = position;
    memcpy(data, bdBuffer.blockContent, BLOCK_SIZE);
    return 0;
}

/* Schreibt Daten in den Buffer in Größe der BLOCK_SIZE.
 * Sollte in einem neuen Block geschrieben werden, so werden
 * die Daten die aktuellen Daten im Buffer in das BlockDevice geschrieben.
 * Ist der Buffer an seiner StartPosition (0xFFFF FFFF), werden nur
 * die Daten in char* data in den Buffer geschrieben.
 *
 * @param position - Blockposition in dem geschrieben werden soll.
 * @param data - Daten die in den Buffer geschrieben werden sollen.
 * @param bd - BlockDevice in dem geschrieben wird.
 * @return - Immmer 0.
*/
int MyFS::writeToBuffer(u_int32_t position, char* data, BlockDevice* bd) {
    if (bdBuffer.blockNumber != position) {
        if (bdBuffer.blockNumber != 0xFFFFFFFF) {
            bd->write(bdBuffer.blockNumber, bdBuffer.blockContent);
            bdBuffer.blockNumber = position;
            memcpy(bdBuffer.blockContent, data, BLOCK_SIZE);
            return 0;
        }
        bdBuffer.blockNumber = position;
        memcpy(bdBuffer.blockContent, data, BLOCK_SIZE);
        return 0;
    }
    memcpy(bdBuffer.blockContent, data, BLOCK_SIZE);
    
    return 0;
}

/*
 * Schreibt die Daten im Buffer in das BlockDevice.
 *
 * @param bd - BlockDevice in dem geschrieben wird.
 * @return - Immer 0.
*/
int MyFS::writeBufferToBlockDevice(BlockDevice* bd) {
    bd->write(bdBuffer.blockNumber, bdBuffer.blockContent);
    return 0;
}

/* Sucht in der DMap freie Plätze und schreibt deren PointerWert in das Pointer Array.
 * Um möglichst viele Einträge gleichzeitig im BlockDevice zu ändern, wird in ein Buffer geschrieben.
 * Muss im nächsten DMap-Block gesucht werden, so werden alle Änderungen in das BlockDevice geschrieben und
 * der DMap-Block in dem weiter gesucht werden muss wird geladen.
 * Sobald das Array voll ist (counter == sizeOfArray) werden die letzten Änderungen in das BlockDevice geschrieben
 * und 0 wird zurückGegeben.
 *
 * @param bd - BlockDevice, aus dem gelesen und geschrieben wird.
 * @param pointerArray - Das Array welches ausgefüllt werden soll mit DataPointers.
 * @param sizeOfArray - Die Anzahl der DataPointer im Array die gespeichert werden sollen.
 * @param startPosition - Position ab der gesucht werden soll.
 * @return - 0 bei erfolgreichen füllen, return < 0 wenn nicht genügend DataBlocks vorhanden sind(Sollte !NIEMALS!
 * passieren, da vorher schon
 * geprüft wurde ob überhaupt genug DataBlocks vorhanden sind. Wenn es trotzdem dazu kommt, werden die neuen Einträge
 * nicht rückgängig gemacht
 * und es werden DataBlocks als besetzt markiert, die niemanden gehören.(Achtung, gilt nur für mkfs.myfs! Bei mount.myfs kann nicht
 * geprüft werden für fuseWrite() ob vor dem Schreibvorgang genug Platz da ist.)
*/
u_int32_t MyFS::getFreeDataPointers(BlockDevice* bd, u_int32_t* pointerArray, u_int32_t sizeOfArray,uint32_t startPosition) {
    char dataDMap[BLOCK_SIZE] = {0};
    u_int8_t bitMask;
    u_int32_t counter = 0;
    uint32_t blockOffset = startPosition / (BLOCK_SIZE * 8);
    uint32_t byteOffset = (startPosition - blockOffset * (BLOCK_SIZE * 8)) / 8 ;
    uint32_t bitOffset = startPosition - (blockOffset * (BLOCK_SIZE * 8) + byteOffset * 8);
    for (u_int32_t blockNumber = blockOffset; blockNumber < NUMB_OF_DATA_MAP_BLOCKS; blockNumber++) {
        bd->read(DATA_MAP_START + blockNumber, dataDMap);

        for (int byte = byteOffset; byte < BLOCK_SIZE; byte++) {
            bitMask = 0x80;
            for (int bit = bitOffset; bit < 8; bit++) {
                if ((dataDMap[byte] & bitMask) == 0) {
                    dataDMap[byte] |= bitMask;
                    writeToBuffer(DATA_MAP_START + blockNumber, dataDMap, bd);
                    pointerArray[counter] = bit + byte * 8 + blockNumber * BLOCK_SIZE * 8;
                    counter++;
                    if (counter == sizeOfArray) {
                        writeBufferToBlockDevice(bd);
                        return 0;
                    }
                }
                bitMask >>= 1;
            }
            bitOffset = 0;
        }
        byteOffset = 0;
    }
    return -(EFBIG);
}

/* Gibt die Anzahl der benötigten Blöcke zurück.
 *
 * @param i - Größe der Datei in Byte.
 * @return - Anzahl der Blöcke.
*/
u_int32_t MyFS::getMaxBlocksNeeded(u_int32_t i) {
    if (i % BLOCK_SIZE != 0) {
        return (i / BLOCK_SIZE) + 1;
    } else if (i == 0) {
        return 1;
    }
    return (i / BLOCK_SIZE);
}

/*Erhöht den Wert der vorhandenen Dateien im Filesystem um 1.
 *@param bd - BlockDevice in dem der Wert erhöht werden soll.
*/
void MyFS::superBlockNumFilesIncrease(BlockDevice* bd) {
    char buffer[BLOCK_SIZE] = {0};
    bd->read(SUPER_BLOCK_START, buffer);
    super_block* sb = (super_block*) buffer;
    sb->numbFiles++;
    bd->write(SUPER_BLOCK_START, buffer);
}

//---------------Methoden die nur für mkfs.myfs benötigt werden---------------//

/* Stellt die größe der Blöcke im BlockDevice auf BLOCK_SIZE
 * und füllt alle Blöcke mit Nullen.
 *
 * @param bd - BlockDevice was mit Nullen gefüllt wird
*/
void MyFS::initBlockDevice(BlockDevice* bd) {
    char buffer[BLOCK_SIZE] = {0};
    bd->resize(BLOCK_SIZE);
    for(int i = 0; i < NUMB_OF_BLOCKS; i++) {
        bd->write(i, buffer);
    }
}
    
/* Erstellt ein SuperBlock im BlockDevice mit Informationen über das FileSystem.
 * @param bd - BlockDevice in dem der SuperBlock geschrieben wird.
*/
void MyFS::createSuperBlock(BlockDevice* bd) {
    char structure[BLOCK_SIZE] = {0};
    super_block* sb = (super_block*) structure;
    sb->blockSize = BLOCK_SIZE;                                     /* Block size */
    sb->filesystemSize = FILESYSTEM_SIZE;                           /* Filesystem size */
    sb->numbFiles = 0;                                              /* Number of files starts with 0*/
    sb->first_imap_block = INDOE_MAP_START;                         /* IMAP block start */
    sb->first_dmap_block = DATA_MAP_START;                          /* DMAP block start */
    sb->first_fat_block = FAT_START;                                /* FAT block start */
    sb->first_inode_block = INODE_START;                            /* Inode block start */
    sb->first_data_block = DATA_START;                              /* Data block start */
    bd->write(SUPER_BLOCK_START, (char*) sb);
}
  
/* Erstellt eine neue Inode und schreibt diese in das BlockDevice.
 * Die Metadaten werden von der übergebenen Datei übernommen.(Bis auf access-Time, creation-Time, usedId und grpId)
 * Diese Methode wird nur für mkfs.myfs aufgerufen. Siehe createNewInode() für die mount.myfs Methode.
 *
 * @param bd - BlockDevice in dem die Inode geschrieben wird.
 * @param path - Datei von dem die Metadaten übernommen werden.
 * @param dataPointer - Pointer auf den ersten DatenBlock.
 * @param iNodePointer - Position in welchen InodeBlock die Inode geschrieben werden soll.
 * @return - immer 0
*/
void MyFS::createInodeBlock(BlockDevice* bd, char* path, u_int32_t dataPointer, u_int32_t iNodePointer) {
    char structure[BLOCK_SIZE] = {0};
    struct stat data;
    stat(path, &data);
    inode* node = (inode*) structure;
    
    strncpy(node->file_name, path, sizeof(node->file_name));        /* File name */
    node->st_size = data.st_size;                                   /* total size, in bytes */
    node->user_id = getuid();                                       /* user ID of owner */
    node->grp_id = getgid();                                        /* group ID of owner */
    node->protection = (S_IFREG | 0444);                            /* protection */
    node->atime = time(0);                                          /* time of last access */
    node->ctime = time(0);                                          /* time of last status change */
    node->mtime = data.st_mtime;                                    /* time of last modification */
    node->first_data_block = dataPointer;                           /* number of first file data-block */
    node->st_blocks = getMaxBlocksNeeded(data.st_size);             /* number of 512 byte blocks allocated */
    bd->write(INODE_START + iNodePointer, (char*) node);
}

/* Prüft nach ob eine Dateiname bereits in einem der Inodes im BlockDevice vorhanden ist.
 * Es werden zuerst in der IMap nach belegten Inodes gesucht. Wird ein belegter Platz gefunden, so wird
 * der Dateiname in der Inode mit dem übergebenen Namen verglichen.
 *
 * @param bd - BlockDevice in dem die Namen in den Inodes geprüft werden.
 * @return - true wenn ein Dateiname bereits vorhanden ist, false wenn nicht vorhanden.
*/
int MyFS::checkFileExist(BlockDevice* bd,const char* path) {
    char dataIMap[BLOCK_SIZE] = {0};
    char buffer[BLOCK_SIZE] = {0};
        
    bd->read(INDOE_MAP_START, dataIMap);
        
    u_int8_t bitMask;
        
    for (int byte = 0; byte < NUM_DIR_ENTRIES / 8; byte++) {
        bitMask = 0x80;
            
        for (int bitOffset = 0; bitOffset < 8; bitOffset++) {
            if ((dataIMap[byte] & bitMask) == bitMask) {
                readFromBuffer(INODE_START + bitOffset + byte * 8, buffer, bd);
                inode* node = (inode*) buffer;
                if (strcmp(path, node->file_name) == 0) {
                    return bitOffset + byte * 8;
                }
            }
            bitMask >>= 1;
        }
    }
    return -(ENOENT);
}

/* Prüft ob genug DataBlocks vorhanden sind für eine Datei.
 * Die Methode geht durch die DMap und zählt alle freien Blöcke bis genug freie Blöcke gezählt worden
 * sind um die Datei reinzuschreiben.
 *
 * @param bd - Das BlockDevice indem geprüft werden soll.
 * @param size - Die Dateigröße in bytes.
 * @return - 0 wenn genug DataBlocks vorhanden, return < 0 wenn nicht genug DatenBlöcke vorhanden.
*/
int MyFS::checkFreeDataSize(BlockDevice* bd, u_int32_t size) {
    char dataDMap[BLOCK_SIZE] = {0};
    u_int8_t bitMask = 0x80;
    u_int32_t freeSpace = 0;
    
    for (u_int32_t blockNumber = 0; blockNumber < NUMB_OF_DATA_MAP_BLOCKS; blockNumber++) {
        readFromBuffer(DATA_MAP_START + blockNumber, dataDMap, bd);
        
        for (int byte = 0; byte < BLOCK_SIZE; byte++) {
            bitMask = 0x80;
            
            for (int bit = 0; bit < 8; bit++) {
                if ((dataDMap[byte] & bitMask) == 0) {
                    freeSpace += BLOCK_SIZE;
                }
                if (size < freeSpace) {
                    return 0;
                }
                bitMask >>= 1;
            }
        }
    }
    return -(EFBIG);
}

/* Fügt eine Datei in das übergebene BlockDevice hinzu.
 * Die Methode prüft, ob eine Datei mit gleichen Dateinamen bereits im BlockDevice vorhanden ist,
 * genug freie Datenblöcke vorhanden sind und ein Inodeplatz vorhanden ist. Die Methode erstellt dann ein
 * Array indem alle benötigten DataPointers reinkommen. Es wird dann die Inode erstellt und alle DataPointers
 * werden in die FAT eingetragen. Mithilfe eine FileStream im Binary-Mode wird dann die Datei in ein Char Array
 * "all_data" geschrieben.
 * Um in das BlockDevice zu schreiben, brauchen wir ein zweiten Char Array "singleBlock" mit der Blockgröße des
 * BlockDevice.
 * Dieses Array wird von stücken aus "all_Data" gefüllt und in den DataBlock geschrieben. Die DataPointers geben an
 * wo hingeschrieben wird.
 * Sobald die Datei in das DataBlock vollständig geschreiben wurde, wird im SuperBlock die Anzahl der Files im
 * FileSystem um 1 erhöht.
 * Returnwert 0 bei Erfolg. Bei Fehler eine ErrorNumber.
 *
 * @param bd - BlockDevice in das geschrieben wird.
 * @param path - Dateipfad der Datei die hinzugefügt werden soll.
 * @return - 0 bei Erfolg, return < 0 bei Fehler.
*/
int MyFS::addFile(BlockDevice* bd, char* path) {
    if (checkFileExist(bd, path) >= 0) {
        return -(EEXIST);
    }
    
    struct stat info;
    stat(path, &info);
    
    if (checkFreeDataSize(bd, info.st_size) < 0) {
        return -(EFBIG);
    }
    
    u_int32_t iNodePointer = getFreeInodePointer(bd);
    
    if (iNodePointer == 0xFFFFFFFF) {
        return -(ENOSPC);
    }
    
    u_int32_t blocksToWrite = getMaxBlocksNeeded(info.st_size);
    u_int32_t dataPointers[blocksToWrite];
    u_int32_t sizeOfArray = sizeof(dataPointers) / sizeof(dataPointers[0]);
    
    getFreeDataPointers(bd, dataPointers, sizeOfArray,0);
    createInodeBlock(bd, path, dataPointers[0], iNodePointer);
    writeFatEntries(bd, dataPointers, sizeOfArray);
    
    char *all_data = (char*) malloc(info.st_size);
    
    FILE* f = fopen(path, "rb");
    fread(all_data, info.st_size, 1, f);
    fclose(f);
    
    char singleBlock[BLOCK_SIZE] = {0};
    
    for (u_int32_t dataBlock = 0; dataBlock < blocksToWrite - 1; dataBlock++) {
        memcpy(singleBlock, all_data + BLOCK_SIZE * dataBlock, BLOCK_SIZE);
        bd->write(dataPointers[dataBlock] + DATA_START, singleBlock);
    }
    
    auto rest = info.st_size % BLOCK_SIZE;
    char restData[BLOCK_SIZE] = {0};
    memcpy(restData, all_data + (blocksToWrite - 1) * BLOCK_SIZE, rest);
    bd->write(dataPointers[blocksToWrite - 1] + DATA_START, restData);
    superBlockNumFilesIncrease(bd);
    
    free(all_data);
    
    return 0;
}
