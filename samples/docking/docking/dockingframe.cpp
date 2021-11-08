
#if wxUSE_DOCKING

#include <wx/gbsizer.h>

#include <wx/app.h>
#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingframe.h>
#include <wx/docking/dockingpanel.h>

using namespace std;

wxDockingInfo *wxDockingInfo::m_default = nullptr;

namespace
{
	// If a panel is floating, we have to keep track of it
	// so we can properly de-/serialize the layout.
	// this list will contain only floating frames.
	vector<wxDockingFrame *> gFrames;
};

wxIMPLEMENT_DYNAMIC_CLASS(wxDockingFrame, wxFrame);

wxDockingFrame::wxDockingFrame()
{
	init();
}

wxDockingFrame::wxDockingFrame(wxWindow *parent,
	wxWindowID id,
	const wxString &title,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString &name)
: wxFrame()
{
	init();
	Create(parent, id, title, pos, size, style, name);
}

wxDockingFrame::wxDockingFrame(wxWindow *parent,
	wxWindowID id,
	wxDockingPanel *root,
	const wxPoint &pos,
	const wxSize &size,
	long style,
	const wxString &name)
: wxFrame()
{
	init();
	m_rootPanel = root;

	wxString title;
	if (root)
		title = root->GetTitle();

	Create(parent, id, title, pos, size, style, name);
	root->Reparent(this);
	Refresh();
}

wxDockingFrame::~wxDockingFrame()
{
	UnbindEventHandlers();

	// Remove this frame from the list.
	for (vector<wxDockingFrame *>::iterator it = gFrames.begin(); it != gFrames.end(); ++it)
	{
		if (*it == this)
		{
			gFrames.erase(it);
			break;
		}
	}
}

void wxDockingFrame::init(void)
{
	m_rootPanel = nullptr;
	m_activePanel = nullptr;
	m_sizer = nullptr;

	m_toolbarsLeft = nullptr;
	m_toolbarsRight = nullptr;
	m_toolbarsTop = nullptr;
	m_toolbarsBottom = nullptr;

	m_defaults.defaults();
}

bool wxDockingFrame::Create(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
	if (!wxFrame::Create(parent, id, title, pos, size, style, name))
		return false;

	// Register the frame in our list. Should be done first thing after initialzing our frame.
	gFrames.push_back(this);

	m_activePanel = m_rootPanel;

	m_sizer = new wxGridBagSizer(0, 0);
	m_sizer->SetFlexibleDirection(wxBOTH);
	m_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	SetSizer(m_sizer);
	UpdateToolbarLayout();

	BindEventHandlers();

	return true;
}

