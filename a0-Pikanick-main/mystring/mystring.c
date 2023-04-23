#include "mystring.h"
#include <stdio.h>
#include <stdlib.h>
/*
 *   Implement the following functions:
 *
 *   You are NOT allowed to use any standard string functions such as
 *   strlen, strcpy or strcmp or any other string function.
 */

/*
 *  mystrlen() calculates the length of the string s,
 *  not including the terminating character '\0'.
 *  Returns: length of s.
 */
int mystrlen(const char *s) {
  /* Complete the body of the function */
   int counter = 0;
    while(*s!='\0')
    {
        counter++;
        s++;
    }
    return counter;
  //return 0;
}

/*
 *  mystrcpy()  copies the string pointed to by src (including the terminating
 *  character '\0') to the array pointed to by dst.
 *  Returns: a  pointer to the destination string dst.
 */
char *mystrcpy(char *dst, const char *src) {
  /* Complete the body of the function */
  char * tmp= dst;
  while(*src!='\0')
  {
     *tmp = *src;
      tmp++;
      src++;
  }
  *tmp = '\0';
  return dst;
  //return NULL;
}

/*
 * mystrcmp() compares two strings alphabetically
 * Returns:
 *  -1 if s1  < s2
 *      0 if s1 == s2
 *      1 if s1 > s2
 */
int mystrcmp(const char *s1, const char *s2) {
  /* Complete the body of the function */
  int result=0;
  while(*s1!='\0'&&*s2!='\0')
  {
     if (*s1 != *s2) {
            break;
        }
        s1++;
        s2++;
  }
  result=*(const unsigned char*)s1 - *(const unsigned char*)s2;
  if (result  < 0) // s2>s1
  {
    return -1;
  }

  else if (result  > 0) // s2<s1
  {
    return 1;
  }

  else //(result  == 0) // s2=s1
  {
    return 0;
  }
}

/*
 * mystrdup() creates a duplicate of the string pointed to by s.
 * The space for the new string is obtained using malloc.
 * If the new string can not be created, a null pointer is returned.
 * Returns:  a pointer to a new string (the duplicate)
                 or null If the new string could not be created for
                 any reason such as insufficient memory.
 */
char *mystrdup(const char *s1) 
{
  /* Complete the body of the function */
  char *d1;
  int n=mystrlen(s1);
  d1 = (char*) malloc((n+1)*sizeof(char)); // initialize char pointer (string)
  if (d1==NULL)
  {
    return d1;
  }
  else
  {
    return mystrcpy(d1, s1);
  }
}