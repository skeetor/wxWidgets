/////////////////////////////////////////////////////////////////////////////
// Purpose:     XML resources editor
// Author:      Vaclav Slavik
// Created:     2000/05/05
// RCS-ID:      $Id$
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWindows headers
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/xrc/xml.h"
#include "wx/image.h"
#include "wx/wx.h"

#include "editor.h"
#include "preview.h"
#include "propframe.h"


// -- Application

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

IMPLEMENT_APP(MyApp)


bool MyApp::OnInit()
{
    SetVendorName(wxT("wxWindows"));
    SetAppName(wxT("wxrcedit"));
    wxString arg = (argc >= 1) ? argv[1] : "";
    wxInitAllImageHandlers();
    wxFrame *frame = new EditorFrame(NULL, arg);
    SetTopWindow(frame);
    frame->Show(TRUE);
    PreviewFrame::Get()->Show(TRUE);
    PropertiesFrame::Get()->Show(TRUE);
    return TRUE;
}

