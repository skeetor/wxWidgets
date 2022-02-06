#if wxUSE_DOCKING

#include <wx/dcclient.h>
#include <wx/bitmap.h>
#include <wx/docking/dockingbutton.h>

wxBEGIN_EVENT_TABLE(wxDockingButton, wxFrame)
	EVT_PAINT(wxDockingButton::OnPaint)
wxEND_EVENT_TABLE()

wxDockingButton::wxDockingButton(wxWindow *parent, wxBitmap *bitmap)
: wxFrame(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT)
, m_bitmap(bitmap)
{
	SetClientSize(bitmap->GetSize());
}

void wxDockingButton::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);

	dc.DrawBitmap(*GetBitmap(), 0, 0, true);
}

#endif // wxUSE_DOCKING
