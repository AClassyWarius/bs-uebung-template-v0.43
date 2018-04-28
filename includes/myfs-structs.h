//
//  myfs-structs.h
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#ifndef myfs_structs_h
#define myfs_structs_h

#define NAME_LENGTH 255
#define BLOCK_SIZE 512
#define NUM_DIR_ENTRIES 64
#define NUM_OPEN_FILES 64

// TODO: Add structures of your file system here




struct super_block {
    u_int32_t s_log_block_size;         /* Block size, 512 byte */
    u_int32_t s_log_frag_size;          /* Fragment size, 512 byte * 65536 blocks = 33554432 byte (33 MB) */
    u_int32_t s_inodes_count;           /* Inodes count */
    u_int32_t s_blocks_count;           /* Blocks count */
    u_int32_t s_free_blocks_count;      /* Free blocks count */
    u_int32_t s_free_inodes_count;      /* Free inodes count */
    u_int32_t s_first_inode_block;      /* First Inode Block */
    u_int32_t s_first_data_block;       /* First Data Block */
    u_int32_t s_blocks_per_group;       /* Blocks per group, 4096 blocks */
    u_int32_t s_frags_per_group;        /* Fragments per group, 4096 blocks * 512 byte = 2097152 byte (2MB)*/
    u_int32_t s_inodes_per_group;       /* Inodes per group */
};



struct inode_bitmap {
    
};

/*  A bitmap is a sequence of bits. Each bit represents a specific block (block bitmap) or
    inode (inode bitmap) in the block group. A bit value of 0 indicates that the block/inode
    is free, while a value of 1 indicates that the block/inode is being used. A bitmap alwys
    refers to the block-group it belongs to, and its size must fit in one block.
    Limiting the size of a bitmap to one block also limits the size of a block-group, because
    a bitmap alwys refers to the blocks/indoes in the group it belongs to.
    Consider the blocks bitmap: given a block size of 512 bytes, and knowing that each byte is
    made of 8 bits, we can calculate the maximum number of blocks that the blocks bitmap can
    represent: 8 * 512 = 4096 blocks. Therefore, 4096 blocks is the size of a block-group using
    a 512-byte. So we need 16 block-groups -> (33MB) 33554432 Byte / 512 Byte = 65536 blocks
    65536 blocks / 4 blocks = 16 block-groups
 */
struct data_bitmap {
    
};

struct fat {
    char file_name[16];                     /* short File name */
    u_int32_t inode_number;                 /* Inode number of the file */
};

struct inode {
    char file_name[FILENAME_MAX];           /* File name */
    off_t st_size;                          /* total size, in bytes */
    uid_t user_id;                          /* user ID of owner */
    gid_t grp_id;                           /* group ID of owner */
    mode_t protection;                      /* protection */
    time_t atime;                           /* time of last access */
    time_t mtime;                           /* time of last modification */
    time_t ctime;                           /* time of last status change */
    u_int32_t first_data_block;             /* number of first file data block */
    blkcnt_t st_blocks;                     /* number of 512 byte blocks allocated */
};



#endif /* myfs_structs_h */
