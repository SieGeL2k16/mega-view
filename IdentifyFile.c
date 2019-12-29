/*
 *  IdentifyFile.c - Contains everything about Identification of files
 */

#include <clib/alib_protos.h>					// Protos for amiga.lib
#include <fame/famedoorcommands.h>
#include <Fame/fameDoorProto.h>
#include <proto/fileid.h>
#include <proto/utility.h>
#include <proto/fame.h>
#include <proto/icon.h>
#include <utility/utility.h>
#include <libraries/fileid.h>
#include <libraries/fileid_IDDefs.h>
#include "global_defines.h"
#include "struct_ex.h"
#include "proto.h"

STATIC char searchstring[80];	// Global searchstring buffer to allow continous searchings
long	 zipret=NULL;

/*
 *  Prototypes
 */

void 		GetBBSFile(void);
void 		IdentifyFile(char *);
void		ShowText(char *file);
ULONG 	ZippySearch(BPTR fp, long currpos);
long 		AskForTest(char *file, BOOL isarc);
void		ShowHex(char *);
void 		GetVersionInfo(long fidnum,char *name);               // V3.1ß
void		ConvHTML(char *fullname);															// V3.2ß
void 		ReadMP3(char *fullname);															// V3.4

STATIC 	long AskForPause(char *,long,long,long,BPTR,BOOL);		// Pause Prompt for ShowText()
STATIC	void IconInfo(char *path);														// Displays ToolTypes of icons

/*
 *  This is the main identification function. First the Executable will be
 *  checked by FileID.library, then mEGA-vIEW checks if the file has the
 *  Restricted comment set or has the same name like the current user.datas
 *  have. If all these checks are passed successfully, the file will be
 *  processed inside the switch/case pair. All functions must return without
 *  exiting to make sure that all functions are correctly quit !
 */

