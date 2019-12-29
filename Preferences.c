/*
 *  Preferences.c - Contains all Preferences-related Routines
 */

#include <Fame/fameDoorProto.h>
#include <proto/utility.h>
#include <utility/utility.h>
#include "global_defines.h"
#include "struct_ex.h"
#include "proto.h"

void 	ReadSettings(void);
static long CheckTheLine(char *);         // Checks for required keywords in the current line
static void ReadAllOther(void);						// Reads out the FAME Settings


static char *checkarray[MAX_CHECK]={"ARJ_PATH","DMS_PATH","EXE_PATH","LHA_PATH",
																		"LZX_PATH","ZIP_PATH","GUIDE_PATH","SHRINK_PATH",
																		"WARP_PATH","ZOOM_PATH","ZOO_PATH","DMS_FILEID_PATH",
																		"EXE_FILEID_PATH","HTML_PATH","RAR_PATH","TGZ_PATH",
																		"TEMP_PATH","ALLOW_ARCHIVES","WRITE_CALLERSLOG",
																		"DOWNLOAD_LEVEL","ONLY_CHECK","SYSOP_VIEWLEVEL",
																		"USE_ASL_REQUESTER"};

/*
 *  ReadSettings() - Get's all informations out of mega-view.cfg. The cfg-file
 *                   doesn't have to be in a special format, this routine gets
 *                   all infos via ReadArgs() and is in this case very flexible.
 */

void ReadSettings(void)
	{
	BPTR	prefspointer=NULL;
	char	mybuf[256],dummy[200];
  long	retvar=NULL,myarray[1]={0L},mydummy=NULL;
	struct	RDArgs *myargs=NULL;
	struct	CSource mysource;

  *mybuf='\0';*dummy='\0';
	if(!(prefspointer=Open("FAME:ExternEnv/Doors/mEGA-vIEW.cfg",MODE_OLDFILE))) wb("\n\r[37mUnable to open preferences (FAME:ExternEnv/Doors/mEGA-vIEW.cfg) !");
	if(!(myargs=AllocDosObject(DOS_RDARGS, NULL)))
		{
		Close(prefspointer); prefspointer=NULL;
		wb(NO_MEM);
		}
	*mybuf='\0';
	while(FGets(prefspointer,mybuf,255))
		{
		retvar=CheckTheLine(mybuf);
		if(retvar<0)
			{
			*mybuf='\0';*mytemp1='\0';
			continue;
			}
		mysource.CS_Buffer=mybuf;
		mysource.CS_Length=strlen(mybuf);
		mysource.CS_CurChr=0;
		myargs->RDA_Source=mysource;
		myargs->RDA_Flags=RDAF_NOPROMPT;
		*mytemp1='\0';
		strcpy(mytemp1,checkarray[retvar]);
		switch(retvar)
			{
			case	PREFS_DL_LEVEL:
			case	PREFS_ONLY_CHECK:
			case	PREFS_SYSVIEW_LEVEL:  strcat(mytemp1,"/N");
																	break;
			}
		if(!(myargs=ReadArgs(mytemp1,myarray,myargs)))
			{
			Close(prefspointer);prefspointer=NULL;
			FreeDosObject(DOS_RDARGS,myargs);
			PutStringFormat("\n\r[37mCONFIG-ERROR: Entry [35m%s[37m has a wrong value !!!\n\n\rDOS-Error: [32m%ld [37m - ",checkarray[retvar],IoErr());
			Fault(IoErr(),NULL,mytemp1,MAX_BUF-1);
		  wb(mytemp1);
			}
		switch(retvar)
			{
			case 	PREFS_WRITE_CLOG:
    	case	PREFS_ALLOWARCHIVES:
			case	PREFS_USE_ASL:					SPrintf(dummy,"%s",myarray[0]);
						      			          	if(!Stricmp(dummy,"Yes"))
																			{
																			switch(retvar)
																				{
																				case	PREFS_ALLOWARCHIVES:		MyPrefs->MV_Switches |= BITDEF_ALLOWARCHIVES;
                                        	                          	break;
																				case	PREFS_WRITE_CLOG:				MyPrefs->MV_Switches |= BITDEF_WRITE_CLOG;
																																			break;
																				case	PREFS_USE_ASL:					GetCommand("",0,0,0,SR_LogonType);
																																			if((MyFAMEDoorMsg->fdom_Data2!=3) && (ASLBase)) MyPrefs->MV_Switches |= BITDEF_USE_ASL;
                                                                      break;
																				}
                                    	}
																		FreeArgs(myargs);
                                  	break;
			case	PREFS_DL_LEVEL:
			case	PREFS_ONLY_CHECK:
			case	PREFS_SYSVIEW_LEVEL:		mydummy=*(LONG *)myarray[0];
																		if(mydummy<1)
																			{
                                    	Close(prefspointer);prefspointer=NULL;
																			FreeDosObject(DOS_RDARGS,myargs);
																			PutStringFormat("\n\r[37mCONFIG-ERROR: [35m%s[37m must be set to a value greater than Zero !!",checkarray[retvar]);
																			wb("");
																			}
                                  	switch(retvar)
																			{
																			case	PREFS_DL_LEVEL:  			MyPrefs->MV_DL_Level=mydummy;
																																	break;
																			case	PREFS_ONLY_CHECK:			MyPrefs->MV_Check=mydummy;
                                     		                       		break;
																			case	PREFS_SYSVIEW_LEVEL:	MyPrefs->MV_Sysop=mydummy;
                                    															break;
                                    	}
																		FreeArgs(myargs);
                                  	break;

      default:											if(retvar<MAX_ARRAY) SPrintf(MyPrefs->MV_Pathes[retvar],"%s",myarray[0]);
																		FreeArgs(myargs);
                                  	break;
			}
		}
	Close(prefspointer);prefspointer=NULL;
	FreeDosObject(DOS_RDARGS,myargs);
	ReadAllOther();											// Configure meGA-vIEW now...
	FAMEStrCopy(MyPrefs->MV_Pathes[PREFS_TEMPDIR],ActTempDir,255);
	*mybuf='\0';
  SPrintf(mybuf,"MV.%ld",node);
	AddPart(ActTempDir,mybuf,255);			// Built Temp Dir for general usage
	*tempfile='\0';
	SPrintf(tempfile,"MV_TEMP.%ld",node);
	}

