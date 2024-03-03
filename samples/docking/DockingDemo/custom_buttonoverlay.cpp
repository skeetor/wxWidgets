#if wxUSE_DOCKING

#include <wx/bitmap.h>
#include <wx/display.h>

#include <bitmaps/DockBorderUp.xpm>
#include <bitmaps/DockBorderDown.xpm>
#include <bitmaps/DockBorderLeft.xpm>
#include <bitmaps/DockBorderRight.xpm>

#include <bitmaps/DockFloat.xpm>
#include <bitmaps/DockTab.xpm>
#include <bitmaps/DockTabCtrl.xpm>

#include <bitmaps/DockDown.xpm>
#include <bitmaps/DockLeft.xpm>
#include <bitmaps/DockRight.xpm>

#include "custom_buttonoverlay.h"

static wxBitmap wxDockingBorderUpBMP(wxDockingBorderUp);
static wxBitmap wxDockingBorderDownBMP(wxDockingBorderDown);
static wxBitmap wxDockingBorderLeftBMP(wxDockingBorderLeft);
static wxBitmap wxDockingBorderRightBMP(wxDockingBorderRight);

static wxBitmap wxDockingFloatBMP(wxDockingFloat);
static wxBitmap wxDockingTabBMP(wxDockingTab);
static wxBitmap wxDockingTabCtrlBMP(wxDockingTabCtrl);

static wxBitmap wxDockingDownBMP(wxDockingDown);
static wxBitmap wxDockingLeftBMP(wxDockingLeft);
static wxBitmap wxDockingRightBMP(wxDockingRight);

CustomOverlay::CustomOverlay(wxDockingFrame *parent)
	: wxDockingButtonOverlay(parent)
	, m_parent(parent)
{
	Init();
	CalcSize();
}

CustomOverlay::~CustomOverlay()
{
}

#define ADD_BUTTON(vec, parent, bmp) \
	m_buttons.push_back(std::move(std::make_unique<wxDockingButton>(parent, bmp))); \
	vec.push_back(m_buttons.back().get());

void CustomOverlay::Init()
{
	// Needs to be in the order of the enum
	ADD_BUTTON(m_topToBottom, m_parent, &wxDockingBorderUpBMP);			// IDX_FRAME_TOP
	m_topToBottom.push_back(GetTopPanelButton());						// IDX_PANEL_TOP
	ADD_BUTTON(m_topToBottom, m_parent, &wxDockingFloatBMP);			// IDX_FLOATING
	ADD_BUTTON(m_topToBottom, m_parent, &wxDockingTabCtrlBMP);			// IDX_CENTER
	ADD_BUTTON(m_topToBottom, m_parent, &wxDockingTabBMP);				// IDX_TAB
	ADD_BUTTON(m_topToBottom, m_parent, &wxDockingDownBMP);				// IDX_PANEL_BOTTOM
	ADD_BUTTON(m_topToBottom, m_parent, &wxDockingBorderDownBMP);		// IDX_FRAME_BOTTOM

	ADD_BUTTON(m_leftToRight, m_parent, &wxDockingBorderLeftBMP);		// IDX_FRAME_LEFT
	ADD_BUTTON(m_leftToRight, m_parent, &wxDockingLeftBMP);				// IDX_PANEL_LEFT
	ADD_BUTTON(m_leftToRight, m_parent, &wxDockingRightBMP);			// IDX_PANEL_RIGHT
	ADD_BUTTON(m_leftToRight, m_parent, &wxDockingBorderRightBMP);		// IDX_FRAME_RIGHT
}

void CustomOverlay::Refresh()
{
	for (std::unique_ptr<wxDockingButton> &b : m_buttons)
		b->Refresh();

	super::Refresh();
}

void CustomOverlay::Show()
{
	for (std::unique_ptr<wxDockingButton> &b : m_buttons)
		b->Show();

	super::Show();
}

void CustomOverlay::Hide()
{
	for (std::unique_ptr<wxDockingButton> &b : m_buttons)
		b->Hide();

	super::Hide();
}

bool CustomOverlay::IsButtonHit(wxDockingEvent const &event)
{
	wxWindow *w = event.GetWindowAtPoint();

	for (wxDockingButton *b : m_topToBottom)
	{
		if (w == b)
			return true;
	}

	for (wxDockingButton *b : m_leftToRight)
	{
		if (w == b)
			return true;
	}

	return super::IsButtonHit(event);
}

void CustomOverlay::ProcessOverlay(wxDockingEvent &event)
{
	if (IsButtonHit(event))
		event.SetTarget(event.GetLastKnownTarget());

	if (ButtonHitUpdate(event))
		return;

	wxDockingInfo &tgt = event.GetTarget();
	wxDockingEntity tw = tgt.GetDockingEntity();
	DoLayout(tw);
	Show();
}

