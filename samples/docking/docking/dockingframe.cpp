
#if wxUSE_DOCKING

#include <wx/gbsizer.h>

#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingframe.h>
#include <wx/docking/dockingpanel.h>

using namespace std;

namespace
{
	// If a panel is floating, we have to keep track of it
	// so we can properly de-/serialize the layout.
	// this list will contain only floating frames.
	vector<wxDockingFrame *> gFrames;
};

wxIMPLEMENT_DYNAMIC_CLASS(wxDockingFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxDockingFrame, wxFrame)
	EVT_CHILD_FOCUS(wxDockingFrame::OnChildFocus)
wxEND_EVENT_TABLE()

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
}

bool wxDockingFrame::Create(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
	if (!wxFrame::Create(parent, id, title, pos, size, style, name))
		return false;

	if (!m_rootPanel)
		m_rootPanel = new wxDockingPanel(this);

	m_activePanel = m_rootPanel;

	// Register the frame in our list.
	gFrames.push_back(this);

	m_sizer = new wxGridBagSizer(0, 0);
	m_sizer->SetFlexibleDirection(wxBOTH);
	m_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	SetSizer(m_sizer);
	UpdateToolbarLayout();

	return true;
}

void wxDockingFrame::OnChildFocus(wxChildFocusEvent &event)
{
	wxWindow *w = event.GetWindow();
	wxDockingPanel *p = FindDockingPanel(w);

	wxString s = "Active panel: ";
	if (p)
		s += p->GetTitle();
	else
		s += "UNKNOWN";

	s << " - " << (void *)p;
	gFrames[0]->GetStatusBar()->SetStatusText(s);

	event.Skip();
}

wxDockingPanel *wxDockingFrame::FloatPanel(wxDockingPanel *panel)
{
	wxDockingPanel *parent = FindDockingPanel(panel->GetParent());
	if (parent)
	{
		// TODO: The panel must be removed if it currently is part of some other wxDockingFrame/Panel
	}

	wxDockingFrame *frame = new wxDockingFrame(this, wxID_ANY, panel);
	wxDockingPanel *dp = frame->GetRootPanel();
	dp->SetFloatingWindow(frame);
	frame->Show();

	return dp;
}

wxDockingPanel *wxDockingFrame::FloatPanel(wxWindow *panel, wxDockingInfo const &info)
{
	wxDockingFrame *frame = new wxDockingFrame(this, wxID_ANY, info.m_title);
	wxDockingPanel *dp = frame->GetRootPanel();
	panel->Reparent(dp);
	dp->SetFloatingWindow(frame);
	frame->Show();
	wxSize sz = frame->GetClientSize();

	return dp;
}

wxDockingPanel *wxDockingFrame::TabbedPanel(wxWindow *panel, wxDockingInfo const &info)
{
	wxDockingPanel *dockingPanel = info.m_dockingPanel;
	if (!dockingPanel)
		dockingPanel = m_rootPanel;

	wxWindow *existingPanel = dockingPanel->GetUserPanel();
	wxSize sz = dockingPanel->GetClientSize();
	if (existingPanel)
	{
		// If the panel already has a panel, then we need to add this one via a notebook.
		wxNotebook *nb = dockingPanel->GetNotebook();
		if (!nb)
		{
			nb = new wxNotebook(dockingPanel, wxID_ANY);
			dockingPanel->SetNotebook(nb);
			existingPanel->Reparent(nb);
			nb->AddPage(existingPanel, dockingPanel->GetTitle(), true);
		}

		panel->Reparent(nb);
		nb->AddPage(panel, info.m_title, true);
	}
	else
	{
		dockingPanel->SetUserPanel(panel);
		dockingPanel->SetTitle(info.m_title);
		panel->Reparent(dockingPanel);
	}

	return dockingPanel;
}

