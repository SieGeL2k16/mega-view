/*
 *              FAME mEGA-vIEW V3.x by SieGeL (tRSi/F-iNNOVATiON)
 *
 */

#include <clib/alib_protos.h>					// Protos for amiga.lib
#include <Fame/fameDoorProto.h>
#include <proto/fileid.h>
#include <proto/utility.h>
#include <proto/asl.h>
#include <utility/utility.h>
#include <libraries/fileid.h>					// Die benoetigten Headerfiles und
#include <libraries/fileid_IDDefs.h>	// Definitionen von FileID.library
#include <libraries/asl.h>
#include "global_defines.h"
#include "struct.h"
#include "proto.h"

STATIC 	char 	*Ver="\0$VER: FAME mEGA-vIEW "VER" "__AMIGADATE__"\0",
          		*NO_SPECIAL="\n\n\r[37mYou may not include any special symbols ![m";

STATIC	char LastASLDir[128];					// Stores last used ASL Directory

extern char *_ProgramName;

/*
 * --- Prototypes ----
 */

void __autoopenfail(void) { _XCEXIT(0);}	// Dummy function for SAS
long 	main(void);
void 	CloseLibs(void);
void 	ShowMyTitle(BOOL cls);
void  ShowVersion(BOOL);
void 	ShutDown(long);
void 	MainLoop(void);
void 	wb(char *);
void 	ScanFAMEPathes(BOOL Scanflag);
long 	AskToContinue(char *, long,USHORT);
BOOL  ShowASL(void);

/*
 *  --- Main Entry Point ---
 */

long main(void)
	{
	struct 	RDArgs *rda=NULL;
	long    ArgArray[1]={0L};
  BOOL		spec=FALSE;

	fp1=NULL; fp2=NULL; finfo=NULL;MyPrefs=NULL;							// Set glob. Ptr to ZERO!
	*ActTempDir='\0';*LastASLDir='\0';
	if(rda=ReadArgs("NODE-NR./A/N",ArgArray,rda))
		{
		node=*(LONG *)ArgArray[0];
		SPrintf((STRPTR)FAMEDoorPort,"FAMEDoorPort%ld",node);
		FreeArgs(rda);rda=NULL;
		}
	else
		{
		FreeArgs(rda);
		PrintFault(IoErr(),(FilePart(_ProgramName)));

#ifndef PUBLIC_RELEASE

		Decrypt(XYZ);

#else

		strcpy(XYZ,"Public Release Version");

#endif

		Printf("\n%s is a FAME BBS-Door and is only usable via the BBS !\n\n%s\n\n",(FilePart(_ProgramName)),XYZ);
		exit(0);
		}
	if(!(UtilityBase=(struct Library *) OpenLibrary("utility.library",37L))) { SetIoErr(ERROR_INVALID_RESIDENT_LIBRARY);exit(20);}
	if(!(FAMEBase=(struct FAMELibrary *) 	OpenLibrary(FAMENAME,0))) { SetIoErr(ERROR_INVALID_RESIDENT_LIBRARY);CloseLibs();}
  ASLBase=(struct Library *)OpenLibrary("asl.library",37);

#ifndef PUBLIC_RELEASE

	Decrypt(XYZ);

#else

	strcpy(XYZ,"Public Release Version");

#endif

	if(FIMStart(0,0UL)) CloseLibs();
	if(!(FileIDBase=(struct FileIDBase *)OpenLibrary(FILEIDNAME,8L))) wb("\n\r[37mUnable to open FileID.library V8++ !!![m\n\r");
	if(!(mv_pool=CreatePool(MEMF_CLEAR|MEMF_PUBLIC,51200L,51200L))) wb(NO_MEM);
  if(!(mytemp1=AllocPooled(mv_pool,MAX_BUF))) wb(NO_MEM);
  if(!(mytemp2=AllocPooled(mv_pool,MAX_BUF))) wb(NO_MEM);
  if(!(readbuf=AllocPooled(mv_pool,READ_BUF))) wb(NO_MEM);
	if(!(tempbuf=AllocPooled(mv_pool,READ_BUF))) wb(NO_MEM);
	if(!(FileName=AllocPooled(mv_pool,MAX_BUF))) wb(NO_MEM);
	if(!(MyPrefs=AllocPooled(mv_pool,sizeof(struct MV_Prefs)))) wb(NO_MEM);
	if(!(myanchor=AllocPooled(mv_pool,sizeof(struct AnchorPath)+256))) wb(NO_MEM);
	if(!(ds=AllocPooled(mv_pool,sizeof(struct DateStamp)))) wb(NO_MEM);
	if(!(dt=AllocPooled(mv_pool,sizeof(struct DateTime)))) wb(NO_MEM);
	if(!(finfo=FIAllocFileInfo())) wb(NO_MEM);
	if(!(myfib=AllocDosObject(DOS_FIB,NULL))) wb(NO_MEM);
	myanchor->ap_Strlen=255;
	*mytemp1='\0';
	ShowMyTitle(FALSE);
	ReadSettings();
	CreateTempDir();
  GetCommand(FileName,0,0,0,NR_GetArgument1);				// Get first argument
	if(*FileName)
		{
		if(!Stricmp(FileName,"$")) ShowVersion(FALSE);
		GetCommand(mytemp1,0,0,0,NR_GetArgument2);			// Check for NS
		if(*mytemp1)
			{
			if(!Stricmp(mytemp1,"NS")) MV_FLAGS |=BITDEF_VIEWNS;
			else MV_FLAGS &= ~BITDEF_VIEWNS;
			if(!Stricmp(mytemp1,"NOASL")) MyPrefs->MV_Switches &=~BITDEF_USE_ASL;
			}
		GetCommand(mytemp1,0,0,0,NR_GetArgument3);			// Check if 3rd Parm is used
		if(*mytemp1)
			{
			if(!Stricmp(mytemp1,"NS")) MV_FLAGS |=BITDEF_VIEWNS;
			else MV_FLAGS &= ~BITDEF_VIEWNS;
			if(!Stricmp(mytemp1,"NOASL")) MyPrefs->MV_Switches &=~BITDEF_USE_ASL;
			}
    if(CheckSpecialChars(FileName)==TRUE)
			{
			if(pathlen<LEN_SYSOP)
				{
				PutString(NO_SPECIAL,1);
        *FileName='\0';
  	    }
			else
				{
				spec=TRUE;
        }
			}
    else spec=FALSE;
		*mytemp2='\0';
		if(CheckForPattern(FileName,mytemp2)==FALSE) GetBBSFile();
		else ScanFAMEPathes(spec);
		}
  MainLoop();
	wb("[m");
	}

