//
//  PersistentStore.m
//  Scanner
//
//  Created by Toan Vuong on 2/5/16.
//  Copyright © 2016 Occipital. All rights reserved.
//
#import "PersistentStore.h"

std::string PersistentStore::get(const char *key)
{
    NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
    NSString* keyString = @(key);
    
    NSString* value = [userDefaults stringForKey:keyString];
    if (value != nil)
    {
        return std::string([value UTF8String]);
    }
    return "";
}

int PersistentStore::getAsInt(const char *key)
{
    std::string val = PersistentStore::get(key);
    if (val != "")
    {
        return std::stoi(val);
    }
    return -1;
}

void PersistentStore::set(const char *key, const char *value)
{
    NSUserDefaults* userDefaults = [NSUserDefaults standardUserDefaults];
    NSString* keyString = @(key);
    NSString* valueString = @(value);
    
    [userDefaults setObject:valueString forKey:keyString];
    [userDefaults synchronize];
}