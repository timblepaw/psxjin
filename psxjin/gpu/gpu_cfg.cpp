/***************************************************************************
                          cfg.c  -  description
                             -------------------
    begin                : Sun Oct 28 2001
    copyright            : (C) 2001 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/


#include "stdafx.h"

#define _IN_CFG



#include "stdafx.h"
#include "gpu_record.h"
#include <vfw.h>
#include <stdio.h>

#include "externals.h"
#include "gpu_cfg.h"
#include "gpu.h"



/////////////////////////////////////////////////////////////////////////////
// CONFIG FILE helpers
/////////////////////////////////////////////////////////////////////////////

static int tempDest; //this is for the co,mpiler to not throw in a million of warnings
char pConfigFile[MAX_PATH*2] = ".\\psxjin.ini";

#ifndef _FPSE

#include <sys/stat.h>

// some helper macros:

#define GetValue(name, var) \
 p = strstr(pB, name); \
 if (p != NULL) { \
  p+=strlen(name); \
  while ((*p == ' ') || (*p == '=')) p++; \
  if (*p != '\n') var = atoi(p); \
 }

#define GetFloatValue(name, var) \
 p = strstr(pB, name); \
 if (p != NULL) { \
  p+=strlen(name); \
  while ((*p == ' ') || (*p == '=')) p++; \
  if (*p != '\n') var = (float)atof(p); \
 }

#define SetValue(name, var) \
 p = strstr(pB, name); \
 if (p != NULL) { \
  p+=strlen(name); \
  while ((*p == ' ') || (*p == '=')) p++; \
  if (*p != '\n') { \
   len = sprintf(t1, "%d", var); \
   strncpy(p, t1, len); \
   if (p[len] != ' ' && p[len] != '\n' && p[len] != 0) p[len] = ' '; \
  } \
 } \
 else { \
  size+=sprintf(pB+size, "%s = %d\n", name, var); \
 }

#define SetFloatValue(name, var) \
 p = strstr(pB, name); \
 if (p != NULL) { \
  p+=strlen(name); \
  while ((*p == ' ') || (*p == '=')) p++; \
  if (*p != '\n') { \
   len = sprintf(t1, "%.1f", (double)var); \
   strncpy(p, t1, len); \
   if (p[len] != ' ' && p[len] != '\n' && p[len] != 0) p[len] = ' '; \
  } \
 } \
 else { \
  size+=sprintf(pB+size, "%s = %.1f\n", name, (double)var); \
 }

/////////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#ifdef _WINDOWS

/////////////////////////////////////////////////////////////////////////////
// globals

char szKeyDefaults[11]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
char szDevName[128];

////////////////////////////////////////////////////////////////////////
// prototypes

BOOL OnInitCfgDialog(HWND hW);
void OnCfgOK(HWND hW);
BOOL OnInitSoftDialog(HWND hW);
void OnSoftOK(HWND hW);
void OnCfgCancel(HWND hW);
void OnCfgDef1(HWND hW);
void OnCfgDef2(HWND hW);
void OnBugFixes(HWND hW);

//void OnRecording(HWND hW);

void SelectDev(HWND hW);
BOOL bTestModes(void);
//void OnKeyConfig(HWND hW);
void GetSettings(HWND hW);
void OnClipboard(HWND hW);
void DoDevEnum(HWND hW);
char * pGetConfigInfos(int iCfg);

////////////////////////////////////////////////////////////////////////
// funcs

BOOL CALLBACK SoftDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return OnInitSoftDialog(hW);

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_DISPMODE1:
		{
			CheckDlgButton(hW,IDC_DISPMODE2,FALSE);
			return TRUE;
		}
		case IDC_DISPMODE2:
		{
			CheckDlgButton(hW,IDC_DISPMODE1,FALSE);
			return TRUE;
		}
		case IDC_DEF1:
			OnCfgDef1(hW);
			return TRUE;
		case IDC_DEF2:
			OnCfgDef2(hW);
			return TRUE;
		case IDC_SELFIX:
			OnBugFixes(hW);
			return TRUE;	
		case IDC_SELDEV:
			SelectDev(hW);
			return TRUE;
		case IDCANCEL:
			OnCfgCancel(hW);
			return TRUE;
		case IDOK:
			OnSoftOK(hW);
			return TRUE;
		case IDC_CLIPBOARD:
			OnClipboard(hW);
			return TRUE;
		}
	}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
// init dlg
////////////////////////////////////////////////////////////////////////

void ComboBoxAddRes(HWND hWC,char * cs)
{
	int i=ComboBox_FindString(hWC,-1,cs);
	if (i!=CB_ERR) return;
	tempDest = ComboBox_AddString(hWC,cs);
}

void gpu_WriteConfig(void);

BOOL OnInitSoftDialog(HWND hW)
{
	HWND hWC;
	char cs[256];
	int i;
	DEVMODE dv;

	gpu_WriteConfig();

	if (szDevName[0])
		SetDlgItemText(hW,IDC_DEVICETXT,szDevName);

	hWC=GetDlgItem(hW,IDC_RESOLUTION);

	memset(&dv,0,sizeof(DEVMODE));
	dv.dmSize=sizeof(DEVMODE);
	i=0;

	while (EnumDisplaySettings(NULL,i,&dv))
	{
		wsprintf(cs,"%4d x %4d - default",dv.dmPelsWidth,dv.dmPelsHeight);
		ComboBoxAddRes(hWC,cs);
		if (dv.dmDisplayFrequency > 40 && dv.dmDisplayFrequency < 200 )
		{
			wsprintf(cs,"%4d x %4d , %4d Hz",dv.dmPelsWidth,dv.dmPelsHeight,dv.dmDisplayFrequency);
			ComboBoxAddRes(hWC,cs);
		}
		i++;
	}

	ComboBoxAddRes(hWC," 320 x  200 - default");
	ComboBoxAddRes(hWC," 320 x  240 - default");
	ComboBoxAddRes(hWC," 400 x  300 - default");
	ComboBoxAddRes(hWC," 512 x  384 - default");
	ComboBoxAddRes(hWC," 640 x  480 - default");
	ComboBoxAddRes(hWC," 800 x  600 - default");
	ComboBoxAddRes(hWC,"1024 x  768 - default");
	ComboBoxAddRes(hWC,"1152 x  864 - default");
	ComboBoxAddRes(hWC,"1280 x 1024 - default");
	ComboBoxAddRes(hWC,"1600 x 1200 - default");

	if (iRefreshRate)
		wsprintf(cs,"%4d x %4d , %4d Hz",iResX,iResY,iRefreshRate);
	else wsprintf(cs,"%4d x %4d - default",iResX,iResY);

	i=ComboBox_FindString(hWC,-1,cs);
	if (i==CB_ERR) i=0;
	tempDest = ComboBox_SetCurSel(hWC,i);

	hWC=GetDlgItem(hW,IDC_COLDEPTH);
	tempDest = ComboBox_AddString(hWC,"16 Bit");
	tempDest = ComboBox_AddString(hWC,"32 Bit");
	wsprintf(cs,"%d Bit",iColDepth);                      // resolution
	i=ComboBox_FindString(hWC,-1,cs);
	if (i==CB_ERR) i=0;
	tempDest = ComboBox_SetCurSel(hWC,i);

	hWC=GetDlgItem(hW,IDC_SCANLINES);
	tempDest = ComboBox_AddString(hWC,"Scanlines disabled");
	tempDest = ComboBox_AddString(hWC,"Scanlines enabled (standard)");
	tempDest = ComboBox_AddString(hWC,"Scanlines enabled (double blitting - nVidia fix)");
	tempDest = ComboBox_SetCurSel(hWC,iUseScanLines);

	SetDlgItemInt(hW,IDC_WINX,iResX,FALSE);    // window size
	SetDlgItemInt(hW,IDC_WINY,iResY,FALSE);

	if (UseFrameLimit)    CheckDlgButton(hW,IDC_USELIMIT,TRUE);
	if (UseFrameSkip)     CheckDlgButton(hW,IDC_USESKIPPING,TRUE);
	if (iWindowMode)      CheckRadioButton(hW,IDC_DISPMODE1,IDC_DISPMODE2,IDC_DISPMODE2);
	else                  CheckRadioButton(hW,IDC_DISPMODE1,IDC_DISPMODE2,IDC_DISPMODE1);
	if (iSysMemory)       CheckDlgButton(hW,IDC_SYSMEMORY,TRUE);
	if (iStopSaver)       CheckDlgButton(hW,IDC_STOPSAVER,TRUE);
	if (iUseFixes)        CheckDlgButton(hW,IDC_GAMEFIX,TRUE);
	if (bVsync)           CheckDlgButton(hW,IDC_VSYNC,TRUE);
	if (bTransparent)     CheckDlgButton(hW,IDC_TRANSPARENT,TRUE);
	if (iDebugMode)       CheckDlgButton(hW,IDC_DEBUGMODE,TRUE);
	if (bSSSPSXLimit)     CheckDlgButton(hW,IDC_SSSPSXLIMIT,TRUE);
	if (bKkaptureMode)    CheckDlgButton(hW,IDC_KKAPTURE,TRUE);

	hWC=GetDlgItem(hW,IDC_NOSTRETCH);                     // stretching
	tempDest = ComboBox_AddString(hWC,"Stretch to full window size");
	tempDest = ComboBox_AddString(hWC,"1:1 (faster with some cards)");
	tempDest = ComboBox_AddString(hWC,"Scale to window size, keep aspect ratio");
	tempDest = ComboBox_AddString(hWC,"2xSaI stretching (needs a fast cpu)");
	tempDest = ComboBox_AddString(hWC,"2xSaI unstretched (needs a fast cpu)");
	tempDest = ComboBox_AddString(hWC,"Super2xSaI stretching (needs a very fast cpu)");
	tempDest = ComboBox_AddString(hWC,"Super2xSaI unstretched (needs a very fast cpu)");
	tempDest = ComboBox_AddString(hWC,"SuperEagle stretching (needs a fast cpu)");
	tempDest = ComboBox_AddString(hWC,"SuperEagle unstretched (needs a fast cpu)");
	tempDest = ComboBox_AddString(hWC,"Scale2x stretching (needs a fast cpu)");
	tempDest = ComboBox_AddString(hWC,"Scale2x unstretched (needs a fast cpu)");
	tempDest = ComboBox_AddString(hWC,"HQ2X unstretched (Fast CPU+mmx)");
	tempDest = ComboBox_AddString(hWC,"HQ2X stretched (Fast CPU+mmx)");
	tempDest = ComboBox_AddString(hWC,"Scale3x stretching (needs a fast cpu)");
	tempDest = ComboBox_AddString(hWC,"Scale3x unstretched (needs a fast cpu)");
	tempDest = ComboBox_AddString(hWC,"HQ3X unstretched (Fast CPU+mmx)");
	tempDest = ComboBox_AddString(hWC,"HQ3X stretching (Fast CPU+mmx)");
	tempDest = ComboBox_SetCurSel(hWC,iUseNoStretchBlt);

	hWC=GetDlgItem(hW,IDC_DITHER);                        // dithering
	tempDest = ComboBox_AddString(hWC,"No dithering (fastest)");
	tempDest = ComboBox_AddString(hWC,"Game dependend dithering (slow)");
	tempDest = ComboBox_AddString(hWC,"Always dither g-shaded polygons (slowest)");
	tempDest = ComboBox_SetCurSel(hWC,iUseDither);

	if (iFrameLimit==2)                                   // frame limit wrapper
		CheckDlgButton(hW,IDC_FRAMEAUTO,TRUE);
	else CheckDlgButton(hW,IDC_FRAMEMANUELL,TRUE);

	sprintf(cs,"%.2f",fFrameRate);
	SetDlgItemText(hW,IDC_FRAMELIM,cs);                   // set frame rate

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
// on ok: take vals
////////////////////////////////////////////////////////////////////////

void GetSettings(HWND hW)
{
	HWND hWC;
	char cs[256];
	int i,j;
	char * p;

	hWC=GetDlgItem(hW,IDC_RESOLUTION);                    // get resolution
	i=ComboBox_GetCurSel(hWC);
	tempDest = ComboBox_GetLBText(hWC,i,cs);
	iResX=atol(cs);
	p=strchr(cs,'x');
	iResY=atol(p+1);
	p=strchr(cs,',');									   // added by syo
	if (p) iRefreshRate=atol(p+1);						  // get refreshrate
	else  iRefreshRate=0;

	hWC=GetDlgItem(hW,IDC_COLDEPTH);                      // get color depth
	i=ComboBox_GetCurSel(hWC);
	tempDest = ComboBox_GetLBText(hWC,i,cs);
	iColDepth=atol(cs);

	hWC=GetDlgItem(hW,IDC_SCANLINES);                     // scanlines
	iUseScanLines=ComboBox_GetCurSel(hWC);

	i=GetDlgItemInt(hW,IDC_WINX,NULL,FALSE);              // get win size
	if (i<50) i=50;
	if (i>20000) i=20000;
	j=GetDlgItemInt(hW,IDC_WINY,NULL,FALSE);
	if (j<50) j=50;
	if (j>20000) j=20000;
	iResX = i;
	iResY = j;

	if (IsDlgButtonChecked(hW,IDC_DISPMODE2))             // win mode
		iWindowMode=1;
	else iWindowMode=0;

	if (IsDlgButtonChecked(hW,IDC_USELIMIT))              // fps limit
		UseFrameLimit=1;
	else UseFrameLimit=0;

	if (IsDlgButtonChecked(hW,IDC_USESKIPPING))           // fps skip
		UseFrameSkip=1;
	else UseFrameSkip=0;

	if (IsDlgButtonChecked(hW,IDC_GAMEFIX))               // game fix
		iUseFixes=1;
	else iUseFixes=0;

	if (IsDlgButtonChecked(hW,IDC_SYSMEMORY))             // use system memory
		iSysMemory=1;
	else iSysMemory=0;

	if (IsDlgButtonChecked(hW,IDC_STOPSAVER))             // stop screen saver
		iStopSaver=1;
	else iStopSaver=0;

	if (IsDlgButtonChecked(hW,IDC_VSYNC))                 // wait VSYNC
		bVsync=bVsync_Key=TRUE;
	else bVsync=bVsync_Key=FALSE;

	if (IsDlgButtonChecked(hW,IDC_TRANSPARENT))           // transparent menu
		bTransparent=TRUE;
	else bTransparent=FALSE;

	if (IsDlgButtonChecked(hW,IDC_SSSPSXLIMIT))           // SSSPSX fps limit mode
		bSSSPSXLimit=TRUE;
	else bSSSPSXLimit=FALSE;

	if (IsDlgButtonChecked(hW,IDC_DEBUGMODE))             // debug mode
		iDebugMode=1;
	else iDebugMode=0;

	if (IsDlgButtonChecked(hW,IDC_KKAPTURE))             // .kkapture mode
		bKkaptureMode=TRUE;
	else bKkaptureMode=FALSE;

	hWC=GetDlgItem(hW,IDC_NOSTRETCH);
	iUseNoStretchBlt=ComboBox_GetCurSel(hWC);

	hWC=GetDlgItem(hW,IDC_DITHER);
	iUseDither=ComboBox_GetCurSel(hWC);

	if (IsDlgButtonChecked(hW,IDC_FRAMEAUTO))             // frame rate
		iFrameLimit=2;
	else iFrameLimit=1;

	GetDlgItemText(hW,IDC_FRAMELIM,cs,255);
	fFrameRate=(float)atof(cs);

	if (fFrameRate<1.0f)   fFrameRate=1.0f;
	if (fFrameRate>1000.0f) fFrameRate=1000.0f;
}

void OnSoftOK(HWND hW)
{
	GetSettings(hW);

	if (!iWindowMode && !bTestModes())                    // check fullscreen sets
	{
		MessageBox(hW,"Resolution/color depth not supported!","Error",MB_ICONERROR|MB_OK);
		return;
	}

	gpu_WriteConfig();

	EndDialog(hW,TRUE);
}

////////////////////////////////////////////////////////////////////////
// on clipboard button
////////////////////////////////////////////////////////////////////////

void OnClipboard(HWND hW)
{
	HWND hWE=GetDlgItem(hW,IDC_CLPEDIT);
	char * pB;
	GetSettings(hW);
	pB=pGetConfigInfos(1);

	if (pB)
	{
		SetDlgItemText(hW,IDC_CLPEDIT,pB);
		SendMessage(hWE,EM_SETSEL,0,-1);
		SendMessage(hWE,WM_COPY,0,0);
		free(pB);
		MessageBox(hW,"Configuration info successfully copied to the clipboard\nJust use the PASTE function in another program to retrieve the data!","Copy Info",MB_ICONINFORMATION|MB_OK);
	}
}

////////////////////////////////////////////////////////////////////////
// Cancel
////////////////////////////////////////////////////////////////////////

void OnCfgCancel(HWND hW)
{
	EndDialog(hW,FALSE);
}

////////////////////////////////////////////////////////////////////////
// Bug fixes
////////////////////////////////////////////////////////////////////////

BOOL CALLBACK BugFixesDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		int i;

		for (i=0;i<32;i++)
		{
			if (dwCfgFixes&(1<<i))
				CheckDlgButton(hW,IDC_FIX1+i,TRUE);
		}
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hW,FALSE);
			return TRUE;

		case IDOK:
		{
			int i;
			dwCfgFixes=0;
			for (i=0;i<32;i++)
			{
				if (IsDlgButtonChecked(hW,IDC_FIX1+i))
					dwCfgFixes|=(1<<i);
			}
			EndDialog(hW,TRUE);
			return TRUE;
		}
		}
	}
	}
	return FALSE;
}

void OnBugFixes(HWND hW)
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_FIXES),
	          hW,(DLGPROC)BugFixesDlgProc);
}

////////////////////////////////////////////////////////////////////////
// Recording options
////////////////////////////////////////////////////////////////////////

bool HACK_CODEC_CHOOSE(HWND hW)
{
	{		
		BITMAPINFOHEADER bitmap = {40,640,480,1,24,0,640*480*3,2048,2048,0,0};
		if (!ICCompressorChoose(hW,ICMF_CHOOSE_DATARATE|ICMF_CHOOSE_KEYFRAME,&bitmap,NULL,&RECORD_COMPRESSION2,"24 bit Compression")) return false;
		printf("%d %d \n",RECORD_COMPRESSION2.cbState,sizeof(RECORD_COMPRESSION_STATE2));
		if (RECORD_COMPRESSION2.cbState>sizeof(RECORD_COMPRESSION_STATE2))
		{
			memset(&RECORD_COMPRESSION2,0,sizeof(RECORD_COMPRESSION2));
			memset(&RECORD_COMPRESSION_STATE2,0,sizeof(RECORD_COMPRESSION_STATE2));
			RECORD_COMPRESSION2.cbSize	= sizeof(RECORD_COMPRESSION2);			
		}
		else
		{
			if (RECORD_COMPRESSION2.lpState!=RECORD_COMPRESSION_STATE2)
				memcpy(RECORD_COMPRESSION_STATE2,RECORD_COMPRESSION2.lpState,RECORD_COMPRESSION2.cbState);
		}
		RECORD_COMPRESSION2.lpState = RECORD_COMPRESSION_STATE2;
	}
	//RefreshCodec(hW);
	return true;
}

////////////////////////////////////////////////////////////////////////
// default 1: fast
////////////////////////////////////////////////////////////////////////

void OnCfgDef1(HWND hW)
{
	HWND hWC;

	hWC=GetDlgItem(hW,IDC_RESOLUTION);
	tempDest = ComboBox_SetCurSel(hWC,1);
	hWC=GetDlgItem(hW,IDC_COLDEPTH);
	tempDest = ComboBox_SetCurSel(hWC,0);
	hWC=GetDlgItem(hW,IDC_SCANLINES);
	tempDest = ComboBox_SetCurSel(hWC,0);
	CheckDlgButton(hW,IDC_USELIMIT,FALSE);
	CheckDlgButton(hW,IDC_USESKIPPING,TRUE);
	CheckRadioButton(hW,IDC_DISPMODE1,IDC_DISPMODE2,IDC_DISPMODE1);
	CheckDlgButton(hW,IDC_FRAMEAUTO,FALSE);
	CheckDlgButton(hW,IDC_FRAMEMANUELL,TRUE);
	CheckDlgButton(hW,IDC_SHOWFPS,FALSE);
	hWC=GetDlgItem(hW,IDC_NOSTRETCH);
	tempDest = ComboBox_SetCurSel(hWC,1);
	hWC=GetDlgItem(hW,IDC_DITHER);
	tempDest = ComboBox_SetCurSel(hWC,0);
	SetDlgItemInt(hW,IDC_FRAMELIM,200,FALSE);
	SetDlgItemInt(hW,IDC_WINX,320,FALSE);
	SetDlgItemInt(hW,IDC_WINY,240,FALSE);
	CheckDlgButton(hW,IDC_VSYNC,FALSE);
	CheckDlgButton(hW,IDC_TRANSPARENT,TRUE);
	CheckDlgButton(hW,IDC_DEBUGMODE,FALSE);
	CheckDlgButton(hW,IDC_KKAPTURE,FALSE);
}

////////////////////////////////////////////////////////////////////////
// default 2: nice
////////////////////////////////////////////////////////////////////////

void OnCfgDef2(HWND hW)
{
	HWND hWC;

	hWC=GetDlgItem(hW,IDC_RESOLUTION);
	tempDest = ComboBox_SetCurSel(hWC,2);
	hWC=GetDlgItem(hW,IDC_COLDEPTH);
	tempDest = ComboBox_SetCurSel(hWC,0);
	hWC=GetDlgItem(hW,IDC_SCANLINES);
	tempDest = ComboBox_SetCurSel(hWC,0);
	CheckDlgButton(hW,IDC_USELIMIT,TRUE);
	CheckDlgButton(hW,IDC_USESKIPPING,FALSE);
	CheckRadioButton(hW,IDC_DISPMODE1,IDC_DISPMODE2,IDC_DISPMODE1);
	CheckDlgButton(hW,IDC_FRAMEAUTO,TRUE);
	CheckDlgButton(hW,IDC_FRAMEMANUELL,FALSE);
	CheckDlgButton(hW,IDC_SHOWFPS,FALSE);
	CheckDlgButton(hW,IDC_VSYNC,FALSE);
	CheckDlgButton(hW,IDC_TRANSPARENT,TRUE);
	CheckDlgButton(hW,IDC_DEBUGMODE,FALSE);
	CheckDlgButton(hW,IDC_KKAPTURE,FALSE);
	hWC=GetDlgItem(hW,IDC_NOSTRETCH);
	tempDest = ComboBox_SetCurSel(hWC,0);
	hWC=GetDlgItem(hW,IDC_DITHER);
	tempDest = ComboBox_SetCurSel(hWC,2);

	SetDlgItemInt(hW,IDC_FRAMELIM,200,FALSE);
	SetDlgItemInt(hW,IDC_WINX,640,FALSE);
	SetDlgItemInt(hW,IDC_WINY,480,FALSE);
}

////////////////////////////////////////////////////////////////////////
// read config
////////////////////////////////////////////////////////////////////////

void gpu_ReadConfig(void)
{
	char Conf_File[256];
	char Str_Tmp[256];
	memset(szDevName,0,128);
	memset(&guiDev,0,sizeof(GUID));

	strcpy(Conf_File, ".\\psxjin.ini");	//TODO: make a global for other files
	
	iResX = GetPrivateProfileInt("GPU", "iResX", 320, Conf_File);
	iResY = GetPrivateProfileInt("GPU", "iResY", 240, Conf_File);
	iRefreshRate = GetPrivateProfileInt("GPU", "iRefreshRate", 0, Conf_File);
	iWindowMode = GetPrivateProfileInt("GPU", "iWindowMode", 1, Conf_File);
	iColDepth = GetPrivateProfileInt("GPU", "iColDepth", 16, Conf_File);
	UseFrameLimit = GetPrivateProfileInt("GPU", "UseFrameLimit", 1, Conf_File);
	UseFrameSkip = GetPrivateProfileInt("GPU", "UseFrameSkip", 0, Conf_File);
	iFrameLimit = GetPrivateProfileInt("GPU", "iFrameLimit", 2, Conf_File);
	dwCfgFixes = GetPrivateProfileInt("GPU", "dwCfgFixes", 0, Conf_File);
	iUseFixes = GetPrivateProfileInt("GPU", "iUseFixes", 0, Conf_File);
	iUseScanLines = GetPrivateProfileInt("GPU", "iUseScanLines", 0, Conf_File);
	iUseNoStretchBlt = GetPrivateProfileInt("GPU", "iUseNoStretchBlt", 0, Conf_File);
	iUseDither = GetPrivateProfileInt("GPU", "iUseDither", 0, Conf_File);
	iUseGammaVal = GetPrivateProfileInt("GPU", "iUseGammaVal", 2048, Conf_File);

	if (!iFrameLimit)
	{
		UseFrameLimit=1;
		UseFrameSkip=0;
		iFrameLimit=2;
	}
	
	GetPrivateProfileString("GPU", "fFrameRate", "200.0", &Str_Tmp[0], 256, Conf_File);	
	fFrameRate = atof(&Str_Tmp[0]);
	iSysMemory = GetPrivateProfileInt("GPU", "iSysMemory", 0, Conf_File);
	iStopSaver = GetPrivateProfileInt("GPU", "iStopSaver", 0, Conf_File);
	bVsync = bVsync_Key = GetPrivateProfileInt("GPU", "bVsync", 0, Conf_File);
	bTransparent = GetPrivateProfileInt("GPU", "bTransparent", 0, Conf_File);
	bSSSPSXLimit = GetPrivateProfileInt("GPU", "bSSSPSXLimit", 0, Conf_File);
	iDebugMode = GetPrivateProfileInt("GPU", "iDebugMode", 0, Conf_File);
	bKkaptureMode = GetPrivateProfileInt("GPU", "bKkaptureMode", 0, Conf_File);
	
	GetPrivateProfileString("GPU", "DeviceName", 0, &szDevName[0], 128, Conf_File);	

	//Recording options
	RECORD_RECORDING_MODE = GetPrivateProfileInt("GPU", "RECORD_RECORDING_MODE", 0, Conf_File);
	RECORD_VIDEO_SIZE = GetPrivateProfileInt("GPU", "RECORD_VIDEO_SIZE", 0, Conf_File);
	RECORD_RECORDING_WIDTH = GetPrivateProfileInt("GPU", "RECORD_RECORDING_WIDTH", 0, Conf_File);
	RECORD_RECORDING_HEIGHT = GetPrivateProfileInt("GPU", "RECORD_RECORDING_HEIGHT", 0, Conf_File);
	RECORD_FRAME_RATE_SCALE = GetPrivateProfileInt("GPU", "RECORD_FRAME_RATE_SCALE", 0, Conf_File);
	RECORD_COMPRESSION_MODE = GetPrivateProfileInt("GPU", "RECORD_COMPRESSION_MODE", 0, Conf_File);
	
//	RECORD_COMPRESSION1 = GetPrivateProfileInt("GPU", "RECORD_COMPRESSION1", 0, Conf_File);
	
	GetPrivateProfileString("GPU", "RECORD_COMPRESSION_STATE2", 0, (LPSTR)&RECORD_COMPRESSION_STATE2[0], 4096, Conf_File);
	
	if (RECORD_RECORDING_WIDTH>1024) RECORD_RECORDING_WIDTH = 1024;
	if (RECORD_RECORDING_HEIGHT>768) RECORD_RECORDING_HEIGHT = 768;
	if (RECORD_VIDEO_SIZE>2) RECORD_VIDEO_SIZE = 2;
	if (RECORD_FRAME_RATE_SCALE>7) RECORD_FRAME_RATE_SCALE = 7;

	guiDev.Data1 = GetPrivateProfileInt("GPU", "GUID1", 0, Conf_File);
	guiDev.Data2 = GetPrivateProfileInt("GPU", "GUID2", 0, Conf_File);
	guiDev.Data3 = GetPrivateProfileInt("GPU", "GUID3", 0, Conf_File);
	GetPrivateProfileString("GPU", "GUID4", 0, (LPSTR)&guiDev.Data4[0], 8, Conf_File);

	memset(&RECORD_COMPRESSION2,0,sizeof(RECORD_COMPRESSION2));
	RECORD_COMPRESSION2.cbSize = sizeof(RECORD_COMPRESSION2);
	RECORD_COMPRESSION2.lpState = RECORD_COMPRESSION_STATE2;

	if (!iColDepth) iColDepth=32;
	if (iUseFixes) dwActFixes=dwCfgFixes;
	SetFixes();

	if (iUseGammaVal<0 || iUseGammaVal>1536) iUseGammaVal=2048;
}

////////////////////////////////////////////////////////////////////////

void ReadWinSizeConfig(void)
{
	char Conf_File[256];

	strcpy(Conf_File, ".\\psxjin.ini");	//TODO: make a global for other files
	
	iResX = GetPrivateProfileInt("GPU", "iResX", 320, Conf_File);
	iResY = GetPrivateProfileInt("GPU", "iResY", 240, Conf_File);	
}

////////////////////////////////////////////////////////////////////////
// write config
////////////////////////////////////////////////////////////////////////

void gpu_WriteConfig(void)
{
	char Conf_File[1024] = ".\\psxjin.ini";	//TODO: make a global for other files
	char Str_Tmp[1024];

	sprintf(Str_Tmp, "%d", iResX);
	WritePrivateProfileString("GPU", "iResX", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iResY);
	WritePrivateProfileString("GPU", "iResY", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iRefreshRate);
	WritePrivateProfileString("GPU", "iRefreshRate", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iWindowMode);
	WritePrivateProfileString("GPU", "iWindowMode", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iColDepth);
	WritePrivateProfileString("GPU", "iColDepth", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", UseFrameLimit);
	WritePrivateProfileString("GPU", "UseFrameLimit", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", UseFrameSkip);
	WritePrivateProfileString("GPU", "UseFrameSkip", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iFrameLimit);
	WritePrivateProfileString("GPU", "iFrameLimit", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", dwCfgFixes);
	WritePrivateProfileString("GPU", "dwCfgFixes", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iUseFixes);
	WritePrivateProfileString("GPU", "iUseFixes", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iUseScanLines);
	WritePrivateProfileString("GPU", "iUseScanLines", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iUseNoStretchBlt);
	WritePrivateProfileString("GPU", "iUseNoStretchBlt", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iUseDither);
	WritePrivateProfileString("GPU", "iUseDither", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iUseGammaVal);
	WritePrivateProfileString("GPU", "iUseGammaVal", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%f", fFrameRate);
	WritePrivateProfileString("GPU", "fFrameRate", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iSysMemory);
	WritePrivateProfileString("GPU", "iSysMemory", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iStopSaver);
	WritePrivateProfileString("GPU", "iStopSaver", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", bVsync);
	WritePrivateProfileString("GPU", "bVsync", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", bTransparent);
	WritePrivateProfileString("GPU", "bTransparent", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", bSSSPSXLimit);
	WritePrivateProfileString("GPU", "bSSSPSXLimit", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", iDebugMode);
	WritePrivateProfileString("GPU", "iDebugMode", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", bKkaptureMode);
	WritePrivateProfileString("GPU", "bKkaptureMode", Str_Tmp, Conf_File);
	
	WritePrivateProfileString("GPU", "DeviceName", szDevName, Conf_File);
//*****
//		WritePrivateProfileString("GPU", "guiDev", &guiDev, Conf_File);
	

	if (RECORD_COMPRESSION2.cbState>sizeof(RECORD_COMPRESSION_STATE2) || RECORD_COMPRESSION2.lpState!=RECORD_COMPRESSION_STATE2)
	{
		memset(&RECORD_COMPRESSION2,0,sizeof(RECORD_COMPRESSION2));
		memset(&RECORD_COMPRESSION_STATE2,0,sizeof(RECORD_COMPRESSION_STATE2));
		RECORD_COMPRESSION2.cbSize	= sizeof(RECORD_COMPRESSION2);
		RECORD_COMPRESSION2.lpState = RECORD_COMPRESSION_STATE2;
	}

	sprintf(Str_Tmp, "%d", RECORD_RECORDING_MODE);
	WritePrivateProfileString("GPU", "RECORD_RECORDING_MODE", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", RECORD_VIDEO_SIZE);
	WritePrivateProfileString("GPU", "RECORD_VIDEO_SIZE", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", RECORD_RECORDING_WIDTH);
	WritePrivateProfileString("GPU", "RECORD_RECORDING_WIDTH", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", RECORD_RECORDING_HEIGHT);
	WritePrivateProfileString("GPU", "RECORD_RECORDING_HEIGHT", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", RECORD_FRAME_RATE_SCALE);
	WritePrivateProfileString("GPU", "RECORD_FRAME_RATE_SCALE", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", RECORD_COMPRESSION_MODE);
	WritePrivateProfileString("GPU", "RECORD_COMPRESSION_MODE", Str_Tmp, Conf_File);

	WritePrivateProfileString("GPU", "iDebugMode", Str_Tmp, Conf_File);
	WritePrivateProfileString("GPU", "RECORD_COMPRESSION_STATE2", (LPSTR)RECORD_COMPRESSION_STATE2, Conf_File);

	sprintf(Str_Tmp, "%d", guiDev.Data1);
	WritePrivateProfileString("GPU", "GUID1", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", guiDev.Data2);
	WritePrivateProfileString("GPU", "GUID2", Str_Tmp, Conf_File);
	sprintf(Str_Tmp, "%d", guiDev.Data3);
	WritePrivateProfileString("GPU", "GUID3", Str_Tmp, Conf_File);
	WritePrivateProfileString("GPU", "GUID4", (LPSTR)guiDev.Data4, Conf_File);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND gHWND;

static HRESULT WINAPI Enum3DDevicesCallback( GUID* pGUID, LPSTR strDesc,
    LPSTR strName, LPD3DDEVICEDESC pHALDesc,
    LPD3DDEVICEDESC pHELDesc, LPVOID pvContext )
{
	BOOL IsHardware;

// Check params
	if ( NULL==pGUID || NULL==pHALDesc || NULL==pHELDesc)
		return D3DENUMRET_CANCEL;

// Handle specific device GUIDs. NullDevice renders nothing
	if ( IsEqualGUID( *pGUID, IID_IDirect3DNullDevice ) )
		return D3DENUMRET_OK;

	IsHardware = ( 0 != pHALDesc->dwFlags );
	if (!IsHardware) return D3DENUMRET_OK;

	bDeviceOK=TRUE;

	return D3DENUMRET_OK;
}

static BOOL WINAPI DirectDrawEnumCallbackEx( GUID FAR* pGUID, LPSTR strDesc,
    LPSTR strName, VOID* pV,
    HMONITOR hMonitor )
{
// Use the GUID to create the DirectDraw object, so that information
// can be extracted from it.

	LPDIRECTDRAW pDD;
	LPDIRECTDRAW4 g_pDD;
	LPDIRECT3D3 pD3D;

	if ( FAILED( DirectDrawCreate( pGUID, &pDD, 0L ) ) )
	{
		return D3DENUMRET_OK;
	}

// Query the DirectDraw driver for access to Direct3D.
	if ( FAILED(IDirectDraw_QueryInterface(pDD, IID_IDirectDraw4, (VOID**)&g_pDD)))
	{
		IDirectDraw_Release(pDD);
		return D3DENUMRET_OK;
	}
	IDirectDraw_Release(pDD);

// Query the DirectDraw driver for access to Direct3D.

	if ( FAILED( IDirectDraw4_QueryInterface(g_pDD,IID_IDirect3D3, (VOID**)&pD3D)))
	{
		IDirectDraw4_Release(g_pDD);
		return D3DENUMRET_OK;
	}

	bDeviceOK=FALSE;

// Now, enumerate all the 3D devices
	IDirect3D3_EnumDevices(pD3D,Enum3DDevicesCallback,NULL);

	if (bDeviceOK)
	{
		HWND hWC=GetDlgItem(gHWND,IDC_DEVICE);
		int i=ComboBox_AddString(hWC,strDesc);
		GUID * g=(GUID *)malloc(sizeof(GUID));
		if (NULL != pGUID) *g=*pGUID;
		else              memset(g,0,sizeof(GUID));
		tempDest = ComboBox_SetItemData(hWC,i,g);
	}

	IDirect3D3_Release(pD3D);
	IDirectDraw4_Release(g_pDD);
	return DDENUMRET_OK;
}

//-----------------------------------------------------------------------------

static BOOL WINAPI DirectDrawEnumCallback( GUID FAR* pGUID, LPSTR strDesc,
    LPSTR strName, VOID* pV)
{
	return DirectDrawEnumCallbackEx( pGUID, strDesc, strName, NULL, NULL );
}

//-----------------------------------------------------------------------------

void DoDevEnum(HWND hW)
{
	LPDIRECTDRAWENUMERATEEX pDDrawEnumFn;

	HMODULE hDDrawDLL = GetModuleHandle("DDRAW.DLL");
	if (NULL == hDDrawDLL) return;

	gHWND=hW;

	pDDrawEnumFn = (LPDIRECTDRAWENUMERATEEX)
	               GetProcAddress( hDDrawDLL, "DirectDrawEnumerateExA" );

	if (pDDrawEnumFn)
		pDDrawEnumFn( DirectDrawEnumCallbackEx, NULL,
		              DDENUM_ATTACHEDSECONDARYDEVICES |
		              DDENUM_DETACHEDSECONDARYDEVICES |
		              DDENUM_NONDISPLAYDEVICES );
	else
		DirectDrawEnumerate( DirectDrawEnumCallback, NULL );
}

////////////////////////////////////////////////////////////////////////

void FreeGui(HWND hW)
{
	int i,iCnt;
	HWND hWC=GetDlgItem(hW,IDC_DEVICE);
	iCnt=ComboBox_GetCount(hWC);
	for (i=0;i<iCnt;i++)
	{
		free((GUID *)ComboBox_GetItemData(hWC,i));
	}
}

////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DeviceDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hWC;
		int i;
		DoDevEnum(hW);
		hWC=GetDlgItem(hW,IDC_DEVICE);
		i=ComboBox_FindStringExact(hWC,-1,szDevName);
		if (i==CB_ERR) i=0;
		tempDest = ComboBox_SetCurSel(hWC,i);
		hWC=GetDlgItem(hW,IDC_GAMMA);
		ScrollBar_SetRange(hWC,0,1024,FALSE);
		if (iUseGammaVal==2048) ScrollBar_SetPos(hWC,512,FALSE);
		else
		{
			ScrollBar_SetPos(hWC,iUseGammaVal,FALSE);
			CheckDlgButton(hW,IDC_USEGAMMA,TRUE);
		}
	}

	case WM_HSCROLL:
	{
		HWND hWC=GetDlgItem(hW,IDC_GAMMA);
		int pos=ScrollBar_GetPos(hWC);
		switch (LOWORD(wParam))
		{
		case SB_THUMBPOSITION:
			pos=HIWORD(wParam);
			break;
		case SB_LEFT:
			pos=0;
			break;
		case SB_RIGHT:
			pos=1024;
			break;
		case SB_LINELEFT:
			pos-=16;
			break;
		case SB_LINERIGHT:
			pos+=16;
			break;
		case SB_PAGELEFT:
			pos-=128;
			break;
		case SB_PAGERIGHT:
			pos+=128;
			break;

		}
		ScrollBar_SetPos(hWC,pos,TRUE);
		return TRUE;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			FreeGui(hW);
			EndDialog(hW,FALSE);
			return TRUE;
		case IDOK:
		{
			HWND hWC=GetDlgItem(hW,IDC_DEVICE);
			int i=ComboBox_GetCurSel(hWC);
			if (i==CB_ERR) return TRUE;
			guiDev=*((GUID *)ComboBox_GetItemData(hWC,i));
			tempDest = ComboBox_GetLBText(hWC,i,szDevName);
			FreeGui(hW);

			if (!IsDlgButtonChecked(hW,IDC_USEGAMMA))
				iUseGammaVal=2048;
			else
				iUseGammaVal=ScrollBar_GetPos(GetDlgItem(hW,IDC_GAMMA));

			EndDialog(hW,TRUE);
			return TRUE;
		}
		}
	}
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////

void SelectDev(HWND hW)
{
	if (DialogBox(hInst,MAKEINTRESOURCE(IDD_DEVICE),
	              hW,(DLGPROC)DeviceDlgProc)==IDOK)
	{
		SetDlgItemText(hW,IDC_DEVICETXT,szDevName);
	}
}


////////////////////////////////////////////////////////////////////////

static HRESULT WINAPI EnumDisplayModesCallback( DDSURFACEDESC2* pddsd,
    VOID* pvContext )
{
	if (NULL==pddsd) return DDENUMRET_CANCEL;

	if (pddsd->ddpfPixelFormat.dwRGBBitCount==(unsigned int)iColDepth &&
	    pddsd->dwWidth==(unsigned int)iResX &&
	    pddsd->dwHeight==(unsigned int)iResY)
	{
		bDeviceOK=TRUE;
		return DDENUMRET_CANCEL;
	}

	return DDENUMRET_OK;
}

////////////////////////////////////////////////////////////////////////

BOOL bTestModes(void)
{
	LPDIRECTDRAW pDD;
	LPDIRECTDRAW4 g_pDD;

	GUID FAR * guid=0;
	int i;
	unsigned char * c=(unsigned char *)&guiDev;
	for (i=0;i<sizeof(GUID);i++,c++)
	{
		if (*c)
		{
			guid=&guiDev;
			break;
		}
	}

	bDeviceOK=FALSE;

	if ( FAILED( DirectDrawCreate(guid, &pDD, 0L ) ) )
		return FALSE;

	if (FAILED(IDirectDraw_QueryInterface(pDD, IID_IDirectDraw4, (VOID**)&g_pDD)))
	{
		IDirectDraw_Release(pDD);
		return FALSE;
	}
	IDirectDraw_Release(pDD);

	IDirectDraw4_EnumDisplayModes(g_pDD,0,NULL,NULL,EnumDisplayModesCallback);

	IDirectDraw4_Release(g_pDD);

	return bDeviceOK;
}

////////////////////////////////////////////////////////////////////////
// define key dialog
////////////////////////////////////////////////////////////////////////


#endif