void IdentifyFile(char *fullpath)
	{
  long	idnum=NULL,error=NULL,packsel=NULL;
	UWORD	gfc=0;

	if(!*fullpath) return;
	*searchstring='\0';									// First start, clear searchbuffer
  FIIdentifyFromName(finfo,fullpath);
	gfc=finfo->FI_GlobalFileClass;
	idnum=finfo->FI_ID;
	if(gfc & FIGFCF_EXECUTABLE) idnum=FID_UNKNOWNEXE;
	switch(TestTheFile(fullpath))			// Checks for Restricted etc.
		{
		case	-1: PrintDosError(fullpath,IoErr(),FALSE,FALSE);
							return;
		case  1:	NC_PutString("\n\n\r[37mFile is restricted by the SysOp ! ",0);
							if(MyPrefs->MV_Switches & BITDEF_WRITE_CLOG)
								{
								PutString("Adding comment to callerslog[34m...",0);
								*mytemp1='\0';
								SPrintf(mytemp1,"mEGA-vIEW: ACCESS DENIED FOR FILE %s !\n",fullpath);
								PutCommand(mytemp1,0,0,0,CF_DoCallersLog);
								PutString("[32mdone !\n\r",1);
								}
							else NC_PutString("\n",1);
							return;
		}
	*mytemp2='\0';
  switch(idnum)
		{
		case	FID_UNKNOWNEXE:   if(CheckPacker(MyPrefs->MV_Pathes[PREFS_EXE],TRUE)==FALSE) break;
														if((error=AskForTest(fullpath,FALSE))==FILE_QUIT) return;
														else
															if (error==FILE_VIEW) ShowHex(fullpath);
															else
																{
																SPrintf(mytemp2,"run %s >PIPE:%s %lc%s%lc",MyPrefs->MV_Pathes[PREFS_EXE],tempfile,34,fullpath,34);
																if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_EXE]),error,FALSE,FALSE);
                                NC_PutString("",1);
																}
														GetVersionInfo(idnum,fullpath);
														PutString("",1);
														GetFileID(fullpath,PREFS_EXE);
														break;

		case	FID_ZIP:
		case  FID_LHA:
		case	FID_LZX:					switch(idnum)
															{
															case	FID_ZIP:	packsel=PREFS_ZIP;
																							break;
															case	FID_LZX:	packsel=PREFS_LZX;
																							break;
															case	FID_LHA:	packsel=PREFS_LHA;
                            								  break;
                              }
														if(CheckPacker(MyPrefs->MV_Pathes[packsel],TRUE)==FALSE) break;
                            if((error=AskForTest(fullpath,TRUE))==FILE_QUIT) return;
														if(error==FILE_TEST)
																{
																if(idnum==FID_ZIP) SPrintf(mytemp2,"run %s >PIPE:%s -t %lc%s%lc",MyPrefs->MV_Pathes[PREFS_ZIP],tempfile,34,fullpath,34);
																else SPrintf(mytemp2,"run %s >PIPE:%s -M t %lc%s%lc",MyPrefs->MV_Pathes[packsel],tempfile,34,fullpath,34);
																if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[packsel]),error,FALSE,FALSE);
																}
														else ViewArchive(fullpath,packsel);
														break;

		case	FID_DMS:					if(CheckPacker(MyPrefs->MV_Pathes[PREFS_DMS],TRUE)==FALSE) break;
                            if((error=AskForTest(fullpath,TRUE))==FILE_QUIT) return;
                            if(error==FILE_TEST) SPrintf(mytemp2,"run %s >PIPE:%s TEST %lc%s%lc",MyPrefs->MV_Pathes[PREFS_DMS],tempfile,34,fullpath,34);
                            else SPrintf(mytemp2,"run %s >PIPE:%s VIEW %lc%s%lc",MyPrefs->MV_Pathes[PREFS_DMS],tempfile,34,fullpath,34);
														if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_DMS]),error,FALSE,FALSE);
														GetFileID(fullpath,PREFS_DMS);
														break;

		case	FID_ARJ:					if(CheckPacker(MyPrefs->MV_Pathes[PREFS_ARJ],TRUE)==FALSE) break;
														if((error=AskForTest(fullpath,TRUE))==FILE_QUIT) return;
														if(error==FILE_TEST) SPrintf(mytemp2,"run %s >PIPE:%s t %lc%s%lc",MyPrefs->MV_Pathes[PREFS_ARJ],tempfile,34,fullpath,34);
														else SPrintf(mytemp2,"run %s >PIPE:%s l %lc%s%lc",MyPrefs->MV_Pathes[PREFS_ARJ],tempfile,34,fullpath,34);
														if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_ARJ]),error,FALSE,FALSE);
														break;

		case	FID_ZOO:					if(CheckPacker(MyPrefs->MV_Pathes[PREFS_ZOO],TRUE)==FALSE) break;
														if((error=AskForTest(fullpath,TRUE))==FILE_QUIT) return;
														if(error==FILE_TEST) SPrintf(mytemp2,"run %s >PIPE:%s -test %lc%s%lc",MyPrefs->MV_Pathes[PREFS_ZOO],tempfile,34,fullpath,34);
														else SPrintf(mytemp2,"run %s >PIPE:%s -list %lc%s%lc",MyPrefs->MV_Pathes[PREFS_ZOO],tempfile,34,fullpath,34);
														if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_ZOO]),error,FALSE,FALSE);
														break;

		case	FID_ZOOM:					if(CheckPacker(MyPrefs->MV_Pathes[PREFS_ZOOM],TRUE)==FALSE) break;
														SPrintf(mytemp2,"run %s >PIPE:%s %lc%s%lc",MyPrefs->MV_Pathes[PREFS_ZOOM],tempfile,34,fullpath,34);
														if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_ZOOM]),error,FALSE,FALSE);
														break;

		case	FID_WARP:					if(CheckPacker(MyPrefs->MV_Pathes[PREFS_WARP],TRUE)==FALSE) break;
														SPrintf(mytemp2,"run %s >PIPE:%s %lc%s%lc",MyPrefs->MV_Pathes[PREFS_WARP],tempfile,34,fullpath,34);
														if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_WARP]),error,FALSE,FALSE);
														break;

		case	FID_SHRINK:				if(CheckPacker(MyPrefs->MV_Pathes[PREFS_SHRINK],TRUE)==FALSE) break;
														if((error=AskForTest(fullpath,TRUE))==FILE_QUIT) return;
														if(error==FILE_TEST) SPrintf(mytemp2,"run %s >PIPE:%s t %lc%s%lc",MyPrefs->MV_Pathes[PREFS_SHRINK],tempfile,34,fullpath,34);
														else SPrintf(mytemp2,"run %s >PIPE:%s l %lc%s%lc",MyPrefs->MV_Pathes[PREFS_SHRINK],tempfile,34,fullpath,34);
														if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_SHRINK]),error,FALSE,FALSE);
														break;

    case	FID_AMIGAGUIDE:		if(CheckPacker(MyPrefs->MV_Pathes[PREFS_GUIDE],TRUE)==FALSE)
                              {
                              if(AskBool("[36mSysOp decided to disable Guide Converting to ANSI [34m- [36mView anyway",FALSE)==FALSE) return;
															else ShowText(fullpath);
															return;
                              }
                            NC_PutStringFormat("\n\n\r[36mConverting [32m%s [36mto an ANSI-File[34m...",FilePart(fullpath));
                            *mytemp1='\0';
                            SPrintf(mytemp1,"%s%s.txt",ActTempDir,FilePart(fullpath));
														SPrintf(mytemp2,"%s %lc%s%lc %lc%s%lc",MyPrefs->MV_Pathes[PREFS_GUIDE],34,fullpath,34,34,mytemp1,34);
														if(error=SystemTagList(mytemp2,NULL))
															{
															PrintDosError(MyPrefs->MV_Pathes[PREFS_GUIDE],error,TRUE,FALSE);
															return;
                              }
                            NC_PutString("[32mdone ![m",1);
														ShowText(mytemp1);
														MyDeleteFile(mytemp1);
														return;
	                          break;

		case FID_DISKICON:
		case FID_TRASHCANICON:
		case FID_DEVICEICON:
		case FID_KICKICON:
		case FID_APPICON:
		case FID_UNKNOWNICON:
		case FID_DRAWERICON:
		case FID_TOOLICON:
		case FID_PROJECTICON:		IconInfo(fullpath);
														break;
		case FID_LIBRARY:
		case FID_DEVICEDRIVER:
		case FID_DATATYPE:			PutString("",1);
														GetVersionInfo(idnum,fullpath);
														break;

		case FID_RAR:           if(CheckPacker(MyPrefs->MV_Pathes[PREFS_RAR],TRUE)==FALSE) break;
														if((error=AskForTest(fullpath,TRUE))==FILE_QUIT) return;
														if(error==FILE_TEST) SPrintf(mytemp2,"run %s >PIPE:%s t %lc%s%lc",MyPrefs->MV_Pathes[PREFS_RAR],tempfile,34,fullpath,34);
														else SPrintf(mytemp2,"run %s >PIPE:%s v %lc%s%lc",MyPrefs->MV_Pathes[PREFS_RAR],tempfile,34,fullpath,34);
														if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_RAR]),error,FALSE,FALSE);
														break;

		case FID_TARGZ:					if(CheckPacker(MyPrefs->MV_Pathes[PREFS_TGZ],TRUE)==FALSE) break;
														SPrintf(mytemp2,"run %s >PIPE:%s v %lc%s%lc",MyPrefs->MV_Pathes[PREFS_TGZ],tempfile,34,fullpath,34);
														if(error=MyExecute(mytemp2)) PrintDosError(FilePart(MyPrefs->MV_Pathes[PREFS_TGZ]),error,FALSE,FALSE);
														break;

		case FID_HTML:					ConvHTML(fullpath);
														break;

		case FID_MP3:						ReadMP3(fullpath);
														break;

  	default:                NC_PutString("[m",1);
														switch(CheckExternal(fullpath))
                            	{
															case	SHOW_TEXT:  ShowText(fullpath);
                                                break;
															case	SHOW_HEX:   ShowHex(fullpath);
																								break;
															}
														PutStringFormat("%lc[m\n\r",(char)15);
														break;
		}
	*searchstring='\0';				// Finished the actual text, clear buffer again
	zipret=NULL;
	}