bool CustomOverlay::ButtonHitUpdate(wxDockingEvent &event)
{
	wxWindow *w = event.GetWindowAtPoint();

	if (w == m_topToBottom[IDX_FRAME_TOP])
	{
		UpdateEvent(event, wxTOP, true);
		return true;
	}

	if (w == m_topToBottom[IDX_PANEL_TOP])
		return super::ButtonHitUpdate(event);

	if (w == m_topToBottom[IDX_FLOATING])
	{
		UpdateEvent(event, wxFLOATING, true);
		return true;
	}

	if (w == m_topToBottom[IDX_CENTER])
	{
		UpdateEvent(event, wxCENTRAL, false);
		return true;
	}

	if (w == m_topToBottom[IDX_TAB])
	{
		// The user wants to attach the window as a tab to an existing
		// notebook. So we have to check if the current window is indeed
		// inside a notebook.
		wxDockingInfo &tgt = event.GetTarget();
		wxDockingEntity &tw = tgt.GetWindow();
		wxDockingEntity const &nb = wxDockingUtils::FindParentPanel(tw);
		if (nb.GetNotebook() != nullptr)
		{
			tgt.SetWindow(nullptr, wxDOCKING_NONE);
			tgt.SetPage(static_cast<size_t>(wxNOT_FOUND));
			tgt.SetTabArea(true);
			UpdateEvent(event, wxCENTRAL, false);
		}
		else
		{
			UpdateEvent(event, wxTOP, false);
			event.SetDockingAllow(false);
		}

		return true;
	}

	if (w == m_topToBottom[IDX_PANEL_BOTTOM])
	{
		UpdateEvent(event, wxBOTTOM, false);
		return true;
	}

	if (w == m_topToBottom[IDX_FRAME_BOTTOM])
	{
		UpdateEvent(event, wxBOTTOM, true);
		return true;
	}

	if (w == m_leftToRight[IDX_FRAME_LEFT])
	{
		UpdateEvent(event, wxLEFT, true);
		return true;
	}

	if (w == m_leftToRight[IDX_PANEL_LEFT])
	{
		UpdateEvent(event, wxLEFT, false);
		return true;
	}

	if (w == m_leftToRight[IDX_PANEL_RIGHT])
	{
		UpdateEvent(event, wxRIGHT, false);
		return true;
	}

	if (w == m_leftToRight[IDX_FRAME_RIGHT])
	{
		UpdateEvent(event, wxRIGHT, true);
		return true;
	}

	return false;
}

void CustomOverlay::UpdateEvent(wxDockingEvent &event, wxDirection direction, bool toFrame)
{
	wxDockingInfo &tgt = event.GetTarget();
	tgt.SetDirection(direction);

	wxDockingEntity p;
	if (!toFrame)
		p = tgt.GetDockingEntity();
	else
		tgt.SetWindow(p);

	tgt.SetPanel(p);
}

void CustomOverlay::CalcSize()
{
	m_buttonRect = wxSize();

	// Total height of our rectangle
	for (wxDockingButton *&b : m_topToBottom)
		m_buttonRect.y += b->GetSize().GetHeight();

	// Total width of our rectangle
	for (wxDockingButton *&b : m_leftToRight)
		m_buttonRect.x += b->GetSize().GetWidth();

	// The middle column contributes to the total width
	m_buttonRect.x += m_topToBottom[0]->GetSize().GetWidth();
}

void CustomOverlay::DoLayout(wxDockingEntity const &target)
{
	// We assume that all buttons have the same height and width.
	wxSize bsz = m_topToBottom[0]->GetSize();
	wxRect sr = target->GetScreenRect();

	wxPoint rect;
	rect.x = ((sr.width - m_buttonRect.GetWidth()) / 2) + sr.x;
	rect.y = ((sr.height - m_buttonRect.GetHeight()) / 2) + sr.y;

	wxDisplay display(wxDisplay::GetFromWindow(target));
	wxRect screen = display.GetClientArea();

	if (rect.x < 0)
		rect.x = 0;

	if (rect.y < 0)
		rect.y = 0;

	if ((rect.x + m_buttonRect.GetWidth()) > screen.width)
		rect.x -= (rect.x + m_buttonRect.GetWidth()) - screen.width;

	if ((rect.y + m_buttonRect.GetHeight()) > screen.height)
		rect.y -= (rect.y + m_buttonRect.GetHeight()) - screen.height;

#define MAKE_COORD(v, a, b) wxPoint(v.x + a, v.y + b)

	int offset = bsz.GetWidth() * 2;
	wxPoint pos = rect;
	for (wxDockingButton *&b : m_topToBottom)
	{
		b->SetPosition(MAKE_COORD(pos, offset, 0));
		pos.y += bsz.GetHeight();
	}

	pos = rect;
	m_leftToRight[IDX_FRAME_LEFT]->SetPosition(MAKE_COORD(pos, 0, offset));
	pos.x += bsz.GetWidth();
	m_leftToRight[IDX_PANEL_LEFT]->SetPosition(MAKE_COORD(pos, 0, offset));

	pos.x += bsz.GetWidth() * 2;
	m_leftToRight[IDX_PANEL_RIGHT]->SetPosition(MAKE_COORD(pos, 0, offset));
	pos.x += bsz.GetWidth();
	m_leftToRight[IDX_FRAME_RIGHT]->SetPosition(MAKE_COORD(pos, 0, offset));
}

#endif // wxUSE_DOCKING
