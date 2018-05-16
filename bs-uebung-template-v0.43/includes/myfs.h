//
//  myfs.h
//  myfs
//
//  Created by Oliver Waldhorst on 02.08.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef myfs_h
#define myfs_h

#include <fuse.h>
#include <cmath>
#include <iostream>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

using namespace std;

#include "blockdevice.h"
#include "myfs-structs.h"

class MyFS {
private:
    static MyFS *_instance;
    FILE *logFile;
    
    struct buffer {
        u_int32_t blockNumber = 0xFFFFFFFF;
        char blockContent[BLOCK_SIZE];
    }bdBuffer;
    
public:
    static MyFS *Instance();
    
    // TODO: Add attributes of your file system here
    
    BlockDevice bd_fuse;
    
    MyFS();
    ~MyFS();
    
    // --- Methods called by FUSE ---
    // For Documentation see https://libfuse.github.io/doxygen/structfuse__operations.html
    int fuseGetattr(const char *path, struct stat *statbuf);
    int fuseReadlink(const char *path, char *link, size_t size);
    int fuseMknod(const char *path, mode_t mode, dev_t dev);
    int fuseMkdir(const char *path, mode_t mode);
    int fuseUnlink(const char *path);
    int fuseRmdir(const char *path);
    int fuseSymlink(const char *path, const char *link);
    int fuseRename(const char *path, const char *newpath);
    int fuseLink(const char *path, const char *newpath);
    int fuseChmod(const char *path, mode_t mode);
    int fuseChown(const char *path, uid_t uid, gid_t gid);
    int fuseTruncate(const char *path, off_t newSize);
    int fuseUtime(const char *path, struct utimbuf *ubuf);
    int fuseOpen(const char *path, struct fuse_file_info *fileInfo);
    int fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
    int fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
    int fuseStatfs(const char *path, struct statvfs *statInfo);
    int fuseFlush(const char *path, struct fuse_file_info *fileInfo);
    int fuseRelease(const char *path, struct fuse_file_info *fileInfo);
    int fuseFsync(const char *path, int datasync, struct fuse_file_info *fi);
#ifdef __APPLE__
    int fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags, uint32_t x);
    int fuseGetxattr(const char *path, const char *name, char *value, size_t size, uint x);
#else
    int fuseSetxattr(const char *path, const char *name, const char *value, size_t size, int flags);
    int fuseGetxattr(const char *path, const char *name, char *value, size_t size);
#endif
    void* fuseInit(struct fuse_conn_info *conn);
    int fuseListxattr(const char *path, char *list, size_t size);
    int fuseRemovexattr(const char *path, const char *name);
    int fuseOpendir(const char *path, struct fuse_file_info *fileInfo);
    int fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
    int fuseReleasedir(const char *path, struct fuse_file_info *fileInfo);
    int fuseFsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo);
    int fuseTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo);
    int fuseCreate(const char *, mode_t, struct fuse_file_info *);
    void fuseDestroy();
    
    // TODO: Add methods of your file system here
    
    void initBlockDevice(BlockDevice* bd);
    int readFromBuffer(u_int32_t position, char* data, BlockDevice* bd);
    int writeToBuffer(u_int32_t position, char* data, BlockDevice* bd);
    int writeToBufferToBlockDevice(BlockDevice* bd);
    void createSuperBlock(BlockDevice* bd);
    int createInodeBlock(BlockDevice* bd, char* path, u_int32_t dataPointer, u_int32_t iNodePointer);
    int checkFileExist(BlockDevice* bd,const char* path);
    u_int32_t getFreeInodePointer(BlockDevice* bd);
    int checkFreeDataSize(BlockDevice* bd, u_int32_t size);
    u_int32_t getFreeDataPointers(BlockDevice* bd, u_int32_t* pointerArray, u_int32_t sizeOfArray);
    u_int32_t getMaxBlocksNeeded(u_int32_t i);
    void writeFatEntries(BlockDevice* bd, u_int32_t* pointers, u_int32_t sizeOfArray);
    void superBlockNumFilesIncrease(BlockDevice* bd);
    int addFile(BlockDevice* bd, char* path);
    
    //------------------- Methods for mount.myfs -------------------------------
    
    int getNumbOfFiles();
    inode* getInodesOfFiles(int numbOfFiles);
    u_int32_t getCurrentDataPointer(u_int32_t startPointer,u_int32_t dataBlockNr);
    u_int32_t nextDataPointer(u_int32_t dataPointer);
    
};

#endif /* myfs_h */
