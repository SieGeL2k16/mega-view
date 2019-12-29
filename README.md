# Textviewer for FAME BBS. Recognizes over 600 Filetypes

These are the sources of my FAME Textviewer called "meGA-vIEW".

Original description:

``
This is the native FAME Version of my well-known Textviewer.
Many improvements are made (better textdisplay including full
CRSR key control, much faster, better archive support) and new
filetype support added. mEGA-vIEW is able to handle the following
filetypes directly:

EXE,ZIP,LHA,LZX,DMS,ARJ,ZOO,ZOOM,WARP,SHRINK,AGUIDE,ICONS,RAR,TGZ,HTML,MP3

Additional unknown filetypes could be also added. The archive comes together
with an installer script and a AmigaGuide manual.
``


I've successfully compiled these sources with SAS/C 6.58 on my WinUAE system, to compile just enter:

```
sc *.c PARMS=REGISTER NOSTKCHK SCODE SDATA STRIPDBG NOMINC NOICONS LINK
```

Please note that you need the FAME SDK installed to successfully compile this code!

The public release archive can be found on my homepage:

https://www.saschapfalz.de/downloads_readme.php?FID=12&CAT=0


Have fun,

SieGeL/tRSi
