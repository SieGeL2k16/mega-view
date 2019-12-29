/*
 *  Support.c - Contains checks and other required routines for mega-view
 *
 */

#include <Fame/fameDoorProto.h>
#include <proto/utility.h>
#include <proto/fileid.h>
#include <libraries/fileid.h>								// Required defs for FileID.library
#include <libraries/fileid_IDDefs.h>
#include <utility/utility.h>
#include "global_defines.h"
#include "struct_ex.h"
#include "proto.h"

long 		MyDeleteFile(char *was);              		// Sets the deleted flag and removes the file
void 		ConvertStrings(UBYTE*,UBYTE*,BOOL);				// Convert dates via locale
void 		CheckEnglishDate(char*);									// Convert dateformat
void		CutSpaces(char *);
BOOL 		CheckSpecialChars(char *);								// Test for special characters
BOOL 		AddDlPath(char *);                 				// Adds another DLPath to List
void 		FreeDlPathes(void);												// Frees DlPathes Lists
BOOL 		CheckForPattern(char *, char *);					// Test if Filename contains pattern
LONG 		TestTheFile(char *);											// Checks file for restricted etc.
BOOL 		Get_Key(BOOL);														// Wait for User-Input
void 		PrintDosError(char *,long,BOOL,BOOL); 		// General DosError exit function
void 		CreateTempDir(void);											// Creates global Tempdir
void 		RemoveTempDir(void);											// Removes global tempdir
BOOL 		CheckPacker(char *,BOOL);									// Tests if specified prg exists
long 		MyExecute(char *cmd);											// Executes the cmd string via PIPE:
void 		MyCutANSI(char *e);												// Enhanced CutAnsi() routine
BOOL 		AskBool(char *, BOOL);										// General Yes/No ask function
long 		CheckForCTRL(void);												// CTRL-C / CTRL-L Check
void 		GetFileID(char *fullpath,long sel);				// Extracts FileID.diz from DMS/Exe

BOOL 		AddFileType(char *extension, char *cmd); 	// Adds External Filetype to List (V3.3)
void 		FreeFileTypes(void);											// Frees list of external filetypes (V3.3)

/* FAME Supporter Functions without AutoChecking */

long NC_GetString( char*, long );
long PutCom( char*, long, long, ULONG, long );
long GetCom( char*, long, long, ULONG, long );

/*
 *  Creates a DateTime struct out of *date & *time setting. If flag is TRUE,
 *  the date string will be adapted to fit the FORMAT_USA template.
 */

void ConvertStrings(UBYTE *date,UBYTE *time,BOOL flag)
	{
	if(flag==TRUE)
		{
		CheckEnglishDate(date);
		}
	dt->dat_Stamp=*ds;
  dt->dat_Format=FORMAT_USA;
	dt->dat_StrDate=date;
	dt->dat_StrTime=time;
	StrToDate(dt);
	}

/*
 *  Rebuilts the date string from the original 'DD-MMM-YY' format into the
 *  regular USA Format, which is 'MM-DD-YY' (LHA / LZX)
 */

