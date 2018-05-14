//
//  mk.myfs.cpp
//  myfs
//
//  Created by Oliver Waldhorst on 07.09.17.
//  Copyright © 2017 Oliver Waldhorst. All rights reserved.
//

#include "myfs.h"
#include "blockdevice.h"
#include "macros.h"



int main(int argc, char *argv[]) {
    
    // TODO: Implement file system generation & copying of files here
    
    
    //-------------- zum testen -------------------------
    argv[1] = "container.bin";
    argv[2] = "file.txt";
    argv[3] = "text.txt";
    argv[4] = "bigdata.txt";
    argc = 5;
    //---------------------------------------------------
    
    
    
    BlockDevice* bd = new BlockDevice(FILESYSTEM_SIZE);
    bd->create(argv[1]);
    MyFS::Instance()->initBlockDevice(bd);
    MyFS::Instance()->createSuperBlock(bd);
    
    
    for (auto arguments = 2; arguments < argc; arguments++) {
        MyFS::Instance()->addFile(bd, argv[arguments]);
    }
    
    return 0;
}
