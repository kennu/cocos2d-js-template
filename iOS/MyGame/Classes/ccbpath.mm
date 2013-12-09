//
//  ccbpath.mm
//  MyGame
//
//  Created by Kenneth Falck on 8.12.2013.
//
//

#include <string>
#import "cocos2d.h"
#import <UIKit/UIKit.h>

extern "C" {
   const char *getCCBDirectoryPath() {
        std::string path = cocos2d::CCFileUtils::sharedFileUtils()->getWritablePath();
        NSString *writeablePath = [NSString stringWithCString:path.c_str() encoding:NSASCIIStringEncoding];
        NSString* dirPath = [writeablePath stringByAppendingPathComponent:@"ccb"];
        return [dirPath cStringUsingEncoding:NSASCIIStringEncoding];
    }
}
