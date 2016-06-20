#pragma once

struct user_key
{
         char    key[100];
         int     len;
};

void get_user_key(int timestamp, int interval);
