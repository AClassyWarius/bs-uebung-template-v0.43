//
//  myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 02.08.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

// For documentation of FUSE methods see https://libfuse.github.io/doxygen/structfuse__operations.html

#undef DEBUG

// TODO: Comment this to reduce debug messages
#define DEBUG
#define DEBUG_METHODS
#define DEBUG_RETURN_VALUES

#include "macros.h"

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

int MyFS::fuseGetattr(const char *path, struct stat *statbuf) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseReadlink(const char *path, char *link, size_t size) {
    LOGM();
    return 0;
}

int MyFS::fuseMknod(const char *path, mode_t mode, dev_t dev) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseMkdir(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

int MyFS::fuseUnlink(const char *path) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseRmdir(const char *path) {
    LOGM();
    return 0;
}

int MyFS::fuseSymlink(const char *path, const char *link) {
    LOGM();
    return 0;
}

int MyFS::fuseRename(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

int MyFS::fuseLink(const char *path, const char *newpath) {
    LOGM();
    return 0;
}

int MyFS::fuseChmod(const char *path, mode_t mode) {
    LOGM();
    return 0;
}

int MyFS::fuseChown(const char *path, uid_t uid, gid_t gid) {
    LOGM();
    return 0;
}

int MyFS::fuseTruncate(const char *path, off_t newSize) {
    LOGM();
    return 0;
}

int MyFS::fuseUtime(const char *path, struct utimbuf *ubuf) {
    LOGM();
    return 0;
}

int MyFS::fuseOpen(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!

    RETURN(0);
}

int MyFS::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseStatfs(const char *path, struct statvfs *statInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseFlush(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    return 0;
}

int MyFS::fuseRelease(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseFsync(const char *path, int datasync, struct fuse_file_info *fi) {
    LOGM();
    return 0;
}

int MyFS::fuseListxattr(const char *path, char *list, size_t size) {
    LOGM();
    RETURN(0);
}

int MyFS::fuseRemovexattr(const char *path, const char *name) {
    LOGM();
    RETURN(0);
}

int MyFS::fuseOpendir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
    
    // <<< My new code
}

int MyFS::fuseReleasedir(const char *path, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

int MyFS::fuseFsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
    LOGM();
    RETURN(0);
}

int MyFS::fuseTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
    LOGM();
    RETURN(0);
}

int MyFS::fuseCreate(const char *path, mode_t mode, struct fuse_file_info *fileInfo) {
    LOGM();
    
    // TODO: Implement this!
    
    RETURN(0);
}

void MyFS::fuseDestroy() {
    LOGM();
}

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
        
        // TODO: Implement your initialization methods here!
    }
    
    RETURN(0);
}

#ifdef __APPLE__
int MyFS::fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags, uint32_t x) {
#else
int MyFS::fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
#endif
    LOGM();
    RETURN(0);
}
    
