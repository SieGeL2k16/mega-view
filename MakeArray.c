/*
 *  MakeArray.c - Creates a array of Char * out of a given Textline
 *
 *  Original by Spy/tRSi - Rewritten and bugfixed by SieGeL/tRSi
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include "struct_ex.h"
#include "proto.h"

struct StringList
	{
  char *String ;
  struct StringList *next ;
	}*sl1,*sl2 ;

void Free_Structs(void);

BOOL INSERT (char *s)
	{
  if (!sl1)
  	{
		if(!(sl1 = (struct StringList *) AllocPooled (mv_pool,sizeof (struct StringList)))) return(FALSE);
    sl2 = sl1;
  	}
  else
  	{
		if(!(sl2->next = (struct StringList *) AllocPooled (mv_pool,sizeof (struct StringList)))) return(FALSE);
    sl2 = sl2->next;
  	}
  sl2->next = NULL;
  if(!(sl2->String = (char *) AllocVec ((strlen(s)+2),MEMF_PUBLIC|MEMF_CLEAR))) return(FALSE);
  strcpy (sl2->String,s);
	return(TRUE);
	}

void MoveStrings (char **res,struct StringList *sl)
	{
	struct 	StringList *slb;
	char 		**strs;

  slb = sl;
  strs = res;
  while(slb)
  	{
    *strs = slb->String;
    strs++;
    slb = slb->next;
  	}
  strs = NULL;
	}
char **MakeArray (char *s)
	{
	char 	*str1,*str2,**result;
	int 	i;

  sl1 = NULL;
	sl2 = NULL;
  str1 = s;
  i = 0;
		while((str2 = FAMEStrChr(str1,' ')))
  	{
    *str2 = 0;
    if(INSERT (str1)==FALSE) return(NULL);
    i++;
    *str2 = ' ';
    while ((*str2 != 0) && (*str2 == ' '))
    str2++;
    str1 = str2;
  	}
  if (*str1 != 0)
  	{
		if(INSERT (str1)==FALSE) return(NULL);
		}
  i++;
  if(!(result = (char **) AllocVec ((i+1)*4,MEMF_PUBLIC|MEMF_CLEAR))) return(NULL);
  MoveStrings (result,sl1);
  return (result);
	}

void FreeArray (char **strings)
	{
	char 	**strs;
	int 	i;

	i = 0 ;
  strs = strings;
  while (*strs)
  	{
    i++ ;
    FreeVec (*strs);
    strs++ ;
  	}
  i++ ;
	FreeVec(strings);
	Free_Structs();
	strings=NULL;
	}

void Free_Structs(void)
	{
	struct StringList *h=sl1;
	while(sl1)
		{
		h=sl1;
		sl1=sl1->next;
		FreePooled(mv_pool,h,sizeof(struct StringList));
		}
	sl1=NULL;sl2=NULL;
	}
