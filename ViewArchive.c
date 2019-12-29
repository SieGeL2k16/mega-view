/*
 *  ViewArchive.c - Contains the functions to parse and view the archive contents, either
 *                  LHA, LZX or ZIP. Other Packers couldn't be used currently, as they do
 *                  not support extracting of single files out of their archives.
 */

#include <clib/alib_protos.h>					// Protos for amiga.lib
#include <fame/famedoorcommands.h>
#include <Fame/fameDoorProto.h>
#include <proto/fileid.h>
#include <proto/utility.h>
#include <proto/fame.h>
#include <utility/utility.h>
#include <libraries/fileid.h>
#include <libraries/fileid_IDDefs.h>
#include "global_defines.h"
#include "struct_ex.h"
#include "proto.h"

/*
 *  ------ Prototype-Definitions -------
 */

void ViewArchive(char *fullpath, long archiver);						// Will be called from IdentifyFile()
void CountIt(void);																					// The Busy Counter

STATIC BOOL Insert_Files(void);															// Reads LHA/LZX Archives
STATIC BOOL Insert_Zip(void);                               // Reads ZIP Archives
STATIC void free_elements (struct Liste *);                 // Removes List of Files from mem
STATIC BOOL AddToArchiveList(struct PackerList *);					// Adds an entry to list of files
STATIC void MakePrevPtr(void);															// Reorganize the prev ptrs
STATIC void DisplayThem(char *);														// Main Display function
STATIC int archivepause(struct Liste *,struct Liste *,int);	// Pauseprompt in archive mode
STATIC void DisplayOnlineHelp(BOOL v,BOOL b);								// Shows online help in archive mode
STATIC void DisplaySingleLine(struct Liste *,int,BOOL);			// Shows the current active line
STATIC void ActOnFileList(char *,long);											// Processes all flagged files

struct Liste 	*quicksort (struct Liste *l);                 // Quicksorts the list of files
struct Liste 	*merge (struct Liste *l1,struct Liste *l2);		// Merges both list parts together
int 					sorting (struct Liste *l1,struct Liste *l2);	// Sorts the list parts

/*
 *  ------ Global Variables used in ViewArchive.c --------
 */

STATIC char *VORTEXT=		"[36mLeft[32m([33m4[32m)[34m, ",
						*WEITERTEXT="[36mRight[32m([33m6[32m)[34m, ",
						*TEXT_1=    "\n\r[35mChoose[33m: [36mUp[32m([33m8[32m) [34m/ [36mDown[32m([33m2[32m)[34m, ",
						*TEXT_2=    "[32m([33mSpace[32m)[34m, [32m([33m?[32m)[34m, [32m([33mQ[32m/[33mCR[32m)",
						*LIST_FLAGS="\f\rList of flagged Files\n\n\r";

struct	PackerList
	{
	char  FullName[200],
				FileName[108],
				Datum[12],
				Uhrzeit[10],
				Bytes[14];
	BOOL  Selected;
	};

struct Liste
	{
	struct 	PackerList fib;																			// Pointer to my list
	struct 	Liste *prev;																				// Ptr to prev. entry
	struct 	Liste *next;		       															// Ptr to next entry
	}*Fib,*Fib2;

struct 	Liste *merge_start,*merge_end;
long		pagecount=NULL,choosen=NULL,pages,actpage;

STATIC 	USHORT Busy_Count;																		// Required for CountIt()

/*
 *  Main Function, here the archive will be loaded into memory and displayed by the user
 */