/*
 *  GetBBSFile() will be called when the user enters only a name without any
 *  pattern or directory path. The function checks all pathes of the current
 *  download pathes for the given file and builts the complete path into the
 *  FileName variable. After this IdentifyFile() will be called.
 */

void GetBBSFile(void)
	{
	struct FAMEPathes *mypath=fp1;

  mypath=fp1;
	while(mypath)
		{
		*mytemp1='\0';
    FAMEStrCopy(mypath->DL_Path,mytemp1,MAX_BUF-1);
		AddPart(mytemp1,FileName,MAX_BUF-1);
		if(CheckPacker(mytemp1,FALSE)==TRUE)
			{
      FAMEStrCopy(mytemp1,FileName,MAX_BUF-1);
			IdentifyFile(FileName);
			return;
			}
		mypath=mypath->next;
		}
	PrintDosError(FileName,ERROR_OBJECT_NOT_FOUND,FALSE,FALSE);
	}

/*
 *  ShowText() is the main display routine, it shows all unknown files as long
 *  as ShowHex() is not called. All possible errors will be handled inside this
 *  function and after clean up return in any case to the calling function.
 */

void ShowText(char *file)
	{
	BPTR		fileptr=NULL;
	long		linecount=NULL,mydummy=NULL,totalsize,shown=NULL,screenstart,
					bufsize,prevpage;
	BOOL		stopit=FALSE;

	NC_PutString("[m",1);
	if(!(fileptr=Open(file,MODE_OLDFILE)))
		{
    PrintDosError(FilePart(file),IoErr(),FALSE,FALSE);
		return;
  	}
	screenstart=prevpage=Seek(fileptr,0L,OFFSET_BEGINNING);
	ExamineFH(fileptr,myfib);
	totalsize=myfib->fib_Size;
	SetIoErr(0);
	while((FGets(fileptr,readbuf,READ_BUF-1)) && (stopit==FALSE))
		{
		bufsize=strlen(readbuf);
		shown+=bufsize;
		if(readbuf[bufsize-1]=='\n') strcat(readbuf,"\r");
		NC_PutString(readbuf,0);
		FAMEStrCopy(readbuf,tempbuf,READ_BUF-1);
		MyCutANSI(tempbuf);
		if(strlen(tempbuf)>=79) linecount+=(strlen(tempbuf)/78);
		else linecount++;
    mydummy=CheckForCTRL();
		if(mydummy==CTRL_C)
			{
			stopit=TRUE;
			SetIoErr(0);
			continue;
			}
		else if(mydummy==CTRL_L) linecount=0;
    switch(AskForPause(FilePart(file),totalsize,shown,linecount,fileptr,FALSE))
			{
			case	VIEW_NO_PROMPT:			continue;

			case	VIEW_LOST_CARRIER:	Close(fileptr);
																fileptr=NULL;
                        				wb(LOST_CARRIER);

			case 	VIEW_QUIT: 					stopit=TRUE;
																SetIoErr(0);
																continue;

			case	VIEW_CONT:					linecount=0;
	                              if(prevpage==screenstart) screenstart=Seek(fileptr,0L,OFFSET_CURRENT);
																else
																	{
																	prevpage=screenstart;
																	screenstart=Seek(fileptr,0L,OFFSET_CURRENT);
																	}
																continue;

			case	VIEW_HELP:					shown=screenstart;
																Seek(fileptr,screenstart,OFFSET_BEGINNING);
																linecount=0;
                                continue;

			case  VIEW_ZIPPY:					shown=screenstart=zipret;
																zipret=Seek(fileptr,0L,OFFSET_CURRENT);
																linecount=1;
                                continue;

			case	VIEW_UP:            screenstart=shown=prevpage;
																Seek(fileptr,prevpage,OFFSET_BEGINNING);
																linecount=0;
																continue;

			case	VIEW_DOWN:					linecount--;
																screenstart+=bufsize;
																continue;
			}
		}
  if(IoErr()>0) PrintDosError("\n\rShowText()",IoErr(),FALSE,FALSE);
	Close(fileptr);fileptr=NULL;
	MV_FLAGS &= ~BITDEF_VIEWNS;
	NC_PutStringFormat("%lc\r[m",(char)15);
	}