wxDockingPanel *wxDockingFrame::SplitPanel(wxWindow *panel, wxDockingInfo const &info)
{
	wxDockingPanel *dockingPanel = info.m_dockingPanel;
	if (!dockingPanel)
		return nullptr;

	wxSize sz = dockingPanel->GetClientSize();

	wxDockingPanel *dp1 = nullptr;
	wxDockingPanel *dp2 = nullptr;
	wxDockingPanel *result = nullptr;

	wxSplitterWindow *splitter = new wxSplitterWindow(dockingPanel, wxID_ANY);
	splitter->SetSashGravity(1.0);

	wxSplitterWindow *existingSplitter = dockingPanel->GetSplitter();
	wxWindow *existingPanel = dockingPanel->GetUserPanel();
	if (existingPanel)
	{
		dp1 = new wxDockingPanel(splitter, wxID_ANY, info.m_title);
		dp1->SetUserPanel(existingPanel);
		existingPanel->Reparent(dp1);
	}
	else if (existingSplitter)
	{
		dp1 = new wxDockingPanel(splitter, wxID_ANY, info.m_title);
		dp1->SetSplitter(existingSplitter);
		existingSplitter->Reparent(dp1);
	}
	else
	{
		wxNotebook *existingNotebook = dockingPanel->GetNotebook();
		dp1 = new wxDockingPanel(splitter, wxID_ANY, info.m_title);
		dp1->SetNotebook(existingNotebook);
		existingNotebook->Reparent(dp1);
	}

	dp2 = new wxDockingPanel(splitter, wxID_ANY, info.m_title);
	dp2->SetUserPanel(panel);
	panel->Reparent(dp2);
	result = dp2;

	wxDirection direction = info.m_direction;
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

	return result;
}

wxDockingPanel *wxDockingFrame::AddPanel(wxWindow *panel, wxDockingInfo const &info)
{
	if (panel == nullptr)
		return nullptr;

	wxDockingPanel *dockingPanel = info.m_dockingPanel;
	if (!dockingPanel)
		dockingPanel = m_rootPanel;

	wxDirection direction = info.m_direction;
	if(dockingPanel->GetChildren().empty())
		direction = wxCENTRAL;

	if (direction == wxCENTRAL)
		return TabbedPanel(panel, info);

	return SplitPanel(panel, info);
}

wxDockingPanel *wxDockingFrame::FindDockingPanel(wxWindow *window) const
{
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

	if (info.m_horizontal)
	{
		if (info.m_direction == wxTOP)
		{
			if (!m_toolbarsTop)
				m_toolbarsTop = new wxDockingPanel(this, wxID_ANY, info.m_title);

			dp = m_toolbarsTop;
		}
		else
		{
			if (!m_toolbarsBottom)
				m_toolbarsBottom = new wxDockingPanel(this, wxID_ANY, info.m_title);

			dp = m_toolbarsBottom;
		}

		tbSize.SetWidth(-1);
	}
	else
	{
		if (info.m_direction == wxLEFT)
		{
			if (!m_toolbarsLeft)
				m_toolbarsLeft = new wxDockingPanel(this, wxID_ANY, info.m_title);

			dp = m_toolbarsLeft;
		}
		else
		{
			if (!m_toolbarsRight)
				m_toolbarsRight = new wxDockingPanel(this, wxID_ANY, info.m_title);

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

bool wxDockingFrame::RemoveToolBar(wxDockingPanel *toolbar, wxDockingInfo const &info)
{
	bool rc = false;
	if (info.m_horizontal)
	{
		if (info.m_direction == wxTOP)
			rc = HideToolbar(m_toolbarsTop);
		else
			rc = HideToolbar(m_toolbarsBottom);
	}
	else
	{
		if (info.m_direction == wxLEFT)
			rc = HideToolbar(m_toolbarsLeft);
		else
			rc = HideToolbar(m_toolbarsRight);
	}

	if (rc)
		UpdateToolbarLayout();

	return rc;
}

#endif // wxUSE_DOCKING
