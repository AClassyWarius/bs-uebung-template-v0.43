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

void editBuffer(string s1, char* buffer, int offset) {
    int i = 0;
    char const *c1 = s1.c_str();
    while (i != s1.length()) {
        buffer[offset] = *c1;
        c1++;
        offset++;
        i++;
    }
}


void createBoot(BlockDevice bd) {
    
    
    char buffer[512] = {0};
    //memset(buffer, 'A', 512);
    
    
    
    editBuffer("datei.txt", buffer, 16);
    editBuffer("kevin.txt", buffer, 32);
    editBuffer("Marius.txt", buffer, 48);
    editBuffer("512 Byte", buffer, 64);
    editBuffer("ggsgsgsgsgs", buffer, 80);
    
    
    
    
    
    bd.write(0, buffer);
    
}



int main(int argc, char *argv[]) {

    // TODO: Implement file system generation & copying of files here
    
    BlockDevice bd;
    
    
    //32MB = 33554432 Bytes, Container erhält ein Block mit der Größe von 32MB
    bd.operator=(BlockDevice(33554432));
    
    bd.create("container.bin");
    
    
    //Buffer gefüllt mit 0, 1MB groß
    char buffer[1048576] = {0};
    //memset(buffer, 0, 1048576);
    
    //Container wird in 32 Blöcke mit einer Größe von je 1MB unterteilt
    bd.resize(1048576);
    
    for(int i = 0; i < 32; i++) {
        bd.write(i, buffer);
    }
    
    //Container wird in 65536 Blöcke eingeteilt mit einer größe von 512 Byte
    bd.resize(512);
    
    
    bd.write(0, buffer);
    
    createBoot(bd);
    
    
    
    /*
    bd.open("container.bin");
    
    char buffer[512];
    
    bd.read(0, buffer);
    
    editBuffer("00000000000000", buffer, 16);
    
    bd.write(0, buffer);
    */
    
    
    
    
    
    
    
    
    
    
    
    
    
        
        
    
    
    
    
    
    return 0;
}
