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
//  HPLMinimal.mm
//  Lux
//
//  Created by Edward Rudd on 8/19/10.
//  Copyright 2010 OutOfOrder.cc. All rights reserved.
//

#import "HPLMinimal.h"


#include <stdarg.h>

namespace hpl {
	void Warning(const char* fmt, ...) {
		va_list ap;
		va_start(ap, fmt);
        char buff[1024];
        vsnprintf(buff, 1024, fmt, ap);
        NSLog(@"Warn: %s", buff);
		va_end(ap);
	}

	void Error(const char* fmt, ...) {
		va_list ap;
		va_start(ap, fmt);
        char buff[1024];
        vsnprintf(buff, 1024, fmt, ap);
        NSLog(@"Err: %s", buff);
		va_end(ap);
	}

	void Log(const char* fmt, ...) {
		va_list ap;
		va_start(ap, fmt);
        char buff[1024];
        vsnprintf(buff, 1024, fmt, ap);
        NSLog(@"Log: %s", buff);
		va_end(ap);
	}
}
