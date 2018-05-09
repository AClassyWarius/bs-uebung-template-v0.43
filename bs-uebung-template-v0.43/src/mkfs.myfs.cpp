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
    
    
    BlockDevice bd;
    
    
    /*
     33MB = 33554432 Bytes, Container erhält ein Block mit der Größe von 32MB
    */
    bd.operator=(BlockDevice(33554432));
    
    bd.create("container.bin");
    
    
    /*
     Buffer gefüllt mit 0, 1MB groß
    */
    char buffer[1048576] = {0};
    
    
    /*
     Container wird in 32 Blöcke mit einer Größe von je 1MB unterteilt
    */
    bd.resize(1048576);
    
    for(int i = 0; i < 32; i++) {
        bd.write(i, buffer);
    }
    
    //Container wird in 65536 Blöcke eingeteilt mit einer größe von 512 Byte
    bd.resize(512);
    
    MyFS::Instance()->createSuperBlock(bd);
    MyFS::Instance()->createInodeBlock(bd, 1, "file.txt");
    MyFS::Instance()->createInodeBlock(bd, 2, "text.txt");
    MyFS::Instance()->readSuperBlock(bd);
    MyFS::Instance()->readInodeBlock(bd, 1);
    MyFS::Instance()->readInodeBlock(bd, 2);
    
    return 0;
}
