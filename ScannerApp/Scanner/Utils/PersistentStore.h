//
//  PersistentStore.h
//  Scanner
//
//  Created by Toan Vuong on 2/5/16.
//  Copyright © 2016 Occipital. All rights reserved.
//

#include <string>

class PersistentStore
{
public:
    static std::string get(const char *key);
    static int getAsInt(const char *key);
    static void set(const char *key, const char *value);
};
