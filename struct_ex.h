/*
 *  --- Global Vars used by mEGA-vIEW.FIM ---
 */

/* Memory Pool Header */

extern APTR mv_pool;

/* System structures */

extern struct FAMELibrary		*FAMEBase;
extern struct Library 			*UtilityBase;
extern struct Library				*ASLBase;
extern struct FileIDBase		*FileIDBase;
extern struct AnchorPath		*myanchor;
extern struct FileInfoBlock	*myfib;
extern struct	FI_FileInfo 	*finfo;
extern struct	DateStamp			*ds;
extern struct	DateTime			*dt;

extern long 	node;							// Current Nodenumber

extern char	XYZ[80],						// RegisterString
						*mytemp1,						// Glob. Stringpointer
						*mytemp2,						// Glob. Stringpointer
						*readbuf,						// Buffer used to read files (ShowText() )
						*tempbuf,						// 2nd buffer used for stripping ANSI codes
						*FileName,          // Global Filename for identify
      			ActTempDir[256],		// Path for Tempfiles (usertemp + processid)
						tempfile[108];			// Path used for PIPE: device

extern char *_ProgramName;

//--- Preferences used by mEGA-vIEW ---

extern struct MV_Prefs
	{
	ULONG	MV_Switches;				// Contains up to 32 different switches
	LONG	MV_DL_Level,				// Required Level to Download files
				MV_Check,						// Required Level to check files
				MV_Sysop;						// Required Level to use the SysOpView Level
  char	MV_Pathes[17][108];	// Array containing all supported programs
	};

extern struct MV_Prefs *MyPrefs;

extern long	pathlen,					// Length of allowed chars for the file to view
						maxlines;					// Maximum lines to display for current user

extern char	viewtest[20];

extern ULONG	MV_FLAGS;				// Contains all internal settings for mEGA-vIEW

/*
 *  --- Global Errortexts ---
 */

extern char *NO_MEM,*LOST_CARRIER;

/*
 *  List containing all pathes for the current conference
 */

extern struct FAMEPathes
	{
	char	DL_Path[102];
	struct FAMEPathes *next;
	}*fp1,*fp2;

extern unsigned long	GlobFIMFlags;

/*
 *  Structure used for External Filetypes (V3.3)
 */

extern struct ExtTypes
	{
	struct 	ExtTypes *next;
	char		Extension[12],
					Command[256];
	}*et1,*et2;