/*
 *  Asks to continue Display, also NS, Z, Q and ? are available
 *  RETURN: -2 = Lost Carrier !
 *          -1 = Stop displaying (Q)
 *           0 = No pause prompt required (continue viewing)
 *           1 = View NonStop (NS)
 *           2 = Zippy search inside the text (Z)
 *           3 = View Online Help
 *           4 = Continue Viewing (ENTER)
 *           5 = View up (CRSR UP)
 *           6 = View Down (CRSR DOWN)
 */

STATIC long AskForPause(char *filename, long fsize, long currsize, long currlines,BPTR fileptr,BOOL HexMode)
	{
	char	helpbuf[202];

	if((currlines<maxlines) || (MV_FLAGS & BITDEF_VIEWNS)) return(VIEW_NO_PROMPT);
  while(1)
		{
		NC_PutStringFormat("\r[K\r[m[36mFile[33m: [35m%s [32m([33m%ld[32m/[33m%ld [36mBytes[32m) [34m- ",filename,currsize,fsize);
		NC_PutString("[36mC[34m,[36mQ[34m,[36mNS[34m,[36mZ[34m,[36m?[34m,[36mUP[34m/[36mDOWN[34m,[36mENTER[33m:[m ",0);
		if(GetCom("",0,0,0,NR_WaitChar)<0) return(VIEW_LOST_CARRIER);
  	switch(MyFAMEDoorMsg->fdom_Data2)
			{
			case	KEY_c		:
			case	KEY_C		: 	NC_PutString("\f\r[m",1);
												return(VIEW_CONT);
			case  KEY_q 	:
			case	KEY_Q 	:		NC_PutStringFormat("%lc\n\r[m",(char)15);
												return(VIEW_QUIT);

			case	KEY_Z 	:
			case	KEY_z 	: 	if(HexMode==TRUE)
													{
													NC_PutString("\r[K\r",0);
													Center("Zippy-Search not possible in Hex mode ! [35m[[34mENTER[35m][m",0,37);
                         	Get_Key(FALSE);
													continue;
													}
												if(zipret) zipret=ZippySearch(fileptr,zipret);
                      	else zipret=ZippySearch(fileptr,currsize);
												switch(zipret)
													{
													case	-1:	return(VIEW_LOST_CARRIER);
													case	-2:	continue;
													default:  return(VIEW_ZIPPY);
													}

			case	KEY_ASK	: 	NC_PutString("\f\r[m",1);
												*helpbuf='\0';
    	                	strcpy(helpbuf,_ProgramName);
												helpbuf[strlen(helpbuf)-strlen(FilePart(helpbuf))]='\0';
        	            	AddPart(helpbuf,"TEXTS/VIEW_HELP.TXT",201);
												PutCom(helpbuf,0,0,0,CF_ShowText);
												Get_Key(TRUE);
												NC_PutString("\f\r[m",1);
												return(VIEW_HELP);

			case	KEY_n:
			case	KEY_N:			NC_PutStringFormat("[32m%lc[m",MyFAMEDoorMsg->fdom_Data2);
      	              	if(GetCom("",0,0,0,NR_WaitChar)<0) return(VIEW_LOST_CARRIER);
												if(MyFAMEDoorMsg->fdom_Data2==KEY_S || MyFAMEDoorMsg->fdom_Data2==KEY_s) MV_FLAGS |=BITDEF_VIEWNS;
												else continue;

			case	CRSR_UP:  	NC_PutString("\f\r[m",1);
                      	return(VIEW_UP);

			case	CRSR_DOWN:  NC_PutString("\r[K\r[m",0);
												return(VIEW_DOWN);

			default:					NC_PutString("\r[K\r",0);
												return(VIEW_CONT);
			}
		}
	}

/*
 *  ZippySearch() searches the Textfile for a substring and prints it out
 *  in Green if any match is found. Returns 0 if function could be
 *  processed successfully. Returns -1 if an error occures or -2 if the
 *  user aborts the searching process
 */

