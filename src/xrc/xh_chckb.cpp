/////////////////////////////////////////////////////////////////////////////
// Name:        xh_chckb.cpp
// Purpose:     XRC resource for wxCheckBox
// Author:      Bob Mitchell
// Created:     2000/03/21
// RCS-ID:      $Id$
// Copyright:   (c) 2000 Bob Mitchell and Verant Interactive
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////
 
// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/xrc/xh_chckb.h"
#include "wx/checkbox.h"

#if wxUSE_CHECKBOX

wxCheckBoxXmlHandler::wxCheckBoxXmlHandler() 
: wxXmlResourceHandler() 
{
    AddWindowStyles();
}

wxObject *wxCheckBoxXmlHandler::DoCreateResource()
{ 
    XRC_MAKE_INSTANCE(control, wxCheckBox)

    control->Create(m_parentAsWindow,
                    GetID(),
                    GetText(wxT("label")),
                    GetPosition(), GetSize(),
                    GetStyle(),
                    wxDefaultValidator,
                    GetName());

    control->SetValue(GetBool( wxT("checked")));
    SetupWindow(control);
    
    return control;
}

bool wxCheckBoxXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("wxCheckBox"));
}

#endif