#ifdef __APPLE__
int MyFS::fuseGetxattr(const char *path, const char *name, char *value, size_t size, uint x) {
#else
int MyFS::fuseGetxattr(const char *path, const char *name, char *value, size_t size) {
#endif
    LOGM();
    RETURN(0);
}
        
// TODO: Add your own additional methods here!
    
void MyFS::initBlockDevice(BlockDevice* bd) {
    char buffer[BLOCK_SIZE] = {0};
    bd->resize(BLOCK_SIZE);
    for(int i = 0; i < NUMB_OF_BLOCKS; i++) {
        bd->write(i, buffer);
    }
}

void MyFS::readSuperBlock(BlockDevice* bd) {
    char buffer[BLOCK_SIZE] = {0};
    bd->read(SUPER_BLOCK_START, buffer);
    super_block* sb = (super_block*) buffer;
    
    //---------------------- zum testen ---------------------------------------
    cout << sb->blockSize << endl;
    cout << sb->first_data_block << endl;
    //-------------------------------------------------------------------------
}
    
void MyFS::readInodeBlock(BlockDevice* bd, u_int32_t blockNo) {
    char buffer[BLOCK_SIZE] = {0};
    bd->read(blockNo, buffer);
    inode* node = (inode*) buffer;
    
    //---------------------- zum testen ---------------------------------------
    cout << node->file_name << endl;
    cout << node->st_size << " size of file" << endl;
    cout << ctime(&node->atime) << " time of last access" << endl;
    cout << ctime(&node->mtime) << " time of last modification" << endl;
    cout << ctime(&node->ctime) << " time of last status change" << endl;
    //-------------------------------------------------------------------------
}
    
    
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
    
int MyFS::createInodeBlock(BlockDevice* bd, char* path, u_int32_t dataPointer) {
    char structure[BLOCK_SIZE] = {0};
    struct stat data;
    stat(path, &data);
    inode* node = (inode*) structure;
    
    strncpy(node->file_name, path, sizeof(node->file_name));        /* File name */
    node->st_size = data.st_size;                                   /* total size, in bytes */
    node->user_id = data.st_uid;                                    /* user ID of owner */
    node->grp_id = data.st_gid;                                     /* group ID of owner */
    node->protection = (S_IFREG | 0444);                            /* protection */
    node->atime = time(0);                                          /* time of last access */
    node->ctime = time(0);                                          /* time of last status change */
    node->mtime = data.st_mtime;                                    /* time of last modification */
    node->first_data_block = dataPointer;                           /* number of first file data-block */
    node->st_blocks = getMaxBlocksNeeded(data.st_size);             /* number of 512 byte blocks allocated */
    
    writeInodeToBlockDevice(bd, node);
    return 0;
}
    
int MyFS::writeInodeToBlockDevice(BlockDevice *bd, inode *node) {
    if (checkFileExist(bd, node->file_name)) {
        return -(EEXIST);
    }
    if (checkFreeDataSize(bd, node->st_size) < 0) {
        return -(EFBIG);
    }
    
    u_int32_t pos = getFreeInodePointer(bd);
    if (pos == 0xFFFFFFFF) {
        return -(ENOSPC);
    }
    
    u_int32_t blockNumber = pos;
    char iNodeBlock[BLOCK_SIZE] = {0};
    char* copy = (char*) node;
    
    bd->read(INODE_START + blockNumber, iNodeBlock);
    memcpy(iNodeBlock, copy, BLOCK_SIZE);
    bd->write(INODE_START + blockNumber, iNodeBlock);
    
    return 0;
}

int MyFS::checkFreeDataSize(BlockDevice* bd, u_int32_t size) {
    char dataDMap[BLOCK_SIZE] = {0};
    u_int8_t bitMask = 0x80;
    u_int32_t freeSpace = 0;
    
    for (u_int32_t blockNumber = 0; blockNumber < NUMB_OF_DATA_MAP_BLOCKS; blockNumber++) {
        bd->read(DATA_MAP_START + blockNumber, dataDMap);
        
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
    return -1;
}


bool MyFS::checkFileExist(BlockDevice* bd, char* path) {
    char dataIMap[BLOCK_SIZE] = {0};
    char buffer[BLOCK_SIZE] = {0};
    
    bd->read(INDOE_MAP_START, dataIMap);
    
    u_int8_t bitMask;
    
    for (int byte = 0; byte < NUM_DIR_ENTRIES / 8; byte++) {
        bitMask = 0x80;
        
        for (int bitOffset = 0; bitOffset < 8; bitOffset++) {
            if ((dataIMap[byte] & bitMask) == bitMask) {
                bd->read(INODE_START + byte, buffer);
                inode* node = (inode*) buffer;
                if (strcmp(path, node->file_name) == 0) {
                    return true;
                }
            }
            bitMask >>= 1;
        }
    }
    return false;
}
    
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


u_int32_t MyFS::getFreeDataPointer(BlockDevice* bd) {
    char dataDMap[BLOCK_SIZE] = {0};
    u_int8_t bitMask;
    
    for (u_int32_t blockNumber = 0; blockNumber < NUMB_OF_DATA_MAP_BLOCKS; blockNumber++) {
        bd->read(DATA_MAP_START + blockNumber, dataDMap);
        
        for (int byte = 0; byte < BLOCK_SIZE; byte++) {
            bitMask = 0x80;
            for (int offSet = 0; offSet < 8; offSet++) {
                if ((dataDMap[byte] & bitMask) == 0) {
                    dataDMap[byte] |= bitMask;
                    bd->write(DATA_MAP_START + blockNumber, dataDMap);
                    return (offSet + byte * 8 + blockNumber * BLOCK_SIZE);
                }
                bitMask >>= 1;
            }
        }
    }
    return 0xFFFFFFFF;
}
    
u_int32_t MyFS::getMaxBlocksNeeded(u_int32_t i) {
    if (i % BLOCK_SIZE != 0) {
        return (i / BLOCK_SIZE) + 1;
    }
    return (i / BLOCK_SIZE);
}
    
void MyFS::writeNewFatEntry(BlockDevice *bd, u_int32_t pointer, u_int32_t nextPointer) {
    char table[BLOCK_SIZE] = {0};
    fat* s_fat = (fat*) table;
    
    u_int32_t blockOffset = pointer / BLOCK_SIZE;
    u_int32_t byteOffset = pointer - BLOCK_SIZE + blockOffset;
    
    bd->read(FAT_START + blockOffset, table);
    s_fat->table[byteOffset] = nextPointer;
    bd->write(FAT_START + blockOffset, table);
}
    
void MyFS::superBlockNumFilesIncrease(BlockDevice* bd) {
    char buffer[BLOCK_SIZE] = {0};
    bd->read(SUPER_BLOCK_START, buffer);
    super_block* sb = (super_block*) buffer;
    sb->numbFiles++;
    bd->write(SUPER_BLOCK_START, buffer);
}
    
int MyFS::addFile(BlockDevice* bd, char* path) {
    u_int32_t dataPointer = getFreeDataPointer(bd);
    u_int32_t nextDataPointer;
    
    if (dataPointer == 0xFFFFFFFF) {
        return -1;
    }
    if (createInodeBlock(bd, path, dataPointer) < 0) {
        return -1;
    }
    
    struct stat info;
    stat(path, &info);
    char all_data[info.st_size];
    
    FILE* f = fopen(path, "rb");
    fread(all_data, info.st_size, 1, f);
    fclose(f);
    
    u_int32_t blocksToWrite = getMaxBlocksNeeded(info.st_size);
    char singleBlock[BLOCK_SIZE] = {0};
    
    for (u_int32_t dataBlock = 0; dataBlock < blocksToWrite - 1; dataBlock++) {
        nextDataPointer = getFreeDataPointer(bd);
        writeNewFatEntry(bd, dataPointer, nextDataPointer);
        dataPointer = nextDataPointer;
        memcpy(singleBlock, all_data + BLOCK_SIZE * dataBlock, BLOCK_SIZE);
        bd->write(nextDataPointer + DATA_START, singleBlock);
    }
    
    auto rest = info.st_size % BLOCK_SIZE;
    nextDataPointer = 0xFFFFFFFF;
    char restData[BLOCK_SIZE] = {0};
    memcpy(restData, all_data + (blocksToWrite - 1) * BLOCK_SIZE, rest);
    bd->write(dataPointer + DATA_START, restData);
    
    superBlockNumFilesIncrease(bd);
    
    return 0;
}
            