ULONG ZippySearch(BPTR fp, long currpos)
	{
  BOOL	zip=FALSE;
	char  zipstr[200];
	long	foundpos=NULL,seekcounter=NULL,i=NULL;

	NC_PutString("\r[K\r[36mEnter Searchstring[33m: [m",0);
	if(NC_GetString(searchstring,68)<0) return(ZIPPY_ERROR);
	if(strlen(searchstring)<1)
		{
    NC_PutString("\r[K\r",0);
		return(ZIPPY_ABORT);
		}
  NC_PutString("\r[K\r[36mSearch from [32m([33mB[32m)[36meginning, [32m([33mT[32m)[36mop of current display or [32m([33mA[32m)[36mbort[33m: [m",0);
  if(GetCom("",0,0,0,NR_WaitChar)<0) return(FALSE);
	switch(MyFAMEDoorMsg->fdom_Data2)
		{
		case	KEY_b:
    case  KEY_B:	Seek(fp,0L,OFFSET_BEGINNING);
									break;
		case	KEY_T:
    case	KEY_t:	Seek(fp,currpos,OFFSET_BEGINNING);
									break;

    default:			return(ZIPPY_ABORT);
		}
	*mytemp1='\0';
	strcpy(mytemp1,searchstring);
	foundpos=FAMEFSearch(mytemp1,fp);
	if(foundpos==-1)
		{
		NC_PutString("\r[K\r",0);
		*mytemp1='\0';
		SPrintf(mytemp1,"Searchstring [37m%s [35mnot found ! [32m<[33mENTER[32m>[m",searchstring);
		Center(mytemp1,0,35);
    Get_Key(FALSE);
		return(ZIPPY_ABORT);
    }
	else NC_PutString("\f\r[m",1);
	Seek(fp,-1L,OFFSET_CURRENT);
	seekcounter++;
	while(zip==FALSE)
		{
    if(!IoErr())
			{
    	while(1)
				{
				i=FGetC(fp);
				if(i==10||i==13)
					{
					seekcounter--;
					zip=TRUE;
          break;
					}
				Seek(fp,-2L,OFFSET_CURRENT);
				if(IoErr())
					{
					zip=TRUE;
					break;
					}
				seekcounter++;
				}
			}
		else break;
		}
	Flush(fp);
	*zipstr='\0';
  if(seekcounter)
		{
    Read(fp,zipstr,seekcounter);
		zipstr[seekcounter]='\0';
		NC_PutString(zipstr,0);
		}
	NC_PutString("[32m",0);
	*zipstr='\0';
	Read(fp,zipstr,strlen(searchstring));
	zipstr[strlen(searchstring)]='\0';
  NC_PutStringFormat("%s[m",zipstr);
	return((Seek(fp,0L,OFFSET_CURRENT))-(strlen(searchstring)+seekcounter));
	}

/*
 *  Function to ask the user if he wish to test or view the selected file
 *  based on variables BITDEF_ALLOWARCHIVES and BITDEF_ONLY_TEST. If both
 *  vars have the right value, the user has the ability to choose what
 *  action should be performed, else the function returns immediatly FILE_TEST
 *  INPUT: char *file ->filename
 *         BOOL isarc ->TRUE for archive types, FALSE for all other filetypes
 *  Returns either QUIT, TEST or VIEW (-1,0,1)
 */

long AskForTest(char *file, BOOL isarc)
	{
 	BOOL checkview=FALSE;		// Will be only TRUE if Viewing is allowed

	if(isarc==TRUE)
		{
		if(MyPrefs->MV_Switches & BITDEF_ALLOWARCHIVES)
			{
			if(MyPrefs->MV_Switches & BITDEF_ALLOW_VIEW)
				{
        checkview=TRUE;
				}
			else return(FILE_TEST);
			}
		else return(FILE_TEST);
		}
	else
		{
		if(MyPrefs->MV_Switches & BITDEF_ALLOW_VIEW) checkview=TRUE;
		else return(FILE_TEST);
		}
 	NC_PutString("\r[K\r",0);
	NC_PutStringFormat("[36mSelect[33m: [32m([33mt[32m)[36mest or [32m([33mV[32m)[36miew file [35m%s[32m ([36m%s[32m)[33m: [32m",FilePart(file),finfo->FI_Description);
  if(GetCom("",0,0,0,NR_WaitChar)<0) wb(LOST_CARRIER);
	switch(MyFAMEDoorMsg->fdom_Data2)
		{
		case	KEY_t:
		case	KEY_T:	NC_PutString("Test",1);
									return(FILE_TEST);
		case	KEY_A:
		case	KEY_a:
		case	KEY_Q:
		case	KEY_q:	NC_PutString("[37mAbort !\n",1);
									return(FILE_QUIT);

    default:			NC_PutString("View",1);
									return(FILE_VIEW);
		}
	}

/*
 *  ShowHex() displays all not printable files in the same output Directory Opus V4.x has.
 *  In difference to the /X Version will the HexOutput made to a file and after converting,
 *  this File will be viewed with the ShowText() Function to have the same read comfort
 */