void ViewArchive(char *fullpath, long archiver)
	{
	long	retcode=NULL;
	char	buf[256];

	Busy_Count=0;
	if(CheckPacker(MyPrefs->MV_Pathes[archiver],FALSE)==FALSE) return;
	*mytemp2='\0';*buf='\0';
	PutStringFormat("\r\n[36mOne moment, reading contents from [35m%s[34m...",FilePart(fullpath));
	CountIt();
	SPrintf(buf,"T:%s",tempfile);
	switch(archiver)
		{
		case PREFS_LHA:		SPrintf(mytemp2,"%s >%s -m -N -r -Qw v %lc%s%lc",MyPrefs->MV_Pathes[PREFS_LHA],buf,34,fullpath,34);
											break;
		case PREFS_ZIP:  	SPrintf(mytemp2,"%s >%s -q -v %lc%s%lc",MyPrefs->MV_Pathes[PREFS_ZIP],buf,34,fullpath,34);
											break;
		case PREFS_LZX:		SPrintf(mytemp2,"%s >%s -m l %lc%s%lc",MyPrefs->MV_Pathes[PREFS_LZX],buf,34,fullpath,34);
		}
	if((retcode=SystemTagList(mytemp2,NULL)))
		{
		retcode=IoErr();
    MyDeleteFile(buf);
		PrintDosError("CreateArcTempFile",retcode,TRUE,FALSE);
		return;
		}
	switch(archiver)
		{
		case	PREFS_LHA:
		case	PREFS_LZX:	if(Insert_Files()==FALSE) return;
                      break;
		case	PREFS_ZIP:	if(Insert_Zip()==FALSE) return;
											break;
		}
	NC_PutString("[32mdone !",1);
	MyDeleteFile(buf);
	DisplayThem(fullpath);
  if(choosen) ActOnFileList(fullpath,archiver);
  else NC_PutString("\f\n\r[35mNo files selected[34m...[m\n\r",1);
	free_elements(Fib);
	}

/*
 *  This function walks through the list and calls for every File IdentifyFile() to allow
 *  identification/downloading/viewing the selected file.
 */

STATIC void ActOnFileList(char *fullpath,long archiver)
  {
	struct Liste *myptr;
	char	pathbuf[256];
	long	error,processed=NULL;

	myptr=Fib;
	while(myptr)
		{
    if(myptr->fib.Selected==TRUE)
			{
      PutStringFormat("\f\n\r[36mNow processing file[33m: [35m%s[34m...",myptr->fib.FileName);
			*pathbuf='\0';
			switch(archiver)
				{
				case PREFS_LHA:	SPrintf(pathbuf,"%s -x2 -N -Qw -Qo e %lc%s%lc %lc%s%lc %s",MyPrefs->MV_Pathes[PREFS_LHA],34,fullpath,34,34,myptr->fib.FullName,34,ActTempDir);
												break;
				case PREFS_LZX:	SPrintf(pathbuf,"%s e %lc%s%lc %lc%s%lc %s",MyPrefs->MV_Pathes[PREFS_LZX],34,fullpath,34,34,myptr->fib.FullName,34,ActTempDir);
												break;
				case PREFS_ZIP:	SPrintf(pathbuf,"%s -j -o %lc%s%lc %lc%s%lc -d %s",MyPrefs->MV_Pathes[PREFS_ZIP],34,fullpath,34,34,myptr->fib.FullName,34,ActTempDir);
												break;

				}
			if(error=SystemTagList(pathbuf,NULL))
       	{
				PrintDosError("Archiver failed with ReturnCode",error,TRUE,FALSE);
				Get_Key(FALSE);
				myptr=myptr->next;
				continue;
				};
      *pathbuf='\0';
			strcpy(pathbuf,ActTempDir);
			AddPart(pathbuf,myptr->fib.FileName,255);
      NC_PutString("[32mdone !\n",1);
			if(MyPrefs->MV_Switches & BITDEF_ALLOW_VIEW) NC_PutString("[35mSelect[33m: [36mView Non[32m([33mS[32m)[36mtop[34m, [32m([33mA[32m)[36mbort[34m, [32m([33mD[32m)[36mownload[34m, [32m([33mEnter[32m)[34m=[36mView normal[33m: ",0);
			else NC_PutString("[35mSelect[33m: [36mView Non[32m([33mS[32m)[36mtop[34m, [32m([33mA[32m)[36mbort[34m, [32m([33mEnter[32m)[34m=[36mView normal[33m: ",0);
			if(GetCom("",0,0,0,NR_WaitChar)<0) return;
			switch(MyFAMEDoorMsg->fdom_Data2)
				{
				case	KEY_S:
				case	KEY_s:	MV_FLAGS |=BITDEF_VIEWNS;
                      NC_PutString("[32mView NonStop ![m\n",1);
											IdentifyFile(pathbuf);
											break;
        case	KEY_q:
				case	KEY_Q:
				case	KEY_A:
				case	KEY_a:	NC_PutString("[37mAborting this file !\n[m",1);
											processed++;
                      myptr=myptr->next;
											continue;
				case	KEY_D:
				case	KEY_d:	NC_PutString("[32mDownloading[34m...[m\n",1);
											if(GetCom(pathbuf,DOORUD_COUNTBYTES,0,0,CF_ZModemSend)<0)
												{
                        PutStringFormat("\n\r[37mError Nr. [32m%ld [37mrecieved during ZModemSend !\n\n\r",MyFAMEDoorMsg->fdom_Data2);
                      	}
											else NC_PutString("\n\r[32mDownload successful !\n\r",1);
											break;
        default:  		NC_PutString("[32mView normal !\n[m",1);
											IdentifyFile(pathbuf);
											break;
				}
			MyDeleteFile(pathbuf);
			processed++;
			if(processed<choosen) Get_Key(TRUE);
			}
		myptr=myptr->next;
		}
	}