/*  void MainLoop(void);
 *
 *  This routine handles all files to process and also checks for NS option,
 *  wildcards and of course special symbols. From here the whole file process
 *  will be controlled and started. All routines have to return to this function
 *  without clean up themself!
 */

void MainLoop(void)
	{
	BOOL	running=TRUE,special=FALSE;
	char	*test;

	while(running)
		{
		*mytemp1='\0';*FileName='\0';
		if(MyPrefs->MV_Switches & BITDEF_USE_ASL)
			{
			ShowASL();
			}
		else
			{
			PutStringFormat("\n\r[m[36mEnter Filename to %s [m([33mENTER = Quit[m)[33m: [m",viewtest);
		  if(GetString(FileName,pathlen)<0) wb(LOST_CARRIER);
			}
		if(!*FileName) break;
		if(!Stricmp(FileName,"$")) ShowVersion(TRUE);
    if(CheckSpecialChars(FileName)==TRUE)
			{
			if(pathlen<LEN_SYSOP)
				{
				PutString(NO_SPECIAL,1);
    	  continue;
      	}
			else
				{
				special=TRUE;
				}
			}
		else special=FALSE;
		if(test=FAMEStrChr(FileName,' '))				// Check for NS Option after name
			{
			FileName[strlen(FileName)-strlen(test)]='\0';
			test++;
			while(FAMEStrChr(test,' ')) test++;		// Supports any amount of spaces ;)
			if(!Stricmp(test,"NS")) MV_FLAGS |=BITDEF_VIEWNS;
			else MV_FLAGS &= ~BITDEF_VIEWNS;
			}
		if(CheckForPattern(FileName,mytemp2)==FALSE) GetBBSFile();
		else ScanFAMEPathes(special);
  	}
	}

/*
 *  ShowASL() opens ASL Filerequester and prompt the local user to select a file to
 *  view with mEGA-vIEW. (V3.4)
 */

