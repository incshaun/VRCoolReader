/***************************************************************************
 *   Copyright (C) 2011 Stephan Olbrich reader42@gmx.de                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "cr3inkview.h"

#include "cr3main.h"
#include "mainwnd.h"

#include <inkview.h>

#define CR3PATH USERDATA "/share/crengine"

#define LOGFILE CR3PATH "/cr3log.ini"
#define SKINPATH CR3PATH "/skins/default"
#define FONTPATH SYSTEMDATA "/fonts/"
#define FONTPATH2 USERDATA "/fonts/"
#define HYPHPATH CR3PATH "/hyph/"
#define CRCACHEPATH CR3PATH "/.cache"
#define KEYMAPPATH CR3PATH "/keymaps/"
#define BOOKMARKPATH CR3PATH "/bookmarks/"
#define HISTFILE CR3PATH "/cr3hist"

CRInkViewWindowManager * wm;

void CRInkViewScreen::update(const lvRect& rc, bool full)
{
    char buf[2000];
    CRLog::trace("CRInkViewScreen::update(%d)", full ? 1 : 0);
    if ( rc.height()>400 && checkFullUpdateCounter() )
        full = true;

    if (!full)
    {
        CRLog::trace("CRInkViewScreen::update() PartialUpdate(%d, %d, %d, %d)", rc.left, rc.top, rc.width(), rc.height());
        Stretch(_front->GetScanLine(rc.top), IMAGE_GRAY8, _front->GetWidth(), rc.height(), _front->GetRowSize(), 0, rc.top, _front->GetWidth(), rc.height(), 0);
        DitherArea(0, rc.top, _front->GetWidth(), rc.height(), 16, DITHER_PATTERN);
        PartialUpdate(rc.left, rc.top, rc.width(), rc.height());
    }
    else
    {
        CRLog::trace("CRInkViewScreen::update() FullUpdate()");
        Stretch(_front->GetScanLine(0), IMAGE_GRAY8, _front->GetWidth(), _front->GetHeight(), _front->GetRowSize(), 0, 0, _front->GetWidth(), _front->GetHeight(), 0);
        DitherArea(0, 0, _front->GetWidth(), _front->GetHeight(), 16, DITHER_PATTERN);
        FullUpdate();
    }
    CRLog::trace("_fullUpdateInterval: %d, _fullUpdateCounter: %d  ", _fullUpdateInterval, _fullUpdateCounter);
}

CRInkViewScreen::CRInkViewScreen(int width, int height): CRGUIScreenBase(width, height, true)
{

}

void CRInkViewWindowManager::closeAllWindows()
{
    CRGUIWindowManager::closeAllWindows();
    CloseApp();
}


CRInkViewWindowManager::CRInkViewWindowManager(int width, int height): CRGUIWindowManager(NULL)
{
    _screen = new CRInkViewScreen(width, height);
}

void CRInkViewWindowManager::update(bool fullScreenUpdate, bool forceFlushScreen)
{
    CRLog::trace("CRInkViewWindowManager::update(%d, %d)", fullScreenUpdate ? 1 : 0, forceFlushScreen ? 1 : 0);
    CRGUIWindowManager::update(fullScreenUpdate, forceFlushScreen);
}

bool CRInkViewWindowManager::getBatteryStatus(int& percent, bool& charging)
{
    percent = GetBatteryPower();
    charging = (IsCharging() == 1) ? true : false;
    return true;
}


int InitDoc(char *fileName)
{
    static const lChar16 * css_file_name = L"fb2.css"; // fb2

    CRLog::trace("InitDoc()");
    CRLog::trace("creating window manager...");
    wm = new CRInkViewWindowManager(600,800);

    CRLog::trace("loading skin...");
    wm->loadSkin(  lString16( SKINPATH ) );

    lString16Collection fontDirs;
    fontDirs.add( lString16( FONTPATH ) );
    fontDirs.add( lString16( FONTPATH2 ) );
    CRLog::info("INIT...");
    if ( !InitCREngine( CR3PATH, fontDirs ) )
        return 0;

    const char * keymap_locations [] = {
        KEYMAPPATH,
        NULL,
    };
    loadKeymaps( *wm, keymap_locations );
    HyphMan::initDictionaries( lString16( HYPHPATH ) );
    if ( !ldomDocCache::init( lString16( CRCACHEPATH ), 0x100000 * 64 ) ) {
        CRLog::error("Cannot initialize swap directory");
    }
    CRLog::trace("creating main window...");
    V3DocViewWin * main_win = new V3DocViewWin( wm, lString16(CR3PATH) );    
//    if ( manual_file[0] )
//        main_win->setHelpFile( lString16( manual_file ) );
//    main_win->loadDefaultCover( lString16( L"/root/abook/crengine/cr3_def_cover.png" ) )

    const lChar16 * ini_fname = L"cr3.ini";
#ifdef SEPARATE_INI_FILES
    if ( strstr(fileName, ".txt")!=NULL || strstr(fileName, ".tcr")!=NULL) {
        ini_fname = L"cr3-txt.ini";
        css_file_name = L"txt.css";
    } else if ( strstr(fileName, ".rtf")!=NULL ) {
        ini_fname = L"cr3-rtf.ini";
        css_file_name = L"rtf.css";
    } else if ( strstr(fileName, ".htm")!=NULL ) {
        ini_fname = L"cr3-htm.ini";
        css_file_name = L"htm.css";
    } else if ( strstr(fileName, ".epub")!=NULL ) {
        ini_fname = L"cr3-epub.ini";
        css_file_name = L"epub.css";
    } else {
        ini_fname = L"cr3-fb2.ini";
        css_file_name = L"fb2.css";
    }
#endif
    CRLog::trace("loading CSS...");
    main_win->loadCSS(  lString16( CR3PATH ) + lString16( L"/" ) + lString16(css_file_name) );

    CRLog::trace("loading Settings...");
    lString16 _ini = lString16( CR3PATH )  + lString16( L"/" ) + ini_fname;
    if ( !main_win->loadSettings( _ini )) {
        CRLog::error("Cannot read settings from file %s", _ini.c_str());
    }
    if ( !main_win->loadHistory( lString16(HISTFILE) ) ) {
        CRLog::error("Cannot read history file %s", HISTORYFILE);
    }

    main_win->setBookmarkDir( lString16(BOOKMARKPATH) );
    wm->activateWindow( main_win );
    if ( !main_win->loadDocument( lString16(fileName) ) ) {
        CRLog::error("Cannot open book file %s\n", fileName);
        delete wm;
        return 0;
    }
    wm->handleAllEvents(false);
    return 1;
}

char * fname;

int main_handler(int type, int par1, int par2)
{
    static int longtouch;
    switch (type) {
        case EVT_INIT:
            CRLog::trace("EVT_INIT");
            if ( !InitDoc( fname ) ) {
                CloseApp();
            }
            break;
        case EVT_SHOW:
            CRLog::trace("EVT_SHOW");
            wm->update(true, true);
            break;
        case EVT_KEYRELEASE:
            CRLog::trace("EVT_KEYRELEASE 0x%x %d", par1, par2);
            par2 = (par2 > 0) ? 1 : 0;
            switch (par1) {
                default:
                    wm->postEvent( new CRGUIKeyDownEvent(par1, par2) );
                    wm->handleAllEvents(false);
                    break;
            }
            break;
        case EVT_POINTERDOWN:
            CRLog::trace("EVT_POINTERDOWN %d %d", par1, par2);
            longtouch = 0;
            break;
        case EVT_POINTERLONG:
            CRLog::trace("EVT_POINTERLONG");
            longtouch = 1;
            break;
        case EVT_POINTERUP:
            CRLog::trace("EVT_POINTERUP %d %d, long: %d", par1, par2, longtouch);
            wm->postEvent( new CRGUIClickEvent(par1, par2, longtouch) );
            wm->handleAllEvents(false);
            break;

    }
    
    
}

int main( int argc, const char * argv[] )
{
    if ( argc<2 ) {
        printf("usage: cr3 <filename>\n");
        return 1;
    }
//    InitCREngineLog(LOGFILE);
    CRLog::setFileLogger("/mnt/ext1/system/share/crengine/cr3.log", true);
    CRLog::setLogLevel(CRLog::LL_TRACE);
    
    fname = (char *)argv[1];

    InkViewMain(main_handler);

    return 0;
}