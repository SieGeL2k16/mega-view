/*
 *  --- All Prototypes used by mEGA-vIEW.FIM ---
 */

// --- Protos defined in main.o ---

extern void ShowMyTitle(BOOL);
extern void wb(char *);

// --- Protos defined in Preferences.o ---

extern void ReadSettings(void);

// --- Protos defined in Support.o ---

extern long MyDeleteFile(char *was);               		// Sets the deleted flag and removes the file
extern void ConvertStrings(UBYTE*,UBYTE*,BOOL);				// Convert dates via locale
extern void CutSpaces(char *s);
extern BOOL CheckSpecialChars(char *testline);
extern BOOL	AddDlPath(char *path);
extern void FreeDlPathes(void);
extern BOOL CheckForPattern(char *,char *);
extern LONG TestTheFile(char *);											// Checks file for restricted etc.
extern BOOL Get_Key(BOOL);														// Wait for User-Input
extern void PrintDosError(char *,long,BOOL,BOOL); 		// General DosError exit function
extern void CreateTempDir(void);											// Creates global Tempdir
extern void RemoveTempDir(void);											// Removes global tempdir
extern BOOL CheckPacker(char *,BOOL);									// Check for programs
extern long MyExecute(char *cmd);											// Executes the cmd string via PIPE:
extern void MyCutANSI(char *e);												// Enhanced CutAnsi() routine
extern BOOL AskBool(char *, BOOL);										// General Yes/No ask function
extern long CheckForCTRL(void);												// Checks for CTRL-C
extern void GetFileID(char *fullpath,long sel);				// Extracts FileID.diz from DMS/Exe
extern BOOL AddFileType(char *extension, char *cmd); 	// Adds External Filetype to List (V3.3)
extern void FreeFileTypes(void);											// Frees list of external filetypes (V3.3)


extern long NC_GetString( char*, long );
extern long PutCom( char*, long, long, ULONG, long );
extern long GetCom( char*, long, long, ULONG, long );

// --- Protos defined in IdentifyFile.o ---

extern void IdentifyFile(char *);											// Performs action according to type
extern void GetBBSFile(void);													// Gets complete path to given file
extern void ShowText(char *);                         // Main Textdisplay function
extern void ReadMP3(char *fullname);									// Shows MP3 Datas (V3.4)


// --- Protos defined in ViewArchive.o ---

extern void ViewArchive(char *,long);           			// Lists archive contents with userselection
extern void CountIt(void);														// The Busy Counter

// --- Protos defined in ExternSupport.o ---

extern long CheckExternal(char *);										// Checks File-Extension for unknown files

// --- Protos defined in Reg_Decrypt.o ---						// Include only for registered Users

#ifndef PUBLIC_RELEASE

extern  VOID  __asm Decrypt(register __a0 STRPTR);
extern ULONG 	__asm CheckRegCRC(register __d2 UWORD);	// Currently not working!

#endif

// --- Protos for MakeArray.o ---

extern char **MakeArray (char *s);										// Strings auswerten
extern void	FreeArray (char **strings);								// Strings Freigeben

// extern STRPTR __asm oFormatVarStrRDFA( register __a0 STRPTR SourceBuffer, register __a1 STRPTR DestBuffer, register __a2 struct TagItem *TagList );