void ShowHex(char *fullname)
	{
	BPTR	fh=NULL;
	long	readed=NULL,zaehler=NULL,lv=NULL,actlines=NULL,totalsize=NULL;
	long	screenstart=NULL,prevpage=NULL;
	char	buf[256],linebuffer[256],ascbuf[18],hex[8];
  BOOL	stopit=FALSE;

	NC_PutStringFormat("%lc\n\r[m",(char) 15);
	*buf='\0';*linebuffer='\0';
	if(!(fh=Open(fullname,MODE_OLDFILE)))
		{
		PrintDosError("ShowHex()",IoErr(),TRUE,FALSE);
		return;
		}
	screenstart=prevpage=Seek(fh,0L,OFFSET_BEGINNING);
	ExamineFH(fh,myfib);
	totalsize=myfib->fib_Size;
	while((readed=Read(fh,buf,16)) && stopit==FALSE)
		{
    FAMENumToStr(zaehler,FNSF_HEX|FNSF_LEADINGZEROES,19,linebuffer);
		strcat(linebuffer,": ");
		zaehler+=readed;
		lv=NULL;
		while(lv<readed)
			{
			if((int)buf[lv]<32||(int)buf[lv]>127) ascbuf[lv]='.';
			else ascbuf[lv]=buf[lv];
			*hex='\0';
			FAMENumToStr((int)buf[lv],FNSF_BYTE|FNSF_HEX|FNSF_LEADINGZEROES,7,hex);
			strcat(linebuffer,hex);
			switch(lv)
				{
				case	3:
				case	7:
				case	11:
        case	15:	strcat(linebuffer," ");
									break;
				}
			lv++;
			};
    ascbuf[lv]='\0';
		if(readed<15) NC_PutStringFormat("\r%s\r[46C%s\n\r",linebuffer,ascbuf);
		else NC_PutStringFormat("\r%s%s\n\r",linebuffer,ascbuf);
		*linebuffer='\0';
		actlines++;
    lv=CheckForCTRL();
		if(lv==CTRL_C)
			{
			stopit=TRUE;
			SetIoErr(0);
			continue;
			}
		else if(lv==CTRL_L) actlines=0;
    switch(AskForPause(FilePart(fullname),totalsize,zaehler,actlines,fh,TRUE))
			{
			case  VIEW_NO_PROMPT:			continue;

      case	VIEW_QUIT:					stopit=TRUE;
																break;

			case  VIEW_CONT:          actlines=0;
		                            if(prevpage==screenstart) screenstart=Seek(fh,0L,OFFSET_CURRENT);
																else
																	{
																	prevpage=screenstart;
																	screenstart=Seek(fh,0L,OFFSET_CURRENT);
																	}
                                break;
			case	VIEW_HELP:					zaehler=screenstart-readed;
																if(zaehler<0) zaehler=0;
																Seek(fh,screenstart,OFFSET_BEGINNING);
																actlines=0;
                                continue;
			case	VIEW_UP:            screenstart=zaehler=prevpage;
																Seek(fh,prevpage,OFFSET_BEGINNING);
																actlines=0;
																continue;

			case	VIEW_DOWN:					actlines--;
																screenstart+=readed;
																continue;

			case	VIEW_LOST_CARRIER:	Close(fh);fh=NULL;
                        				wb(LOST_CARRIER);
			}
		};
	if(fh) Close(fh);fh=NULL;
	MV_FLAGS &= ~BITDEF_VIEWNS;
	NC_PutStringFormat("%lc\r[m",(char)15);
	}

/*
 *  IconInfo() displays all tooltypes for the given Icon...
 */

STATIC void IconInfo(char *fullpath)
	{
	struct 	Library *IconBase=NULL;
	struct  DiskObject *dobj=NULL;
	char    **tt;
	long	  counta=0L,zeilencount=0L,X,Y,ST;

	NC_PutStringFormat("\n\n\r[36mFile identified as [35m%s[36m, ",finfo->FI_Description);
	if(AskBool("view informations",FALSE)==FALSE) return;
  NC_PutStringFormat("\r[36mIcon Information for [35m%s[34m...\n\n\r[36mToolTypes[33m:\n\n\r",FilePart(fullpath));
	*mytemp1='\0';
	FAMEStrCopy(fullpath,mytemp1,255);
	mytemp1[(strlen(mytemp1)-5)]='\0';
	if(!(IconBase=OpenLibrary("icon.library",0)))
		{
		PrintDosError("icon.library",IoErr(),FALSE,FALSE);
		return;
		}
	if(!(dobj=GetDiskObject(mytemp1)))
		{
		PrintDosError("GetDiskObject()",IoErr(),FALSE,FALSE);
		CloseLibrary(IconBase);
		return;
		}
	tt=dobj->do_ToolTypes;
	if(tt)
		{
		while(*tt)
			{
      SPrintf(mytemp1,"[32m%2.2ld[33m. [m%s",counta+1,*tt);
			NC_PutString(mytemp1,1);
			*tt++;counta++;zeilencount++;
			if(zeilencount>maxlines-5)
				{
				Get_Key(TRUE);
				NC_PutString("\f",1);
				zeilencount=0L;
				};
			};
   	};
	if(!counta) Center("No ToolTypes found in this icon !",1,35);
	else NC_PutString("[m",1);
	NC_PutString("\n\r[36mOther informations[33m:\n\n\r[36mDefault-Tool[33m: [32m",0);
	if(!dobj->do_DefaultTool) NC_PutString("[37mnot available !",1);
	else NC_PutString(dobj->do_DefaultTool,1);
	if(dobj->do_CurrentX==NO_ICON_POSITION) X=-1;
	else X=dobj->do_CurrentX;
	if(dobj->do_CurrentY==NO_ICON_POSITION) Y=-1;
	else Y=dobj->do_CurrentY;
	NC_PutStringFormat("[36mX[m/[36mY-Position[33m: [32m%ld[m / [32m%ld\n\r",X,Y);
	if(dobj->do_StackSize<4096) ST=4096;
	else ST=dobj->do_StackSize;
	*mytemp1='\0';
	FAMENumToStr(ST,FNSF_GROUPING|FNSF_NUMLOCALE,MAX_BUF-1,mytemp1);
	NC_PutStringFormat("[3C[36mStacksize[33m: [32m%s [36mBytes\n\n\r",mytemp1);
	FreeDiskObject(dobj);dobj=NULL;
	CloseLibrary(IconBase);IconBase=NULL;
	}