/*
 *  Function to insert all files out of LHA/LZX Archives into my private list to display
 *  it in a CRSR-controlled display.
 */

STATIC BOOL Insert_Files(void)
	{
	BPTR		mygod=NULL;
	long		lv=NULL;
  struct 	PackerList *mypack=NULL;
	char		**parms,**lv_parms,tname[110];

	if(!(mypack=AllocPooled(mv_pool,sizeof(struct PackerList))))
		{
		MyDeleteFile(tname);
		PrintDosError("InsertLHA->AllocPooled()",ERROR_NO_FREE_STORE,TRUE,TRUE);
		return(FALSE);
		}
  SPrintf(tname,"T:%s",tempfile);
	if(!(mygod=Open(tname,MODE_OLDFILE)))
		{
		FreePooled(mv_pool,mypack,sizeof(struct PackerList));
		return(FALSE);
		}
	*mytemp2='\0';
	while(FGets(mygod,mytemp2,199))
		{
    CountIt();
 		if(*mytemp2!='-') continue;
		else break;
		};
  while(FGets(mygod,mytemp2,199))
		{
    CountIt();
		if(*mytemp2=='-') break;
		CutSpaces(mytemp2);
		if(mytemp2[0]==':') continue;
		mytemp2[strlen(mytemp2)-1]='\0';
		parms=NULL;lv_parms=NULL;
		parms=MakeArray(mytemp2);
		lv_parms=parms;
		if(*lv_parms)
			{
			StrToLong(*lv_parms,&lv);
			if(!lv)
				{
				FreeArray(parms);
				continue;
				};
			FAMENumToStr(lv,FNSF_GROUPING|FNSF_NUMLOCALE,13,mypack->Bytes);
			lv_parms++;
			};
		if(*lv_parms) lv_parms++;
		if(*lv_parms) lv_parms++;
		if(*lv_parms)
			{
			if(*lv_parms[0]=='%') lv_parms++;
			if(!lv_parms) { FreeArray(parms);continue;};
			if(strlen(*lv_parms)<9) { FreeArray(parms);continue;}
			strcpy(mypack->Datum,*lv_parms);
			lv_parms++;
			};
		if(*lv_parms) { strcpy(mypack->Uhrzeit,*lv_parms);lv_parms++;};
		if(*lv_parms)
			{
			strcpy(mypack->FullName,*lv_parms);lv_parms++;
			while(*lv_parms)
				{
				strcat(mypack->FullName," ");
				strcat(mypack->FullName,*lv_parms);
				lv_parms++;
				};
			}
		else { FreeArray(parms);continue;};
		if(FAMEStrStr(mypack->FullName,"#")||FAMEStrStr(mypack->FullName,"?")||FAMEStrStr(mypack->FullName,"*")||FAMEStrStr(mypack->FullName,"~")||FAMEStrStr(mypack->FullName,"+")) {FreeArray(parms);continue;};
		strcpy(mypack->FileName,FilePart(mypack->FullName));
		ConvertStrings(mypack->Datum,mypack->Uhrzeit,TRUE);
		if(AddToArchiveList(mypack)==FALSE)
			{
			Close(mygod);mygod=NULL;
			FreePooled(mv_pool,mypack,sizeof(struct PackerList));
			MyDeleteFile(tname);
			FreeArray(parms);
			PrintDosError("InsertLha->AddToArchiveList()",ERROR_NO_FREE_STORE,TRUE,TRUE);
			}
		strcpy(mypack->Bytes,"");
    strcpy(mypack->Datum,"");
		strcpy(mypack->Uhrzeit,"");
		strcpy(mypack->FileName,"");
		strcpy(mypack->FullName,"");
		FreeArray(parms);
    };
	Close(mygod);mygod=NULL;
	FreePooled(mv_pool,mypack,sizeof(struct PackerList));
	MyDeleteFile(tname);
	return(TRUE);
	}