void wxDockingFrame::BindEventHandlers(void)
{
	// We only bind our event handlers for the main frame.
	if (gFrames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

	app->Bind(wxEVT_LEFT_DOWN, &wxDockingFrame::OnMouseLeftDown, this);
	app->Bind(wxEVT_SIZE, &wxDockingFrame::OnSize, this);
	app->Bind(wxEVT_SIZING, &wxDockingFrame::OnSize, this);
}

void wxDockingFrame::UnbindEventHandlers(void)
{
	if (gFrames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

	app->Unbind(wxEVT_LEFT_DOWN, &wxDockingFrame::OnMouseLeftDown, this);
}

void wxDockingFrame::OnSize(wxSizeEvent &event)
{
	DoSize();
	event.Skip();
}

void wxDockingFrame::DoSize(void)
{
	if (m_rootPanel)
	{
		wxSize sz = GetClientSize();
		m_rootPanel->SetSize(sz);
	}
}

void wxDockingFrame::SetActivePanel(wxDockingPanel *panel)
{
	wxString s = "Active panel: ";
	if (panel)
	{
		s << panel->GetTitle();
		s << " (" << panel->m_type << ")";
	}
	else
		s += "UNKNOWN";

	s << " - " << (void *)panel;
	gFrames[0]->GetStatusBar()->SetStatusText(s);

	if (!panel)
	{
		wxCHECK_MSG(panel, , wxT("Active panel can not be a nullptr"));
	}

	m_activePanel = panel;
}

void wxDockingFrame::OnMouseLeftDown(wxMouseEvent &event)
{
	wxPoint mousePos = ::wxGetMousePosition();

	wxWindow *w = wxFindWindowAtPoint(mousePos);
	wxDockingPanel *p = FindDockingParent(w);
	if (p)
	{
		if (!p->isToolBar())
			SetActivePanel(p);
	}

	event.Skip();
}

wxDockingPanel *wxDockingFrame::FloatPanel(wxWindow *panel, wxDockingInfo const &info)
{
	long style = wxSYSTEM_MENU | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN;

	wxDockingFrame *frame = new wxDockingFrame(this, wxID_ANY, info.title(), info.position(), info.size(), style);
	wxDockingPanel *dp = frame->GetRootPanel();
	dp->SetTitle(info.title());
	panel->Reparent(dp);
	dp->SetFloatingWindow(frame);
	frame->Show();

	return dp;
}

wxDockingPanel *wxDockingFrame::FindDirectParent(wxWindow *window) const
{
	if (!window)
		return nullptr;

	return dynamic_cast<wxDockingPanel *>(window->GetParent());
}

wxDockingPanel *wxDockingFrame::FindDockingParent(wxWindow *window) const
{
	if (!window)
		return nullptr;

	do
	{
		wxDockingPanel *p = dynamic_cast<wxDockingPanel *>(window);
		if (p)
			return p;

		window = window->GetParent();
	} 	while (window);

	return nullptr;
}

/*
wxString wxDockingFrame::SerializeLayout(void) const
{
	// If all frames should be serialized, we start at the root frame.
	if (gFrames[0] != this)
		return gFrames[0]->SerializeLayout();

	wxString layout = SerializeFrame();
	if (layout.empty())
		return layout;

	// TODO: Serialize this frame here.

	return layout;
}

wxString wxDockingFrame::SerializeFrame(void) const
{
	wxString layout;

	return layout;
}

bool wxDockingFrame::DeserializeLayout(wxString layout)
{
	// If all frames should be deserialized, we start at the root frame.
	if (gFrames[0] != this)
		return gFrames[0]->DeserializeLayout(layout);

	return DeserializeFrame(layout);
}

bool wxDockingFrame::DeserializeFrame(wxString layout)
{
	return true;
}
*/

void wxDockingFrame::UpdateToolbarLayout(void)
{
	SetSizer(nullptr, true);

	m_sizer = new wxGridBagSizer(0, 0);
	m_sizer->SetFlexibleDirection(wxBOTH);
	m_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	int top = m_toolbarsTop != nullptr;
	int left = m_toolbarsLeft != nullptr;
	int right = 0;
	int border = 0;

	if (m_toolbarsLeft)
		m_sizer->Add(m_toolbarsLeft, wxGBPosition(top, 0), wxGBSpan(1, 1), wxEXPAND, border);

	if (m_toolbarsRight)
	{
		m_sizer->Add(m_toolbarsRight, wxGBPosition(top, 1+left), wxGBSpan(1, 1), wxEXPAND, border);
		right = 1;
	}

	if (m_toolbarsTop)
		m_sizer->Add(m_toolbarsTop, wxGBPosition(0, 0), wxGBSpan(1, 1+left+right), wxEXPAND, border);

	if (m_toolbarsBottom)
		m_sizer->Add(m_toolbarsBottom, wxGBPosition(1+top, 0), wxGBSpan(1, 1+left+right), wxEXPAND, border);

	if (m_rootPanel)
	{
		wxGBPosition rootPos(top, left);
		m_sizer->Add(m_rootPanel, rootPos, wxGBSpan(1, 1), wxEXPAND, border);

		m_sizer->AddGrowableCol(rootPos.GetCol());
		m_sizer->AddGrowableRow(rootPos.GetRow());

		SetSizer(m_sizer, true);
		Layout();
		m_rootPanel->Update();
	}
}

wxDockingPanel *wxDockingFrame::AddToolBar(wxToolBar *toolbar, wxDockingInfo const &info)
{
	if (!toolbar)
		return nullptr;

	wxDockingPanel *dp = nullptr;

	wxSize tbSize = toolbar->GetSize();

	if (info.isToolbarHorizontal())
	{
		if (info.direction() == wxTOP)
		{
			if (!m_toolbarsTop)
				m_toolbarsTop = new wxDockingPanel(this, info.title());

			dp = m_toolbarsTop;
		}
		else
		{
			if (!m_toolbarsBottom)
				m_toolbarsBottom = new wxDockingPanel(this, info.title());

			dp = m_toolbarsBottom;
		}

		tbSize.SetWidth(-1);
	}
	else
	{
		if (info.direction() == wxLEFT)
		{
			if (!m_toolbarsLeft)
				m_toolbarsLeft = new wxDockingPanel(this, info.title());

			dp = m_toolbarsLeft;
		}
		else
		{
			if (!m_toolbarsRight)
				m_toolbarsRight = new wxDockingPanel(this, info.title());

			dp = m_toolbarsRight;
		}

		tbSize.SetHeight(-1);
	}

	toolbar->Reparent(dp);

	// We need to call Realize again after reparenting, otherwise the
	// size of the toolbar will be wrong in some cases.
	dp->SetToolbar(toolbar);
	dp->SetMinSize(tbSize);
	dp->SetMaxSize(tbSize);
	toolbar->Realize();

	UpdateToolbarLayout();

	return dp;
}

bool wxDockingFrame::HideToolbar(wxDockingPanel *&toolbar)
{
	if (toolbar)
	{
		RemoveChild(toolbar);
		delete toolbar;
		toolbar = nullptr;
		return true;
	}

	return false;
}

bool wxDockingFrame::RemoveToolBar(wxToolBar *toolbar, wxDockingInfo const &info)
{
	bool rc = false;
	if (info.isToolbarHorizontal())
	{
		if (info.direction() == wxTOP)
			rc = HideToolbar(m_toolbarsTop);
		else
			rc = HideToolbar(m_toolbarsBottom);
	}
	else
	{
		if (info.direction() == wxLEFT)
			rc = HideToolbar(m_toolbarsLeft);
		else
			rc = HideToolbar(m_toolbarsRight);
	}

	if (rc)
		UpdateToolbarLayout();

	return rc;
}

wxNotebook *wxDockingFrame::ConvertToNotebook(wxWindow *source, wxWindow *currentWindow, wxDockingInfo const &info)
{
	// Convert the source window we want to attach to, to the new notebook.
	wxNotebook *pnb = dynamic_cast<wxNotebook *>(source);
	if (!pnb)
		return pnb;

	int index = pnb->FindPage(currentWindow);
	// Normally the window should be a page of the notebook in this case but we can't do much here.
	if (index != wxNOT_FOUND)
	{
		wxWindow *page = pnb->GetPage(index);
		wxString title = pnb->GetPageText(index);
		pnb->RemovePage(index);
		wxNotebook *nb = new wxNotebook(pnb, wxID_ANY, wxDefaultPosition, info.size(), info.tabStyle());
		page->Reparent(nb);
		pnb->InsertPage(index, nb, title);
		nb->AddPage(page, title, true);

		return nb;
	}

	return nullptr;
}

wxDockingPanel *wxDockingFrame::AddTabPanel(wxWindow *userWindow, wxDockingInfo const &info, wxDockingPanel **notebook)
{
	wxDockingPanel *dummy = nullptr;
	if (!notebook)
		notebook = &dummy;

	// The panel we want to dock to
	wxDockingPanel *dockingTarget = info.dock();

	if (dockingTarget)
	{
		wxCHECK_MSG(!dockingTarget->isToolBar(), nullptr, wxT("Can not dock to a toolbar"));
	}

	wxDockingPanel *dp = CreateTabPanel(userWindow, info, nullptr);
	*notebook = dp;

	SetActivePanel(dp);

	return dp;
}

wxDockingPanel *wxDockingFrame::CreateTabPanel(wxWindow *userWindow, wxDockingInfo const &info, wxWindow *parent)
{
	wxCHECK_MSG(userWindow, nullptr, wxT("userWindow is a nullptr"));

	wxDockingPanel *ndp = nullptr;
	wxDockingPanel *dockingTarget = info.dock();

	if (!dockingTarget)
	{
		if (!parent)
			parent = userWindow->GetParent();

		dockingTarget = new wxDockingPanel(parent);
		ndp = dockingTarget;
	}
	else
	{
		if (!parent)
			parent = dockingTarget->GetParent();
	}

	wxNotebook *nb = dockingTarget->GetNotebook();
	if (!nb)
	{
		if (!ndp)
			ndp = new wxDockingPanel(parent);

		// Convert the current page we want to attach to, to the new notebook.
		nb = ConvertToNotebook(parent, dockingTarget, info);
		if (!nb)
			nb = new wxNotebook(ndp, wxID_ANY, wxDefaultPosition, info.size(), info.tabStyle());

		ndp->SetNotebook(nb);
	}
	else
		ndp = dockingTarget;

	wxDockingPanel *udp = new wxDockingPanel(nb, info.title());
	udp->SetUserWindow(userWindow);
	userWindow->Reparent(udp);
	nb->AddPage(udp, info.title(), true);

	if (!m_rootPanel)
		m_rootPanel = ndp;

	return ndp;
}

wxDockingPanel *wxDockingFrame::SplitPanel(wxWindow *userWindow, wxDockingInfo const &info)
{
	wxDockingPanel *dockingTarget = info.dock();

	if (!dockingTarget)
		dockingTarget = GetRootPanel();

	// This check can only trigger for the first window, when no other panel has been added yet.
	// The first window should always be a center panel.
	wxCHECK_MSG(dockingTarget, nullptr, wxT("Can not dock using a nullptr as target"));

	wxSize sz = dockingTarget->GetSize();

	wxDockingPanel *dp1 = new wxDockingPanel(dockingTarget->GetParent());
	wxSplitterWindow *splitter = new wxSplitterWindow(dp1, wxID_ANY);
	dp1->SetSplitter(splitter);

	// The splitter window will replace the current window, so it will
	// get the full size of it.
	splitter->SetSashGravity(1.0);
	splitter->SetSize(sz);
	dockingTarget->Reparent(splitter);
	if (dockingTarget == m_rootPanel)
		m_rootPanel = dp1;

	swap(dp1, dockingTarget);

	// Create a docking panel for the new window.
	wxDockingInfo childInfo = info;
	childInfo.dock(dockingTarget);
	childInfo.activate();
	wxDockingPanel *dp2 = CreateTabPanel(userWindow, childInfo, splitter);
	wxDockingPanel *result = dp2;

	wxWindow *p = dp1->GetParent();
	p = dp2->GetParent();

	wxDirection direction = info.direction();
	if (direction == wxLEFT || direction == wxRIGHT)
	{
		if (direction == wxLEFT)
			swap(dp1, dp2);

		uint32_t pos = sz.x / 2;
		splitter->SplitVertically(dp1, dp2, pos);
	}
	else
	{
		if (direction == wxUP)
			swap(dp1, dp2);

		uint32_t pos = sz.y / 2;
		splitter->SplitHorizontally(dp1, dp2, pos);
	}

	SetActivePanel(result);

	return result;
}

bool wxDockingFrame::RemovePanel(wxDockingPanel *panel)
{
	wxCHECK_MSG(panel, false, wxT("panel can not be a nullptr"));

	return true;
}

void wxDockingFrame::Undock(wxWindow *userWindow)
{
	wxCHECK_MSG(userWindow, (void)false, wxT("userWindow can not be a nullptr"));

	wxDockingPanel *dp = FindDockingParent(userWindow);

	// If no docking panel was found, the window is not docked.
	if (!dp)
		return;


}

#endif // wxUSE_DOCKING
