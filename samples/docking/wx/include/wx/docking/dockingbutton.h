#ifndef _WX_DOCKING_BUTTON_H_
#define _WX_DOCKING_BUTTON_H_


#if wxUSE_DOCKING

#include <wx/defs.h>
#include <wx/frame.h>
#include <wx/docking/docking_defs.h>

class WXDLLIMPEXP_DOCKING wxDockingButton
: public wxFrame
{
public:
	wxDockingButton(wxWindow *parent, wxBitmap *bitmap);
	~wxDockingButton() override {}

protected:
	wxBitmap *GetBitmap() const { return m_bitmap;  }

	void OnPaint(wxPaintEvent &event);

private:
	wxDECLARE_EVENT_TABLE();

	wxBitmap *m_bitmap;
};

#endif // wxUSE_DOCKING

#endif // _WX_DOCKING_BUTTON_H_
