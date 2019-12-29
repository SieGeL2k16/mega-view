/*
 *  ExternSupport.c - Contains all routines required for external Support
 */

#include <proto/utility.h>
#include <utility/utility.h>
#include <clib/alib_protos.h>						// Protos for amiga.lib
#include <fame/famedoorcommands.h>
#include <Fame/fameDoorProto.h>
#include <proto/fileid.h>
#include <proto/fame.h>
#include <libraries/fileid.h>
#include <libraries/fileid_IDDefs.h>
#include "global_defines.h"
#include "struct_ex.h"
#include "proto.h"

/* --- Prototypes --- */

long CheckExternal(char *);
void Handle(char *s, char *d,char *fullpath);

/*
 *  This will be used in further updates (if they will ever come...)
 */

long CheckExternal(char *fullpath)
	{
	long  	lv=NULL,retcode;
	char  	buf[256];
	struct  ExtTypes *ptr=et1;

	if(finfo->FI_ID) retcode=SHOW_HEX;
	else retcode=SHOW_TEXT;
	*mytemp1=*buf='\0';
	strcpy(mytemp1,FilePart(fullpath));
	lv=strlen(mytemp1);
	while(mytemp1[lv]!='.')
		{
		lv--;
		if(lv<0) return(retcode);
		}
	FAMEStrMid(mytemp1,buf,lv+2,-1);
	if(!Stricmp(buf,"MP3")) 					// FileID.library has problems detecting MP3, this
		{																// may solve the problem completly ;)
		ReadMP3(fullpath);
		return(SHOW_EXT);
		}
	ptr=et1;
	lv=FALSE;
	if(!et1)													// No external Support
		{
		return(retcode);
		}
	while(ptr)
		{
		if(!Stricmp(buf,ptr->Extension))
			{
			lv=TRUE;
			break;
			}
		else ptr=ptr->next;
		}
	if(lv==FALSE) return(retcode);
	NC_PutStringFormat("\n\n\r[36mUsing external Program for [33m%s[34m...[m\n\n\r",finfo->FI_Description);
	Handle(ptr->Command,mytemp1,fullpath);
	lv=NULL;
  if(lv=MyExecute(mytemp1))
		{
		PrintDosError(ptr->Extension,lv,FALSE,FALSE);
		return(retcode);
		}
  NC_PutString("",1);
	return(SHOW_EXT);
	}

/*
 *  Replaces '%O' or '%F' with their according values. Taken from World-Clock code.
 */

void Handle(char *s, char *d,char *fullpath)
	{
	char *help;

	*mytemp2='\0';
	SPrintf(mytemp2,"PIPE:%s",tempfile);
	while(*s)
		{
		if(*s=='%')
			{
			*s++;
			switch(*s)
				{
				case	'F': 	help=fullpath;
						break;
				case	'O':	help=mytemp2;
						break;
				default	:	*help=NULL; *s--;
				}
				if(help) while(*help)	*d++=*help++;
				if(*s) *s++;
			}
		if(*s)	*d++=*s++;
		}
	*d='\0';
	}