/*
 *  Checks *testline for a known keyword, returns offset for checkarray or
 *  -1 to indicate that the current line is no config line !
 */

static long CheckTheLine(char *testline)
	{
	register long	lv=NULL;

	for(lv=0;lv<MAX_CHECK;lv++)
    {
		if(!(Strnicmp(testline,checkarray[lv],strlen(checkarray[lv]))))
			{
			return(lv);
			}
		}
	return(-1);
	}

/*
 *  Reads the rest of the configuration parameters out of FAME structs. If global
 *  Viewing is enabled for the current user, mEGA-vIEW will load in ALL dl-pathes
 *  of ALL conferences to search in all pathes. If this flag is disabled, only all
 *  pathes of the current conference will be scanned.
 */

static void ReadAllOther(void)
	{
	BPTR		dlptr=NULL;
  long  	totalconfs=NULL,lv=NULL;
	char		buffer[256],extbuf[12];

	GetCommand("",0,0,0,NR_AccessLevel);
	if(MyFAMEDoorMsg->fdom_Data2>=MyPrefs->MV_Sysop) pathlen=LEN_SYSOP;	// SysOpAccess
  else pathlen=LEN_NORMAL;
	if(MyFAMEDoorMsg->fdom_Data2>=MyPrefs->MV_Check)
		{
		strcpy(viewtest,"view");
    MyPrefs->MV_Switches |= BITDEF_ALLOW_VIEW;        		// Viewing is allowed
		}
	else
		{
		strcpy(viewtest,"test"); 															// Only test arcs
		}
	GetCommand("",0,0,0,NR_NumLines);
	maxlines=MyFAMEDoorMsg->fdom_Data2-1;										// Max Lines to view
	GetCommand("",0,0,0,NR_ViewFlag);
	if(MyFAMEDoorMsg->fdom_Data2)
		{
		MV_FLAGS |= BITDEF_GLOBVIEW;
    GetCommand("",0,0,0,SR_NumberOfConfs);
		totalconfs=MyFAMEDoorMsg->fdom_Data2;
		}
	else
		{
		MV_FLAGS &= ~BITDEF_GLOBVIEW;
    totalconfs=1;
		}
	*mytemp1='\0';
	GetCommand(mytemp1,0,0,0,AR_DownloadPathI);             // Get 1. DL Path
	if(!*mytemp1) wb("[37mNo Download-Path defined, please report to SysOp !!!\n\r[m");
	if(AddDlPath(mytemp1)==FALSE) wb(NO_MEM);
	*mytemp1='\0';
	GetCommand(mytemp1,0,0,0,AR_AdditioDlPaths);
	if(*mytemp1)
 	  {
		if(dlptr=Open(mytemp1,MODE_OLDFILE))
			{
			*mytemp1='\0';
			while(FGets(dlptr,mytemp1,MAX_BUF-1))
   		  {
				mytemp1[strlen(mytemp1)-1]='\0';
  	    if(AddDlPath(mytemp1)==FALSE)
					{
					Close(dlptr);dlptr=NULL;
					wb(NO_MEM);
					}
				}
			Close(dlptr);dlptr=NULL;
			}
		}
	et1=NULL; et2=NULL;
	if(!(dlptr=Open("FAME:ExternEnv/Doors/mEGA-vIEW_EXT.CFG",MODE_OLDFILE))) return;
	*mytemp1='\0';
	while(FGets(dlptr,mytemp1,255))
		{
		lv=NULL;
		mytemp1[strlen(mytemp1)-1]='\0';
		while(mytemp1[lv]!=' ')
			{
			extbuf[lv]=mytemp1[lv];
			lv++;
			}
		extbuf[lv]='\0';
		while(mytemp1[lv]==' ') lv++;
		FAMEStrMid(mytemp1,buffer,lv+1,-1);
		if(AddFileType(extbuf,buffer)==FALSE)
			{
			Close(dlptr);dlptr=NULL;
			wb(NO_MEM);
			}
		}
	Close(dlptr);dlptr=NULL;
	}

/*
 *  Global Viewing Support - This function loads in ALL available conferences for the current user and puts all
 *  download pathes into a global structure ready to use.
 */