/*
 *  Scans Zip Listfile and builts the list to display
 */

STATIC BOOL Insert_Zip(void)
	{
	BPTR		mygod=NULL;
	struct 	PackerList *mypack=NULL;
	char		**parms,**lv_parms,tname[110];
	STATIC char *baseline="------  ------   ----  -----   ----    ----   ------     ----";
	long		lv=NULL;

	if(!(mypack=AllocPooled(mv_pool,sizeof(struct PackerList))))
		{
		MyDeleteFile(tname);
		PrintDosError("InsertZip->AllocPooled()",ERROR_NO_FREE_STORE,TRUE,TRUE);
		return(FALSE);
		}
	*mytemp2='\0';
  SPrintf(tname,"T:%s",tempfile);
	if(!(mygod=Open(tname,MODE_OLDFILE)))
		{
		FreePooled(mv_pool,mypack,sizeof(struct PackerList));
		return(FALSE);
		}
	*mytemp2='\0';
	while(FGets(mygod,mytemp2,MAX_BUF-1))	
		{	
    CountIt();
		if(FAMEStrStr(mytemp2,baseline)) break;
		};
  while(FGets(mygod,mytemp2,MAX_BUF-1))
		{
    CountIt();
		if(FAMEStrStr(mytemp2,baseline)) break;
		CutSpaces(mytemp2);
		mytemp2[strlen(mytemp2)-1]='\0';
		parms=NULL;lv_parms=NULL;
		if(!(parms=MakeArray(mytemp2)))
			{
			Close(mygod);
			FreePooled(mv_pool,mypack,sizeof(struct PackerList));
			return(FALSE);
			}
		lv_parms=parms;
		if(*lv_parms)
			{
			StrToLong(*lv_parms,&lv);
			if(!lv)
				{
				FreeArray(parms);
				continue;
				};
			FAMENumToStr(lv,FNSF_GROUPING|FNSF_NUMLOCALE,13,mypack->Bytes);
			lv_parms++;
			};
		if(*lv_parms) lv_parms++;
		if(*lv_parms) lv_parms++;
		if(*lv_parms) lv_parms++;
		if(*lv_parms)
			{
			strcpy(mypack->Datum,*lv_parms);
			lv_parms++;
			}
		else { FreeArray(parms); continue;};
		if(*lv_parms) { strcpy(mypack->Uhrzeit,*lv_parms);lv_parms++;};
		if(*lv_parms) lv_parms++;
		if(*lv_parms)
			{
 			strcpy(mypack->FullName,*lv_parms);
			lv_parms++;
			}
		else
			{
			FreeArray(parms);
			continue;
			};
   	if(mypack->FullName[0]=='^') { mypack->FullName[0]=' '; CutSpaces(mypack->FullName);};
		strcpy(mypack->FileName,FilePart(mypack->FullName));
		ConvertStrings(mypack->Datum,mypack->Uhrzeit,FALSE);
		if(AddToArchiveList(mypack)==FALSE)
			{
			Close(mygod);mygod=NULL;
			FreePooled(mv_pool,mypack,sizeof(struct PackerList));
			MyDeleteFile(tname);
			FreeArray(parms);
			PrintDosError("InsertZip->AddToArchiveList()",ERROR_NO_FREE_STORE,TRUE,TRUE);
			}
		strcpy(mypack->Bytes,"");
    strcpy(mypack->Datum,"");
		strcpy(mypack->Uhrzeit,"");
		strcpy(mypack->FileName,"");
		strcpy(mypack->FullName,"");
		FreeArray(parms);
    };
	Close(mygod);
	FreePooled(mv_pool,mypack,sizeof(struct PackerList));
	MyDeleteFile(tname);
	return(TRUE);
	}


