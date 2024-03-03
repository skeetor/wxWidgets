#if wxUSE_DOCKING

#include <wx/docking/buttonoverlay.h>
#include <wx/docking/dockingbutton.h>

#include <wx/docking/bitmaps/dockup.xpm>

static wxBitmap wxDockingUpBMP(wxDockingUp);

wxDockingButtonOverlay::wxDockingButtonOverlay(wxDockingFrame *parent)
: m_parent(parent)
, m_topPanel(std::make_unique<wxDockingButton>((wxWindow *)parent, &wxDockingUpBMP))
{
}

wxDockingButtonOverlay::~wxDockingButtonOverlay()
{
}

void wxDockingButtonOverlay::Refresh()
{
	GetTopPanelButton()->Refresh();
}

void wxDockingButtonOverlay::Show()
{
	m_topPanel->Show();
}

void wxDockingButtonOverlay::Hide()
{
	m_topPanel->Hide();
}

void wxDockingButtonOverlay::ProcessOverlay(wxDockingEvent &event)
{
	// We only show this button on Notebooks,because this is the only one where it is ambigous
	// if the user moves the mouse to the top (on a TOP notebook). It can either mean that the
	// target should be to insert between tabs or the user wants to split above it.
	// If the notebook is empty, then the interaction is also clear.
	if (ButtonHitUpdate(event))
		return;

	wxDockingInfo &tgt = event.GetTarget();
	wxDockingEntity &tw = tgt.GetWindow();
	wxNotebook *nb = tw.GetNotebook();
	if (!nb)
		nb = wxDockingUtils::FindParentPanel(tw).GetNotebook();

	if (nb && nb->GetPageCount())
	{
		Show();
		DoLayout(tw);
		return;
	}

	Hide();
}

bool wxDockingButtonOverlay::IsButtonHit(wxDockingEvent const &event)
{
	return event.GetWindowAtPoint() == GetTopPanelButton();
}

bool wxDockingButtonOverlay::ButtonHitUpdate(wxDockingEvent &event)
{
	// If we didn't hit our own button, then we don't need to look any further
	if (!IsButtonHit(event))
		return false;

	event.SetTarget(event.GetLastKnownTarget());

	// If our button was hit, we set the event to the recorded one, so it will target the appropriate panel.
	wxDockingInfo &tgt = event.GetTarget();
	wxDirection dir = wxALL;

	wxDockingEntity &tw = tgt.GetWindow();
	wxNotebook *nb = tw.GetNotebook();
	if (!nb)
		nb = wxDockingUtils::FindParentPanel(tw).GetNotebook();

	if (nb)
	{
		// We can use the tab orientation here because this is where we want to dock to.
		dir = nb->GetTabOrientation();
		tgt.SetWindow(nb, wxDOCKING_NOTEBOOK);
	}

	tgt.SetDirection(dir);

	return true;
}

void wxDockingButtonOverlay::DoLayout(wxDockingEntity const &target)
{
	wxPoint pos;
	wxRect sr = target->GetScreenRect();

	wxSize sz;
	sz = m_topPanel->GetSize();
	pos.x = sr.x + ((sr.width - sz.GetWidth()) / 2);
	pos.y = sr.y + ((sr.height - sz.GetHeight()) / 2);
	m_topPanel->SetPosition(pos);
}

#endif // wxUSE_DOCKING
