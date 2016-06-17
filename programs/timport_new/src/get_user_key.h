#pragma once

struct user_key
{
         char    key[100];
         int     keyLen;
};

void getUserKey(int timestamp, int timeInterval);