/*
 *  Add function which will add a given entry into the list of files
 */

STATIC BOOL AddToArchiveList(struct PackerList *fib)
	{
	if(!Fib)
		{
		if(!(Fib=(struct Liste *)AllocPooled(mv_pool,sizeof(struct Liste)))) return(FALSE);
		Fib2=Fib;
		}
	else
		{
		if(!(Fib2->next=(struct Liste *)AllocPooled(mv_pool,sizeof(struct Liste)))) return(FALSE);
		Fib2=Fib2->next;
		}
  CopyMem((APTR) fib,(APTR) Fib2,sizeof(struct PackerList));
	Fib2->next=NULL;
	Fib2->prev=NULL;
	pagecount++;
  CountIt();
	return(TRUE);
	}

/*
 *  Function to reorganize the previous pointers from the list after quicksorting it
 */

STATIC void MakePrevPtr(void)
	{
	struct	Liste *te,*first;

  Fib=quicksort(Fib);
	Fib2=te=first=Fib;
	Fib2=Fib=Fib2->next;
	while(Fib2)
		{
		Fib->prev=Fib2->prev=te;
    te=Fib2;
		Fib2=Fib=Fib2->next;
		};
	Fib2=Fib=first;
	}

/*
 *  Removes the list from memory
 */

STATIC void free_elements (struct Liste *l)
	{
	struct Liste *l_help1,*l_help2 ;
  l_help1 = l ;
  while(l_help1)
  	{
		l_help2 = l_help1 ;
    l_help1 = l_help1->next ;
    FreePooled(mv_pool,l_help2,sizeof(struct Liste));
  	}
	Fib=NULL;Fib2=NULL;choosen=pagecount=NULL;
  }

/*
 *  Main Display function taken from /X MV V2.15. Adapted to FAME Output and also optimized
 *  for better performance.
 */

STATIC void DisplayThem(char *fullname)
	{
	int			actuallines,i;
	struct	Liste *lastentry,*firstentry,*mylist;
  BOOL		br=FALSE;
  char		listhead[202],temppath[202];

	*temppath='\0';
	SPrintf(listhead,"\f\n\r[36mListing of archive [32m%s[34m\n\n\r",FilePart(fullname));
	NC_PutString(listhead,0);
	MakePrevPtr();
	pages=1L;
	while(pagecount>maxlines-4)
		{
		pagecount-=(maxlines-4);
		if(pagecount>0) pages++;
		if(pagecount<=0) {pages--;break;};
		};
	actuallines=0;
	firstentry=mylist=Fib2;
	actpage=1L;
	while(mylist)
		{
		DisplaySingleLine(mylist,34,FALSE);
		NC_PutString("",1);
		lastentry=mylist;
		mylist=mylist->next;
		actuallines++;
    if(actuallines>maxlines-5||!mylist)
			{
			switch(archivepause(firstentry,lastentry,actuallines-1))
				{
				case -1:	free_elements(Fib);
									wb(LOST_CARRIER);
        case -2:  NC_PutString(listhead,0);
									mylist=firstentry;
                  actuallines=0;
									continue;
    		case 1:		break;				// CRSR Right, also weiter anzeigen

				case 2:   for(i=0;i!=maxlines-4;i++)
										{
										if(firstentry->prev) firstentry=firstentry->prev;
										else break;
                    }
									mylist=firstentry;
									break;
				case 0:		br=TRUE;
									break;
    		};
			if(br==FALSE)
				{
				if(mylist) NC_PutString(listhead,0);
				else { SPrintf(temppath,"[H[%ldB\r[K\r",actuallines+4);NC_PutString(temppath,0);break;}
				};
			firstentry=mylist;
			actuallines=0;
			};
		if(br==TRUE) break;
		};
	}