/*
 *  Version Information - Tries to extract as much as possible Information about
 *  Standard Amiga Versionsnumbers
 */

void GetVersionInfo(long fidnum,char *name)
	{
	BPTR	myopen=NULL;
	long	pos=NULL;
	char	pathbuf[202];

	switch(fidnum)
		{
		case	FID_GIFPICTURE:
	  case	FID_MSDOSEXE:
		case	FID_JPEG:
		case	FID_ILBM:
		case	FID_8SVX:
		case	FID_UNKNOWNIFF:
		case 	FID_DISKICON:
		case  FID_DRAWERICON:
		case	FID_TRASHCANICON:
		case 	FID_DEVICEICON:
		case	FID_KICKICON:
		case	FID_APPICON:
		case	FID_UNKNOWNICON:
		case	FID_UNKNOWNXPK:
		case	FID_PCOMPRESS:
		case	FID_LHASFX:				return;
		}
	if(fidnum<=FID_XPKSHRI) return;
	if(fidnum>=290&&fidnum<=383) return;
	if(fidnum==finfo->FI_ID)
		{
		NC_PutStringFormat("\n\r[36mVersion of [35m%s[33m: [32m",FilePart(name));
		}
	else
		{
		NC_PutStringFormat("\n\r[36mVersion of [35m%s [32m[[33m%s[32m][33m: ",FilePart(name),finfo->FI_Description);
		}
	*pathbuf='\0';
	if(!(myopen=Open(name,MODE_OLDFILE)))
		{
		NC_PutString("[37mERROR - FILE NOT FOUND !!![m\n\r",1);
		return;
		};
	pos=FAMEFSearch("$VER:",myopen);
	if(pos!=-1)
		{
		Seek(myopen,pos+5,OFFSET_BEGINNING);
		FGets(myopen,pathbuf,199);
		}
	else
		{
		Seek(myopen,0L,OFFSET_BEGINNING);
		strcpy(pathbuf,FilePart(name));
		FAMEStrToUpper(pathbuf);
		pos=FAMEFSearch(pathbuf,myopen);
		if(pos==-1) strcpy(pathbuf,"[37mNot found !");
    else
			{
      Seek(myopen,strlen(pathbuf)+1,OFFSET_CURRENT);
      *mytemp1='\0';
	    FGets(myopen,mytemp1,100);
			SPrintf(pathbuf,"%s",mytemp1);
			}
		}
	Close(myopen);
	NC_PutStringFormat("%s\n\r",pathbuf);
	}

/*
 * ConvHTML() converts a HTML Page to ANSI Format by using HTMLess from TrogloByte/Darkness
 * I have written permission of TrogloByte to use and also ship his program together with
 * mEGA-vIEW V3.x
 *
 */

void ConvHTML(char *fullpath)
	{
	long	diff=NULL,stlen=NULL,length=NULL;
	char	ConvName[256];

	*ConvName='\0';
	if(CheckPacker(MyPrefs->MV_Pathes[PREFS_HTML],TRUE)==FALSE)
  	{
		if(AskBool("[36mSysOp decides to disable HTML to ANSI Converting [34m - [36mView anyway",FALSE)==TRUE) ShowText(fullpath);
		return;
		}
  strcpy(ConvName,FilePart(fullpath));
	stlen=strlen(ConvName);
	while(ConvName[stlen]!='.' && stlen>0)
		{
		stlen--;
		length++;
		}
	length--;
  *ConvName='\0';
	NC_PutStringFormat("\n\n\r[36mConverting [32m%s [36mto an ANSI-File[34m...",FilePart(fullpath));
	SPrintf(mytemp1,"%s -c76 -w -a -m -d=%s %lc%s%lc",MyPrefs->MV_Pathes[PREFS_HTML],ActTempDir,34,fullpath,34);
	if(diff=SystemTagList(mytemp1,NULL))
		{
		PrintDosError(MyPrefs->MV_Pathes[PREFS_HTML],diff,TRUE,FALSE);
		return;
		}
	strcpy(ConvName,ActTempDir);
	strcat(ConvName,FilePart(fullpath));
	ConvName[(strlen(ConvName)-length)-1]='\0';
	strcat(ConvName,".text");
  NC_PutString("[32mdone ![m",1);
	ShowText(ConvName);
  MyDeleteFile(ConvName);
  NC_PutString("[m",1);
	}

