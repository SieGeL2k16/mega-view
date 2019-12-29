/*
 *  --- Global Vars used by mEGA-vIEW.FIM ---
 */

/* Memory Pool Header */

APTR mv_pool=NULL;

/* System structures */

struct 	FAMELibrary		*FAMEBase=NULL;
struct 	FileIDBase		*FileIDBase=NULL;
struct 	Library 			*UtilityBase=NULL;
struct  Library				*ASLBase=NULL;
struct 	AnchorPath		*myanchor=NULL;
struct 	FileInfoBlock	*myfib=NULL;
struct	FI_FileInfo 	*finfo=NULL;
struct	DateStamp			*ds=NULL;
struct	DateTime			*dt=NULL;

long 	node=NULL;								// Current Nodenumber

char	XYZ[80],									// RegisterString
			*mytemp1=NULL,						// Glob. Stringpointer
			*mytemp2=NULL,						// Glob. Stringpointer
			*readbuf=NULL,						// Buffer used to read files (ShowText() )
			*tempbuf=NULL,						// 2nd buffer used for stripping ANSI codes
      *FileName=NULL,						// FileName to use
      ActTempDir[256],					// Path for Tempfiles (usertemp + processid)
			tempfile[108];						// Path used for PIPE: device

//--- Preferences used by mEGA-vIEW ---

struct MV_Prefs
	{
	ULONG	MV_Switches;						// Contains up to 32 different switches
	LONG	MV_DL_Level,						// Required Level to Download files
				MV_Check,								// Required Level to view & check files
				MV_Sysop;								// Required Level to use the SysOpView Level
	char	MV_Pathes[17][108];			// Array containing all supported programs
	};

struct MV_Prefs *MyPrefs=NULL;	// Memory pointer for above struct

long	pathlen=12,								// Length of allowed chars for the file to view
			maxlines=24;							// Maximum lines to display for current user
char	viewtest[20];							// Buffer to hold 'view' or 'test' string

ULONG	MV_FLAGS=NULL;						// Contains all internal settings for mEGA-vIEW

/*
 *  --- Global Errortexts ---
 */

char 	*NO_MEM="\n\n\r[37mNo memory free anymore, please reset your machine ![m",
      *LOST_CARRIER="\n\n\r[37mLOST THE CARRIER !!!";

/*
 *  List containing all conferences for the current conference
 */

struct FAMEPathes
	{
	char		DL_Path[102];
	struct 	FAMEPathes *next;
	}*fp1,*fp2;

extern unsigned long	GlobFIMFlags;

/*
 *  Structure used for External Filetypes (V3.3)
 */

struct ExtTypes
	{
	struct 	ExtTypes *next;
	char		Extension[12],
					Command[256];
	}*et1,*et2;