/*
 *  *start   = Ptr. auf das erste Element dieser Seite
 *  *end     = Ptr. auf das letzte Element dieser Seite
 *  lastline = Zeile des letzten Elements
 */

STATIC int archivepause(struct Liste *start,struct Liste *end,int lastline)
	{
	struct	Liste *zwi;
	BOOL		br=FALSE,vor=FALSE,back=FALSE;
	long		mylength;
	char		glob[202];

  *glob='\0';
	NC_PutString(TEXT_1,0);
	if(start->prev) {NC_PutString(VORTEXT,0);		vor=TRUE;};
	if(end->next) 	{NC_PutString(WEITERTEXT,0);back=TRUE;};
	NC_PutString(TEXT_2,0);
	SPrintf(glob,"Page: %ld/%ld",actpage,pages);
	mylength=78-(strlen(glob));
	PutStringFormat("\r[%ldC[36mPage[33m: [32m%ld[34m/[32m%ld",mylength,actpage,pages);
  *glob='\0';
	NC_PutString("[H\n\n\n\r",0);
  zwi=start;
	do
		{
		DisplaySingleLine(zwi,33,TRUE);
		if(GetCom("",0,0,0,NR_WaitChar)<0) return(-1);
    switch(MyFAMEDoorMsg->fdom_Data2)
			{
      case ENTER:
			case KEY_ESC:
      case KEY_q:
			case KEY_Q:				br=TRUE;
												break;

			case	KEY_ASK:    DisplayOnlineHelp(vor,back);
												return(-2);				// DocFile anzeigen!
			case	ALT_DOWN:
			case	CRSR_DOWN:  DisplaySingleLine(zwi,34,FALSE);
												if(zwi==end||!zwi)
													{
													zwi=start;
													NC_PutString("[H\n\n\n\r",0);
                          }
												else
													{
													zwi=zwi->next;
													NC_PutString("\n\r",0);
													}
												break;
			case  ALT_UP:
			case	CRSR_UP:		DisplaySingleLine(zwi,34,FALSE);
												if(zwi==start||!zwi)
													{
													zwi=end;
													PutStringFormat("[%ldB",lastline);
													}
												else
                          {
													zwi=zwi->prev;
													NC_PutString("[1A",0);
													}
                        break;
      case	54:
			case 	CRSR_RIGHT:	if(back==TRUE) {actpage++;return(1);};
												break;
      case	52:
			case	CRSR_LEFT:	if(vor==TRUE) { actpage--;return(2);};
                        break;

			case 32:  				DisplaySingleLine(zwi,34,FALSE);
												if(zwi->fib.Selected==FALSE) { zwi->fib.Selected=TRUE;choosen++;}
												else { zwi->fib.Selected=FALSE;choosen--;};
												break;

			};
		}while(br==FALSE);
	return(0);
  }

/*
 *  Shows the Online Help in Archive List mode
 */

STATIC void DisplayOnlineHelp(BOOL v,BOOL b)
	{
	static int i;
	static char *hilfe[7]="\f\n\r[26C[36mHelp on Key-Commands\n\r[m[26C~~~~~~~~~~~~~~~~~~~~\n\n\r",
  											"[20C[32mCRSR Up/Down [34m= [33mChoose Entries\n\r",
												"[20C[32mCRSR Left    [34m= [33mJump to previous Page\n\r",
												"[20C[32mCRSR Right   [34m= [33mJump to next Page\n\r",
												"[20C[32m  SPACE      [34m= [33mSelect/Deselect an Entry\n\r",
												"[20C[32m Q/ENTER     [34m= [33mExit and start extracting\n\r",
												"[20C[32m    ?        [34m= [33mThis Command-Help\n\r";

	for(i=0;i<7;i++) NC_PutString(hilfe[i],0);
	Get_Key(TRUE);
	}