BOOL ShowASL(void)
	{
 	struct	FileRequester *myreq=NULL;
	BOOL		retcode=FALSE;
	char		titlebuf[256];

	if(!(myreq=AllocAslRequest(ASL_FileRequest,0)))
		{
		NC_PutString("[37mERROR: No memory for Filerequester structure!!!\n\r",1);
		return(FALSE);
		}
	GetCommand("",0,0,0,SR_NodeNumber);
	*mytemp1='\0';*titlebuf='\0';
	SPrintf(mytemp1,"FAME_Node%ld",MyFAMEDoorMsg->fdom_Data2);
	SPrintf(titlebuf,"MV: Select file to %s...",viewtest);
	if(!*LastASLDir) strcpy(LastASLDir,"SYS:");
	if(AslRequestTags(myreq,ASLFR_PubScreenName,mytemp1,ASLFR_TitleText,titlebuf,ASLFR_InitialDrawer,LastASLDir,TAG_DONE)==TRUE)
		{
		if(myreq->fr_Drawer)
			{
			strcpy((STRPTR)FileName,myreq->fr_Drawer);
			strcpy((STRPTR)LastASLDir,myreq->fr_Drawer);
			AddPart((STRPTR)FileName,myreq->fr_File,255);
			}
		else strcpy((STRPTR)FileName,myreq->fr_File);
		SetIoErr(0);
		retcode=TRUE;
		}
	else retcode=FALSE;
  FreeAslRequest(myreq);myreq=NULL;
	return(retcode);
	}

/*
 *  --- Searches all pathes for the pattern *pat ---
 *  If any match will be found, the user is able to select viewing for this
 *  File, after viewing this function jumps back to MainLoop()!
 *  If scanflag==TRUE this function won't use the downloadpathes of FAME but
 *  searches directly in the supplied directory
 */

void ScanFAMEPathes(BOOL scanflag)
	{
	struct FAMEPathes *mypat=fp1;
	long   pathcounter=1,temprc,found=0;
	STATIC char *suchstring1="\r[K\r[36mSearching for [35m%s [36min DL-Path [33m%ld[34m...",
              *suchstring2="\r[K\r[36mSearching for [35m%s[34m...";

	if(!*FileName) return;
	mypat=fp1;
	while(mypat)
		{
		*mytemp1='\0';*mytemp2='\0';
		if(scanflag==FALSE)
			{
			FAMEStrCopy(mypat->DL_Path,mytemp1,MAX_BUF-1);
			AddPart(mytemp1,FileName, MAX_BUF-1);
      }
		else
			{
			FAMEStrCopy(FileName,mytemp1,MAX_BUF-1);
			}
		CheckForPattern(mytemp1,mytemp2);
		if(scanflag==FALSE)
			{
			NC_PutStringFormat(suchstring1,FileName,pathcounter);
			}
		else
			{
			NC_PutStringFormat(suchstring2,FileName);
			}
		MatchFirst(mytemp2,myanchor);
		if(temprc=IoErr())
			{
			MatchEnd(myanchor);
    	if(scanflag==FALSE)
				{
				mypat=mypat->next;
				pathcounter++;
				continue;
				}
			else break;
			}
		if(myanchor->ap_Info.fib_DirEntryType<0)
			{
			found++;
			switch(AskToContinue(myanchor->ap_Info.fib_FileName,myanchor->ap_Info.fib_Size,0))
				{
				case	-2:	MatchEnd(myanchor);
                  wb(LOST_CARRIER);
				case	-1:	MatchEnd(myanchor);
									wb("\r[K\r[37mQuit mEGA-vIEW !");
        case   0:	break;
				case	 1: FAMEStrCopy(myanchor->ap_Buf,FileName,255);
									MatchEnd(myanchor);
									IdentifyFile(FileName);
									return;
				}
			}
		while(1)
			{
			if(scanflag==FALSE)
				{
				NC_PutStringFormat(suchstring1,FileName,pathcounter);
        }
			else
				{
				NC_PutStringFormat(suchstring2,FileName);
				}
			MatchNext(myanchor);
			if(temprc=IoErr()) break;
			if(myanchor->ap_Info.fib_DirEntryType>0) continue;
			found++;
			switch(AskToContinue(myanchor->ap_Info.fib_FileName,myanchor->ap_Info.fib_Size,0))
				{
				case	-2:	MatchEnd(myanchor);
                  wb(LOST_CARRIER);
				case	-1:	MatchEnd(myanchor);
									wb("\r[K\r[37mQuit mEGA-vIEW !");
        case   0:	continue;
				case	 1: FAMEStrCopy(myanchor->ap_Buf,FileName,255);
									MatchEnd(myanchor);
									IdentifyFile(FileName);
									return;
				}
			}
		MatchEnd(myanchor);
		if(scanflag==FALSE)
			{
			mypat=mypat->next;
			pathcounter++;
			}
		else break;
		}
	if(!found) NC_PutString("[37mnothing found !",1);
	else
		{
		char	local[10];
		if(found==1) strcpy(local,"entry");
		else strcpy(local,"entries");
		NC_PutStringFormat("[32mdone [m([36mfound [33m%ld [36m%s[m)\n\r",found,local);
		}
	}