/*
 *  ReadMP3() reads all Datas from a MP3 file. (V3.4)
 */

#include "mp3.h"

void ReadMP3(char *fullpath)
	{
  BPTR	myptr=NULL;
	ULONG	mask=0UL,layertype=0UL,mpeg=0UL,mylayer=0UL,mp3size=0UL,length=NULL,mins=NULL;

	struct	MP3Header
		{
		ULONG	DATA;			// The first 30-32 Bits are the MP3 Header...
		}mp3;

	struct MP3Tag
		{
		char	tag[3],
					title[30],
          name[30],
					album[30],
					year[4],
					comment[30];
		UBYTE	type;
		}mp3tag;

	NC_PutString("\n\r[36mReading MP3 Infos, please wait[34m...",0);
	if(!(myptr=Open(fullpath,MODE_OLDFILE)))
		{
		NC_PutString("\r[K\r[Unable to open MP3 File !!!\n\r",1);
		return;
    }
	Read(myptr,&mp3,sizeof(struct MP3Header));
	ExamineFH(myptr,myfib);
	mp3size=myfib->fib_Size;
	Seek(myptr,mp3size-128,OFFSET_BEGINNING);
	FAMEMemSet(&mp3tag,'\0',sizeof(struct MP3Tag));
	Read(myptr,&mp3tag,sizeof(struct MP3Tag));
	Close(myptr);myptr=NULL;
 	NC_PutStringFormat("\r[K\r[32mDisplay MPEG Infos for file [35m%s[33m:\n\n\r",FilePart(fullpath));
	FAMENumToStr(mp3size,FNSF_GROUPING|FNSF_NUMLOCALE,MAX_BUF,mytemp1);
 	NC_PutStringFormat(" [36mFilesize[33m: [32m%s [36mBytes\n\r",mytemp1);
	if(mp3.DATA & MP3_STANDARD) mpeg=0;
	else mpeg=1;
	layertype=mp3.DATA & (MP3_LAYER_B13|MP3_LAYER_B14);
	layertype=(layertype >> 17);					// Get decimal value of Layer
	switch(layertype)
		{
		case	1:	mylayer=MP3_LAYER_3;
              break;
		case	2:	mylayer=MP3_LAYER_2;
							break;
		case  3:	mylayer=MP3_LAYER_1;
              break;
		}
	mask=mp3.DATA & (MP3_BITRATEB16|MP3_BITRATEB17|MP3_BITRATEB18|MP3_BITRATEB19);
  mask = (mask >> 12);				// Decimalvalue of Bitrate
	mp3size=mp3size*8;
	length=mp3size/(1000*bitrates[mpeg][mask][mylayer]);
	if(length>60)
		{
		do
      {
			length=length-60;
			mins++;
			}while(length>=60);
		}
	NC_PutStringFormat("   [36mLength[33m: [32m%ld[m:[32m%2.2ld [36mmins\n\r",mins,length);
	NC_PutStringFormat(" [36mStandard[33m: [32m%s%s\n\r",std[mpeg],layers[layertype]);
  NC_PutStringFormat("  [36mBitrate[33m: [32m%ld [36mKBit[m/[36ms\n\r",bitrates[mpeg][mask][mylayer]);
	mask=mylayer=0UL;
	mask=mp3.DATA & (MP3_FREQB20|MP3_FREQB21);
	mask=(mask >> 10);
	mylayer=mp3.DATA & (MP3_MODEB24|MP3_MODEB25);
	mylayer=(mylayer >>6);
  NC_PutStringFormat("   [36mFormat[33m: [32m%ld [36mHz [m/ [32m%s\n",frequency[mpeg][mask],modes[mylayer]);
	NC_PutStringFormat(" [36mUse CRCs[33m: [32m%s\n\r", (!mp3.DATA & MP3_PROTECTION) ? "Yes" : "No");
	NC_PutStringFormat("  [36mPrivate[33m: [32m%s\n\r", (mp3.DATA & MP3_PRIVATE) ? "Yes" : "No");
	NC_PutStringFormat("[36mCopyright[33m: [32m%s\n\r", (mp3.DATA & MP3_COPYRIGHT) ? "Yes" : "No");
	NC_PutStringFormat(" [36mOriginal[33m: [32m%s\n\n\r", (mp3.DATA & MP3_ORIGINAL) ? "Yes" : "No");
	if(Strnicmp(mp3tag.tag,"TAG",3))
		{
    NC_PutString("[37mNo MP3Tag informations found.[m\n",1);
		return;
		}
	NC_PutStringFormat("   [36mArtist[33m: [32m%30.30s\n\r",mp3tag.name);
	NC_PutStringFormat("    [36mTitle[33m: [32m%30.30s\n\r",mp3tag.title);
	NC_PutStringFormat("    [36mAlbum[33m: [32m%30.30s\n\r",mp3tag.album);
	NC_PutStringFormat("     [36mYear[33m: [32m%4.4s\n\r",mp3tag.year);
	NC_PutStringFormat("  [36mComment[33m: [32m%28.28s\n\n\r",mp3tag.comment);
	}