/*
 *  Displays the actual line, either colorized or in dark blue to show which entry is currently
 *  active.
 */

STATIC void DisplaySingleLine(struct Liste *which,int color,BOOL wie)
	{
	*mytemp2='\0';
	switch(wie)
		{
		case FALSE:	SPrintf(mytemp2,"\r  [%ldm%-30.30s %12s %9s  %8s[35m%lc\r",color,FilePart(which->fib.FileName),which->fib.Bytes,which->fib.Datum,which->fib.Uhrzeit,which->fib.Selected ? '*' : ' ');
								break;
		case TRUE:	SPrintf(mytemp2,"\r [35m>[%ldm%-30.30s %12s %9s  %8s[35m%lc\r",color,FilePart(which->fib.FileName),which->fib.Bytes,which->fib.Datum,which->fib.Uhrzeit, which->fib.Selected ? '*' : ' ');
								break;
		};
	NC_PutString(mytemp2,0);
	}

/*
 *  Function to display a rotating Line indicating that mEGA-vIEW is doing something...
 */

void CountIt(void)
	{
	STATIC 	char rotate[4][2]={"/","-","\\","|"};

  PutStringFormat("%s\b",rotate[Busy_Count]);
	Busy_Count = (Busy_Count + 1) & 0x03;
	}

/***************************************************************************************
 *  Here are now Spy's Sorting functions, which will sort the Filenames alphabetically.*
 ***************************************************************************************
 */

int sorting (struct Liste *l1,struct Liste *l2)
	{
	int 	i,erg;

  if (!l1)  return(0);
  if (!l2)  return(1);
  i=0;
  erg=2;
  do
		{
		if (ToLower (l1->fib.FileName [i]) < ToLower (l2->fib.FileName [i]))
    		erg = 1 ;
    if (ToLower (l1->fib.FileName [i]) > ToLower (l2->fib.FileName [i]))
     		erg = 0 ;
    if (l1->fib.FileName [i] == '\0')
      	erg = 1 ;
    if (l2->fib.FileName [i] == '\0')
     		erg = 0 ;
    i++;
  	}while(erg==2);
  return (erg) ;
	}

struct Liste *merge (struct Liste *l1,struct Liste *l2)
	{
  merge_end = NULL ;
  do
		{
		if (sorting (l1,l2))
    	{
			if (merge_end == NULL)
      	{
				merge_end = l1 ;
        merge_start = merge_end ;
      	}
      else
      	{
				merge_start->next = l1 ;
        merge_start = merge_start->next ;
      	}
      l1 = l1->next ;
    	}
    else
    	{
			if (merge_end == NULL)
      	{
				merge_end = l2 ;
        merge_start = merge_end ;
      	}
      else
      	{
				merge_start->next = l2 ;
        merge_start = merge_start->next ;
      	}
      l2 = l2->next ;
    	}
  	} while ((l1 != NULL) || (l2 != NULL)) ;
  merge_start->next = NULL ;
  return (merge_end) ;
	}

struct Liste *quicksort (struct Liste *l)
	{
	struct Liste *l_help1,*l_help2 ;
	int i,j ;
  if (!l)  return (l) ;
  if (!l->next) return (l) ;
  else
  	{
		i = 1 ;
    l_help1 = l ;
    while (l_help1)
    	{
			l_help1 = l_help1->next ;
      i++ ;
    	}
    l_help1 = l ;
    for (j = 1 ; j < ((i/2)-1) ; j++)	l_help1 = l_help1->next ;
    l_help2 = l_help1->next ;
    l_help1->next = NULL ;
    l_help1 = l ;
    l_help1 = quicksort (l_help1) ;
    l_help2 = quicksort (l_help2) ;
    return (merge (l_help1,l_help2)) ;
  	}
	}

