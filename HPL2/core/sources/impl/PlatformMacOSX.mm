/*
 * Copyright © 2011-2020 Frictional Games
 * 
 * This file is part of Amnesia: A Machine For Pigs.
 * 
 * Amnesia: A Machine For Pigs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. 

 * Amnesia: A Machine For Pigs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: A Machine For Pigs.  If not, see <https://www.gnu.org/licenses/>.
 */

//
//  PlatformMacOSX.m
//  Lux
//
//  Created by Edward Rudd on 6/27/09.
//  Copyright 2009 OutOfOrder.cc. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include "system/SystemTypes.h"
#include "system/Platform.h"
#include "system/String.h"

namespace hpl {
    tString cPlatform::GetDataDir()
    {
        tString sTemp;
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        NSString *dataPath;
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *startPath = [[bundle infoDictionary] objectForKey:@"StartFolder"];
        if ([startPath isEqualToString:@"PARENT"]) {
            dataPath = [bundle bundlePath];
            dataPath = [dataPath stringByDeletingLastPathComponent];
        } else { // default is resource
            dataPath = [bundle resourcePath];
        }
        sTemp = [dataPath UTF8String];

        [pool drain];
        return sTemp;
    }

    void OSXAlertBox(eMsgBoxType eType, tString caption, tString message)
    {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSAlert *alert = [NSAlert alertWithMessageText:[NSString stringWithFormat:@"%s", caption.c_str()]
                                         defaultButton:nil // will default to localized OK
                                       alternateButton:nil
                                           otherButton:nil
                             informativeTextWithFormat:@"%s", message.c_str()];
        switch (eType) {
            case eMsgBoxType_Error:
                [alert setAlertStyle:NSCriticalAlertStyle];
                break;
            case eMsgBoxType_Warning:
                [alert setAlertStyle:NSWarningAlertStyle];
                break;
            case eMsgBoxType_Info:
            case eMsgBoxType_Default:
                [alert setAlertStyle:NSInformationalAlertStyle];
        }
        [alert runModal];
        [pool drain];
    }
    
    void OSXLaunchURL(tString url)
    {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSWorkspace *ws = [NSWorkspace sharedWorkspace];
        [ws openURL:[NSURL URLWithString:[NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding]]];
        [pool drain];
    }
    
    bool OSXOpenFile(tString path)
    {
        bool ret = false;
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSWorkspace *ws = [NSWorkspace sharedWorkspace];
        ret = [ws openFile:[NSString stringWithUTF8String: path.c_str()]] == YES;
        [pool drain];
        return ret;
    }
    
    bool OSXRunProgram(tString path, tString args)
    {
        bool ret = false;
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSWorkspace *ws = [NSWorkspace sharedWorkspace];
        NSString *tPath = [NSString stringWithCString:path.c_str() encoding:NSUTF8StringEncoding];
        NSError *err;
        NSLog(@"Type of %@ is %@",tPath, [ws typeOfFile:[tPath stringByStandardizingPath] error:&err]);
        ret = [ws launchApplication:tPath] == YES;
        [pool drain];
        return ret;
    }
    
    tWString OSXGetAppDataDir()
    {
        tWString ret;

        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if ([dirs count] > 0) {
            NSString *str = [dirs objectAtIndex:0];
            const char* dir = [str fileSystemRepresentation];
            if (dir) {
                ret = cString::UTF8ToWChar(dir);
                if (cString::GetLastCharW(ret) != _W("/")) {
					ret += _W("/");
				}
            }
        }
        [pool drain];

        return ret;
    }
}
