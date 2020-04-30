// The MIT License (MIT)
// 
// Copyright (c) 2019 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "compare.h"

/* Function: compare.c
 * Parameters:
 * Returns: Integer that 
 * Purpose:
*/
int compare(char IMG_Name[11], char* input)
{
  //Copy inputfile to a temporary variable
  char input_copy[13];
  strncpy(input_copy, input, 13);
  
  char expanded_name[12];
  memset( expanded_name, ' ', 12 );

  char *token = strtok( input_copy, "." );

  strncpy( expanded_name, token, strlen( token ) );

  token = strtok( NULL, "." );

  if( token )
  {
    strncpy( (char*)(expanded_name+8), token, strlen(token ) );
  }

  expanded_name[11] = '\0';

  int i;
  for( i = 0; i < 11; i++ )
  {
    expanded_name[i] = toupper( expanded_name[i] );
  }
  //Check if inputname matches the filename. If true, return 1 to main function, else return 0.
  if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
    return 1;
  return 0;
}