void CheckEnglishDate(char *chek)
	{
	char			buf[4],newbuf[LEN_DATSTRING];
	register 	long	i,t;
	STATIC		char 	*mons[14]={"","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

	*newbuf='\0';
	strmid(chek,buf,4,3);
	for(i=1;i<13;i++)
		{
		if(!(Stricmp(buf,mons[i])))
			{
			FAMENumToStr(i,FNSF_LEADINGZEROES,3,newbuf);
			newbuf[2]='-';
			for(t=0;t<3;t++) newbuf[t+3]=chek[t];
			for(t=0;t<2;t++) newbuf[6+t]=chek[7+t];
			newbuf[8]='\0';
    	strcpy(chek,newbuf);
			};
		};
	}

/*
 *  MyDeleteFile(char *) Deletes a file, also if the file has the D-elete Bit not set!
 */

long MyDeleteFile(char *was)
	{
	SetProtection(was,FIBB_DELETE);
	return(DeleteFile(was));
	}

/*
 *  -- Checks for path strings inside filename and returns TRUE if ':' or '/' was found
 */

BOOL CheckSpecialChars(char *testline)
	{
	char	*t;

	t=testline;
	while(*t)
		{
		if(*t==':'||*t=='/') return(TRUE);
		else *t++;
		}
  return(FALSE);
	}

/*
 *  Checks if file has RESTRICTED comment or is known as one of the private
 *  files of FAME (i.e. name of user.data etc.) Returns 0 if *file is
 *  valid to process, >0 if file is forbidden and <0 if any error occured
 */


long TestTheFile(char *file)
	{
  BPTR	myptr=NULL;
	char	localbuf[256];

	if(!(myptr=Open(file,MODE_OLDFILE))) return(-1);		// Unable to open file!
	ExamineFH(myptr,myfib);
	*localbuf='\0';
	FAMEStrCopy(myfib->fib_Comment,localbuf,255);
	Close(myptr);myptr=NULL;
  if(FAMEStrStr(localbuf,"RESTRICT")) return(1);			// File is Restricted!
	GetCom(localbuf,0,0,0,RD_GetUserDataLoc);
	if(!(Stricmp(FilePart(file),localbuf))) return(1);  // File has same name like FAME User.data
	else return(0);
	}

/*
 *  --- ListFunctions for Download-Pathes gained from FAME ---
 */

BOOL AddDlPath(char *path)
	{
	if(!fp1)
		{
		if(!(fp1=(struct FAMEPathes *)AllocPooled(mv_pool,sizeof(struct FAMEPathes)))) return(FALSE);
		fp2=fp1;
		}
	else
		{
		if(!(fp2->next=(struct FAMEPathes *)AllocPooled(mv_pool,sizeof(struct FAMEPathes)))) return(FALSE);
		fp2=fp2->next;
		}
	fp2->next=NULL;
	strcpy(fp2->DL_Path,path);
	return(TRUE);
	}

void FreeDlPathes(void)
	{
	struct FAMEPathes *h=fp1;

	while(fp1)
		{
		h=fp1;
		fp1=fp1->next;
		FreePooled(mv_pool,h,sizeof(struct FAMEPathes));
		}
	fp1=NULL;fp2=NULL;
	}

/*
 *  --- ListFunctions for External FileTypes found under FAME:ExternEnv/Doors/mv_extern.cfg ---
 */

BOOL AddFileType(char *extension, char *path)
	{
	if(!et1)
		{
		if(!(et1=(struct ExtTypes *)AllocPooled(mv_pool,sizeof(struct ExtTypes)))) return(FALSE);
		et2=et1;
		}
	else
		{
		if(!(et2->next=(struct ExtTypes *)AllocPooled(mv_pool,sizeof(struct ExtTypes)))) return(FALSE);
		et2=et2->next;
		}
	et2->next=NULL;
	strcpy(et2->Extension,extension);
	strcpy(et2->Command,path);
	return(TRUE);
	}

void FreeFileTypes(void)
	{
	struct ExtTypes *h=et1;

	while(et1)
		{
		h=et1;
		et1=et1->next;
		FreePooled(mv_pool,h,sizeof(struct ExtTypes));
		}
	et1=NULL;et2=NULL;
	}

/*
 *  --- Checks *testline for valid ADOS-Patterns and returns TRUE if any ---
 */

BOOL CheckForPattern(char *testline, char *buffer)
	{
	char localbuf[256];

	*localbuf='\0';
	strcpy(localbuf,testline);
	FAMEStrToUpper(localbuf);
	switch(ParsePatternNoCase(localbuf,buffer, MAX_BUF-1))
		{
		case	1:	break;
		case	0:  return(FALSE);
		case	-1:	wb("\n\r[37mParsePatternNoCase(): Out of Memory!\n\r");
		}
	return(TRUE);
  }

/*
 *  Simple Function to wait for User keypress, returns FALSE if an error
 *  occures during the communication with FAME. If display == FALSE no
 *  output will be done, just waiting for a keypress...
 */

BOOL Get_Key(BOOL display)
	{
  if(display==TRUE) Center("Press any key to continue !",0,32);
	if(GetCom("",0,0,0,NR_WaitChar)<0) return(FALSE);
  NC_PutString("\r[K\r",0);
	return(TRUE);
	}

/*
 *  Prints Dos Error 'error' with optional header 'myheader' via Fault().
 *  If showfail=TRUE a 'FAIL !' String will be printed.
 *  if stopit = TRUE PrintDosError() exits after displaying the string.
 */

void PrintDosError(char *myheader,long error,BOOL showfail,BOOL stopit)
	{
	char localbuf[256];

  if(showfail==TRUE) NC_PutString("[37mFAIL !\n\r",1);
	else NC_PutString("\n\r[37m",1);
	Fault(error,myheader,localbuf,255);
	NC_PutString(localbuf,1);
	if(stopit==TRUE) wb("[m\n\r");
	}

/*
 *  Creates a temporary directory which will be used while running on the
 *  according node. After exiting mEGA-vIEW, the whole directory including
 *  *ALL* contents will be deleted with RemoveTempDir() !!! This is the
 *  successor of mEGA-vIEW V2.15, which forgots sometimes some files inside
 *  the Temp dir!
 */

void CreateTempDir(void)
	{
	BPTR	tmp_ptr=NULL;

	if(*ActTempDir)
		{
		if(!(tmp_ptr=CreateDir(ActTempDir))) PrintDosError("CreateTempDir()",IoErr(),FALSE,TRUE);
		UnLock(tmp_ptr);tmp_ptr=NULL;
	  strcat(ActTempDir,"/");										// For AddPart()...
		}
	}

/*
 *  Removes the temp dir created with CreateTempDir(). If not found, returns
 *  silently, as then something is going wrong, but no need to inform the
 *  user about this!
 */

void RemoveTempDir(void)
	{
	BPTR	tmp_ptr=NULL;

	if(!*ActTempDir) 
		{
		return;										// No more empty dirs ! ;)
		}
  if(tmp_ptr=Lock(ActTempDir,ACCESS_READ))
		{
		if(Examine(tmp_ptr,myfib))
			{
			while(ExNext(tmp_ptr,myfib))
				{
        if(myfib->fib_DirEntryType>0) continue;
        *mytemp1='\0';
				strcpy(mytemp1,ActTempDir);
				AddPart(mytemp1,myfib->fib_FileName,MAX_BUF-1);
				DeleteFile(mytemp1);
				}
			}
		UnLock(tmp_ptr);tmp_ptr=NULL;
		}
	DeleteFile(ActTempDir);						// Finally remove the whole dir
	}

/*
 *  This function checks if the specified program exists. It returns FALSE
 *  if specified program doesn't exists, else TRUE.
 *  INPUT: char *checkpath -> Complete path/name to check for
 *         BOOL showprob   -> TRUE = Error will be reported to the user
 */

BOOL CheckPacker(char *checkpath,BOOL showprob)
	{
	BPTR		testlock=NULL;
	STATIC	char *ENTERKEY="[32m<[33mENTER[32m>[m";

	if(!*checkpath)
		{
		if(showprob==FALSE) return(FALSE);
    NC_PutStringFormat("\r[K\r[37mNo Support for [32m%s [37mgiven ! %s",finfo->FI_Description,ENTERKEY);
		Get_Key(FALSE);
		return(FALSE);
		}
	if(!(testlock=Lock(checkpath,ACCESS_READ)))
		{
		if(showprob==FALSE) return(FALSE);
    NC_PutStringFormat("\r[K\r[37mInvalid Path detected ([32m%s[37m), inform SysOp ! %s",checkpath,ENTERKEY);
		Get_Key(FALSE);
		return(FALSE);
		}
  else UnLock(testlock);
	return(TRUE);
	}

/*
 *  MyExecute() starts the 'cmd' line and immediatly prints the output of
 *  the packers/checkers via PIPE: device to the user (real-time checking!)
 *  Supports also CTRL-L & CTRL-C.
 */

long	MyExecute(char *cmd)
	{
	BPTR	myopen=NULL;
	long	retcode,pipesize;
  char	pipename[256];

	NC_PutString("[m\n",1);
	if(retcode=SystemTagList(cmd,NULL)) return(retcode);
	*pipename='\0';
	SPrintf(pipename,"PIPE:%s",tempfile);
	if(!(myopen=Open(pipename,MODE_OLDFILE))) return((IoErr()));
	do
		{
		pipesize=Read(myopen,pipename,200);
		if(pipesize>0)
			{
			pipename[pipesize]='\0';
			if(pipename[pipesize]=='\n') NC_PutString(pipename,1);
			else NC_PutString(pipename,0);
    	if(CheckForCTRL()==CTRL_C)
				{
				NC_PutString("\n[35mPlease wait until background process is finished[34m...",0);
				pipesize=retcode=-100;
        break;
				}
			}
  	}while(pipesize>0);
	*mytemp1='\0';
	while(pipesize) pipesize=Read(myopen,mytemp1,MAX_BUF-1);
	Close(myopen);
	if(retcode==-100) NC_PutString("[32mdone !",1);
	return(0);
	}

/*
 *  Enhanced CutAnsi() function, which now also strips ANSI Position Codes
 *  and other codes, so the real Chars will be displayed. TAB Codes will be
 *  changed to Spaces to make the output a bit easier.
 */

void MyCutANSI(char *e)
	{
	char 	*d,*s,*b;
	int 	i;

	d=s=e;
	while(*d)
		{
		if(*d=='	')						// Check for TAB
			{
			*s++=' ';
			*d++;
			continue;
			}
		if(*d=='' || *d=='›')
			{
			i=1;
			b=d;
			while(i<=5 && i>0)
				{
				switch(*b)
					{
					case	';'	:	i=1;
											*b++;
											break;
          case  'A' :
          case  'B' :
					case	'H' :
					case  'S' :
					case	'T'	:
					case	'm'	:	i=NULL;
											*b++;
											break;

					case	'\0':	i=6;
											break;
					default		:	*b++; i++;
					}
				}
			if(!i)	d=b;
			else if(*d) *s++=*d++;
			}
		else *s++=*d++;
		}
	*s='\0';
	}

/*
 *  AskBool() - Gets user response for (y/n) questions ...
 */

BOOL AskBool(char *question, BOOL nofirst)
	{
  char	localbuf[202];
	STATIC	char *yes="[32mYes ![m\n",*no="[37mNo ![m\n";

  *localbuf='\0';
  strcpy(localbuf,question);
  if(nofirst==TRUE) strcat(localbuf," [32m([33my[32m/[33mN[32m) [36m? ");
	else strcat(localbuf," [32m([33mY[32m/[33mn[32m) [36m? ");
	NC_PutString(localbuf,0);
	while(1)
		{
		if(GetCom("",0,0,0,NR_WaitChar)<0) wb(LOST_CARRIER);
		switch(MyFAMEDoorMsg->fdom_Data2)
			{
			case	ENTER:	if(nofirst==TRUE)
											{
											NC_PutString(no,1);
											return(FALSE);
											}
	                  else
											{
											NC_PutString(yes,1);
											return(TRUE);
											}
			case	KEY_Y:
			case	KEY_y:	NC_PutString(yes,1);
										return(TRUE);

			case	KEY_N:
      case	KEY_n:  NC_PutString(no,1);
										return(FALSE);
			}
		}
	}

/*
 *  Checks for CTRL-C abort request, returns TRUE if found, else FALSE
 */

long CheckForCTRL(void)
	{
  if(GetCom("",160,0,0,AR_HotKey)<0) return(CTRL_C);
	if(MyFAMEDoorMsg->fdom_Data2==12)
		{
		NC_PutString("\f\r",0);
    return(CTRL_L);
		}
  if(MyFAMEDoorMsg->fdom_Data2==3)
		{
		NC_PutStringFormat("[m[1 p%lc[37m***Break[K[m\n\r",(char)15);
		return(CTRL_C);
		}
	else return(CTRL_NONE);
	}

/*
 *  Function to cut out all found spaces
 */

void CutSpaces(char *s)
	{
	char *d=s;

	while(*d==' ') *d++;
  while(*d) *s++=*d++;
	*s='\0';
	}

/*
 *	Function to read the File_ID.diz file from Executables and DMS-Archives.
 *  Either EXEDescript or DMSDescript will be called, and if the according
 *  Descripter is not available, a message to the user will be displayed.
 *
 */

void GetFileID(char *fullpath,long packersel)
	{
	long	error=NULL,descr=NULL;
	char	localbuf[256];

	switch(packersel)
		{
    case	PREFS_DMS:  descr=PREFS_DMSID;
											break;
		case	PREFS_EXE:	descr=PREFS_EXEID;
											break;
		}
  if(!*MyPrefs->MV_Pathes[descr]) return;
	if(AskBool("[36mShould I check for [35mFILE_ID.DIZ",FALSE)==FALSE) return;
	NC_PutString("[1A\r",0);
  FAMEStrCopy(ActTempDir,localbuf,255);
  AddPart(localbuf,"FILE_ID.DIZ",255);
  SPrintf(mytemp2,"%s >NIL: x %s %lc%s%lc",MyPrefs->MV_Pathes[descr],localbuf,34,fullpath,34);
	if(error=SystemTagList(mytemp2,NULL)) PrintDosError(FilePart(MyPrefs->MV_Pathes[descr]),error,FALSE,FALSE);
	if(CheckPacker(localbuf,FALSE)==FALSE)
		{
		NC_PutStringFormat("\n\r[37mNo [35mFILE_ID.DIZ [37mfound in file [32m%s [37m!\n\n\r",FilePart(fullpath));
		return;
		}
	else ShowText(localbuf);
	MyDeleteFile(localbuf);
	}

/*
 *  FAME Supporter Functions with Autocheck disabled, required for mEGA-vIEW
 *  as many datafiles are kept open and it's not acceptable that FAME is
 *  able to exit mEGA-vIEW without freeing all resources first. Therefor
 *  these functions disable the autocheck, call the original functions and
 *  restore the autocheck flag after calling the function.
 */

long NC_GetString( char *str, long length)
	{
	long	myretcode;

	GlobFIMFlags |=  FIMF_NOCHECK;			// Disable FAME Autocheck
	myretcode=GetString(str,length);
 	GlobFIMFlags &= ~FIMF_NOCHECK;			// Enable it again
	return(myretcode);
	}

long PutCom(char *text, long dat1, long dat2, ULONG dat3, long cmd)
	{
	long	myretcode;

	GlobFIMFlags |=  FIMF_NOCHECK;			// Disable FAME Autocheck
	myretcode=PutCommand(text,dat1,dat2,dat3,cmd);
 	GlobFIMFlags &= ~FIMF_NOCHECK;			// Enable it again
	return(myretcode);
	}

long GetCom(char *text, long dat1, long dat2, ULONG dat3, long cmd)
	{
	long	myretcode;

	GlobFIMFlags |=  FIMF_NOCHECK;			// Disable FAME Autocheck
	myretcode=GetCommand(text,dat1,dat2,dat3,cmd);
 	GlobFIMFlags &= ~FIMF_NOCHECK;			// Enable it again
	return(myretcode);
	}
