/*
Copyright (c) 2024 by Tomaz Mascarenhas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdio.h>

#include "test_utilities.h"

void
swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

char*
str_of_array(const unsigned char* const arr, int len) {
    static char str[4][ARR_MAX_SIZE];
    static int i = 0;
    int j;
    int pt;

    i = (i + 1) % 4;

    pt = sprintf(str[i], "{ ");
    for(j = 0; j < len - 1; ++j) {
        pt += sprintf(str[i] + pt, "0x%02x, ", arr[j]);
    }
    pt += sprintf(str[i] + pt, "0x%02x }", arr[len - 1]);
    str[i][pt] = '\0';

    return str[i];
}