/*
 *  --- Asks if *file with size bytes should be viewed or not
 *
 *  RETURNCODES:	0 = No selected
 *               -1 = Quit selected
 *                1 = Yes selected
 */

long AskToContinue(char *file, long size,USHORT lf)
	{
  char localbuf[256],numbuf[20];

	*localbuf='\0'; *numbuf='\0';
	FAMENumToStr(size,FNSF_GROUPING|FNSF_NUMLOCALE,19,numbuf);
	SPrintf(localbuf,"\r[K\r[36mFound [35m%s [32m([33m%s [36mBytes[32m) [36m- use this file [32m([33mYes[32m/[33mno[32m/[33mquit[32m) [36m? [m",file,numbuf);
	if(GetCom(localbuf,0,0,0,NR_WaitChar)) return(-2);
	switch(MyFAMEDoorMsg->fdom_Data2)
		{
		case	KEY_n:
		case	KEY_N:	NC_PutString("[37mNo !",lf);
									return(0);
		case	KEY_q:
		case	KEY_Q:  NC_PutString("[35mQuit !",lf);
									return(-1);
		default:			NC_PutString("[32mYes !",lf);
									return(1);
		}
	}

/*
 *  --- Displays Title, TRUE with CLS, FALSE without it ---
 */

void ShowMyTitle(BOOL cls)
	{
  char	TitleBuf[202];

	*TitleBuf='\0';
	if(cls==TRUE) NC_PutString("\f\r",1);
	else NC_PutString("\r",1);
	SPrintf(TitleBuf,"FAME mEGA-vIEW [mv%s [36mby SieGeL [m([36mtRSi[m/[36mF[m-[36miNNOVATiON[m)",VER);
  Center(TitleBuf,1,35);
	Center("([36mUse [33m'[32m*[33m' [36mas wildcard[m)",1,0);
	PutString("",1);
	}

/*
 *  --- Prints out Version Information and exit program ---
 */

void ShowVersion(BOOL withtitle)
	{
	if(withtitle==TRUE) ShowMyTitle(TRUE);
	PutString("\r[1A[K\r",1);
	*mytemp1='\0';
	SPrintf(mytemp1,"Last compiled[33m: [36m([m%s %s[36m) - %s [mV%ld.%ld",__DATE__,__TIME__,FILEIDNAME,FileIDBase->LibNode.lib_Version,FileIDBase->LibNode.lib_Revision);
	Center(mytemp1,1,36);
	PutString("",1);
	Center("For Suggestions or Bug-Reports please contact siegel@siegel.in-berlin.de !",0,34);
	wb("\r");
	}

/*
 *  --- Termination Routines ---
 */

void wb(char *err)
	{
	if(*err) NC_PutString(err,1);
	PutStringFormat("[m\n\r%s\n\r",XYZ);
	FIMEnd(0);
	}

void ShutDown(long error)
	{
	RemoveTempDir();
	CloseLibs();
	}

void CloseLibs(void)
	{
	FreeDlPathes();
  FreeFileTypes();

	if(mv_pool)			  DeletePool(mv_pool);mv_pool=NULL;
	if(finfo) 				FIFreeFileInfo(finfo);finfo=NULL;
	if(myfib)         FreeDosObject(DOS_FIB,myfib);myfib=NULL;
	if(UtilityBase)		CloseLibrary(UtilityBase); UtilityBase=NULL;
	if(FileIDBase)		CloseLibrary((struct Library *)FileIDBase); FileIDBase=NULL;
	if(FAMEBase) 			CloseLibrary((struct Library *)FAMEBase); FAMEBase=NULL;
	if(ASLBase)				CloseLibrary((struct Library *)ASLBase); ASLBase=NULL;
	exit(0);
	}
