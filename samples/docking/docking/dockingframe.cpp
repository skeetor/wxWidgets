
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

	if (!m_rootPanel)
		m_rootPanel = new wxDockingPanel(this);

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
}

void wxDockingFrame::UnbindEventHandlers(void)
{
	if (gFrames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

	app->Unbind(wxEVT_LEFT_DOWN, &wxDockingFrame::OnMouseLeftDown, this);
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

wxDockingPanel *wxDockingFrame::AddTabPanel(wxWindow *panel, wxDockingInfo const &info, wxDockingPanel **notebook)
{
	wxDockingPanel *dummy;
	if (!notebook)
		notebook = &dummy;

	wxNotebook *nb = nullptr;
	wxDockingPanel *dockingPanel = info.dock();
	wxWindow *parent = dockingPanel;

	wxCHECK_MSG(dockingPanel, nullptr, wxT("Docking target is nullptr"));
	wxCHECK_MSG(!dockingPanel->isToolBar(), nullptr, wxT("Can not dock to a toolbar"));

	wxSize sz = dockingPanel->GetClientSize();
	nb = dockingPanel->GetNotebook();
	if (!nb)
	{
		// If the dockingpanel holds a window, then we need to create a notebook
		// so we can attech the new one as well. The current window is added as
		// the first page, before the new panel is added.
		if (info.showTab() || dockingPanel->GetWindow())
		{
			nb = new wxNotebook(dockingPanel, wxID_ANY, wxDefaultPosition, info.size(), info.tabStyle());
			if (dockingPanel->GetWindow())
			{
				wxDockingPanel *dp = new wxDockingPanel(nb, dockingPanel->GetTitle());
				dp->TakeDocking(dockingPanel);
				dp->GetWindow()->Reparent(dp);
				nb->AddPage(dp, dp->GetTitle(), true);
			}

			nb->SetSize(sz);

			dockingPanel->SetNotebook(nb);
			dockingPanel->SetTitle(info.title());
			*notebook = dockingPanel;
			parent = nb;
		}
	}
	else
	{
		parent = nb;
		*notebook = dockingPanel;
	}

	wxDockingPanel *dp = new wxDockingPanel(parent, info.title());

	panel->Reparent(dp);
	dp->SetUserWindow(panel);
	dp->SetTitle(info.title());
	if (nb)
		nb->AddPage(dp, info.title(), true);

	SetActivePanel(dp);

	return dp;
}

wxDockingPanel *wxDockingFrame::SplitPanel(wxWindow *panel, wxDockingInfo const &info)
{
	wxDockingPanel *dockingPanel = info.dock();
	wxCHECK_MSG(dockingPanel, nullptr, wxT("Can not dock using a nullptr as target"));

	wxDockingPanel *dp1 = nullptr;
	wxDockingPanel *dp2 = nullptr;
	wxDockingPanel *result = nullptr;

	wxSplitterWindow *splitter = new wxSplitterWindow(dockingPanel, wxID_ANY);
	splitter->SetSashGravity(1.0);

	wxSize sz = dockingPanel->GetClientSize();
	wxWindow *existingWindow = dockingPanel->GetWindow();

	dp1 = new wxDockingPanel(splitter, dockingPanel->GetTitle());
	dp1->TakeDocking(dockingPanel);
	existingWindow->Reparent(dp1);
	splitter->SetSize(sz);

	// If the tab was converted to a notebook, this will be our docking target
	// otherwise it is just the panel itself.
	wxDockingInfo child(info);
	child.dock(dp1);
	wxDockingPanel *childPanel = AddTabPanel(panel, info, &dp2);
	if (!dp2)
		dp2 = childPanel;
	result = dp2;

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
	dockingPanel->SetSplitter(splitter);

	SetActivePanel(result);

	return result;
}

wxDockingPanel *wxDockingFrame::AddPanel(wxWindow *panel, wxDockingInfo const &info, wxDockingPanel **notebook)
{
	wxCHECK_MSG(panel, nullptr, wxT("panel can not be added using a nullptr"));

	if (notebook)
		*notebook = nullptr;

	wxDockingInfo dinfo = info;
	wxDockingPanel *dockingPanel = dinfo.dock();
	if (!dockingPanel)
	{
		dockingPanel = m_rootPanel;
		dinfo.dock(dockingPanel);
	}

	wxDirection direction = dinfo.direction();
	if (dockingPanel->GetChildren().empty())
		direction = wxCENTRAL;

	if (direction == wxCENTRAL)
		return AddTabPanel(panel, dinfo, notebook);

	return SplitPanel(panel, dinfo);
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

wxDockingPanel *wxDockingFrame::FindTabParent(wxWindow *window) const
{
	if (!window)
		return nullptr;

	do
	{
		wxDockingPanel *p = dynamic_cast<wxDockingPanel *>(window);
		if (p && p->isTabbed())
			return p;

		window = window->GetParent();
	} while (window);

	return nullptr;
}

wxDockingPanel *wxDockingFrame::FindDirectTabParent(wxWindow *window) const
{
	if (!window)
		return nullptr;

	wxDockingPanel *p = FindDockingParent(window);
	if (!p)
		return nullptr;

	if (p->isTabbed())
		return p;

	window = p->GetParent();
	if (!window)
		return nullptr;

	p = dynamic_cast<wxDockingPanel *>(window->GetParent());
	if (!p)
		return nullptr;

	if (p->isTabbed())
		return p;

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
//	clearSizer();

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

	wxGBPosition rootPos(top, left);
	m_sizer->Add(m_rootPanel, rootPos, wxGBSpan(1, 1), wxEXPAND, border);

	m_sizer->AddGrowableCol(rootPos.GetCol());
	m_sizer->AddGrowableRow(rootPos.GetRow());

	SetSizer(m_sizer, true);
	Layout();
	m_rootPanel->Update();
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

#endif // wxUSE_DOCKING
