
#if wxUSE_DOCKING

#include <wx/app.h>
#include <wx/object.h>
#include <wx/gbsizer.h>
#include <wx/dcclient.h>
#include <wx/vector.h>
#include <wx/dcscreen.h>
#include <wx/dcclient.h>

#include <wx/docking/dockinginfo.h>
#include <wx/docking/dockingframe.h>
#include <wx/docking/dockingevent.h>
#include <wx/docking/buttonoverlay.h>

class wxDockingEventFilter;

static wxDockingEventFilter *gEventFilter;

static wxColor gColorBlue(wxDOCKING_HINT_FLOATING_RED, wxDOCKING_HINT_FLOATING_GREEN, wxDOCKING_HINT_FLOATING_BLUE);
static wxColor gColorGreen(wxDOCKING_HINT_ALLOW_RED, wxDOCKING_HINT_ALLOW_GREEN, wxDOCKING_HINT_ALLOW_BLUE);
static wxBrush gGreenBrush(wxColor(wxDOCKING_HINT_ALLOW_RED, wxDOCKING_HINT_ALLOW_GREEN, wxDOCKING_HINT_ALLOW_BLUE));

static void PrintDebugBar(wxDockingEvent const &event, wxDockingState const &globalState)
{
	wxPoint mousePos = event.GetEventPos();
	wxDockingInfo const &src = event.GetSource();
	wxDockingInfo const &tgt = event.GetTarget();
	double mouseVector = sqrt((double)((double)mousePos.x - (double)globalState.draggingPos.x) * ((double)mousePos.x - (double)globalState.draggingPos.x)
		+ ((double)mousePos.y - (double)globalState.draggingPos.y) * ((double)mousePos.y - (double)globalState.draggingPos.y));
	wxString s;
	s
		<< "Frame: " << wxString().Format(wxS("%p"), (void *)tgt.GetFrame()) << " "
		<< "Window: " << wxString().Format(wxS("%p"), (void *)src.GetWindow()) << " "
		<< "Page: " << ((src.GetPage() == wxNOT_FOUND) ? "N/A" : wxString() << src.GetPage()) << " "
		<< " ===> "
		<< "Frame: " << wxString().Format(wxS("%p"), (void *)tgt.GetFrame()) << " "
		<< "Window: " << wxString().Format(wxS("%p"), (void *)tgt.GetWindow()) << " "
		<< "Page: " << ((tgt.GetPage() == wxNOT_FOUND) ? "N/A" : wxString() << tgt.GetPage()) << " "
		<< "Dir: " << tgt.GetDirection() << " "
		<< "Allow: " << ((event.IsDockingAllowed() ) ? "true" : "false") << " "
		<< "MousePos: " << event.GetEventPos().x << "(" << globalState.draggingPos.x << ")/" << event.GetEventPos().y << "/(" << globalState.draggingPos.y << ") "
		//<< "Distance: " << mouseVector << " "
		//<< "Area: " << wr.x << "/" << wr.y << "/" << wr.width << "/" << wr.height << " "
		//<< "Type: " << w.GetType() << " "
		;
	wxDockingState::GetInstance().frames[0]->SetStatusText(s);
}

// ------------------------------------------------------------------------------------------
wxIMPLEMENT_DYNAMIC_CLASS(wxDockingFrame, wxFrame);

wxBEGIN_EVENT_TABLE(wxDockingFrame, wxFrame)

	EVT_MOUSE_CAPTURE_LOST(wxDockingFrame::OnMouseCaptureLost)

	EVT_DOCKING_START(wxDockingFrame::OnDockingStart)
	EVT_DOCKING_END(wxDockingFrame::OnDockingEnd)

	EVT_DOCKING_CREATE_PANEL(wxDockingFrame::OnCreateDockingPanel)
	EVT_DOCKING_RELEASE_PANEL(wxDockingFrame::OnReleaseDockingPanel)

	EVT_DOCKING_MOVE_PANEL(wxDockingFrame::OnMovePanel)
	EVT_DOCKING_TRACK(wxDockingFrame::OnTrackMove)
	EVT_DOCKING_HINT(wxDockingFrame::OnUpdateHint)

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
	if (!gEventFilter)
		gEventFilter = new wxDockingEventFilter();

	init();
	Create(parent, id, title, pos, size, style, name);
}

wxDockingFrame::~wxDockingFrame()
{
	// We bind our eventhandler only to the first frame and this will handle all docking needs.
	// However, if there are multiple frames, it can happen that the first frame gets destroyed.
	// In this case we need to pass the eventhandler on to another frame to take over.
	wxDockingState &gs = wxDockingState::GetInstance();
	if (gs.frames[0] == this)
	{
		UnbindEventHandlers();
		RemoveFrameEntry();

		if (!gs.frames.empty())
			gs.frames[0]->BindEventHandlers();
	}
	else
		RemoveFrameEntry();

	if (gs.frames.empty())
	{
		delete gEventFilter;
		gEventFilter = nullptr;
	}
}

void wxDockingFrame::init()
{
	m_rootPanel = nullptr;

	m_sizer = nullptr;

	m_toolbarsLeft = nullptr;
	m_toolbarsRight = nullptr;
	m_toolbarsTop = nullptr;
	m_toolbarsBottom = nullptr;

	m_dockingWidth = wxDOCKING_TRIGGER_WIDTH;
	m_dockingTabWidth = wxDOCKING_TAB_WIDTH;
}

void wxDockingFrame::RemoveFrameEntry() const
{
	wxDockingState &gs = wxDockingState::GetInstance();

	for (wxVector<wxDockingFrame *>::iterator it = gs.frames.begin(); it != gs.frames.end(); ++it)
	{
		if (*it == this)
		{
			gs.frames.erase(it);
			break;
		}
	}

	wxDockingEntity p;
	p.SetFrame(const_cast<wxDockingFrame *>(this));
	gs.panels.remove(p);
}

bool wxDockingFrame::Create(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
	if (!wxFrame::Create(parent, id, title, pos, size, style, name))
		return false;

	wxDockingState &gs = wxDockingState::GetInstance();

	// Register the frame in our list. Should be done first thing after initializing our frame.
	// We also register it as a known window, as it is of course also a potential docking target.
	gs.frames.push_back(this);
	gs.panels.append(this);

	m_sizer = new wxGridBagSizer(0, 0);
	m_sizer->SetFlexibleDirection(wxBOTH);
	m_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	SetSizer(m_sizer);
	UpdateToolbarLayout();

	BindEventHandlers();

	return true;
}

void wxDockingFrame::BindEventHandlers()
{
	// We only bind our event handlers for the main frame.
	if (wxDockingState::GetInstance().frames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

	app->AddFilter(gEventFilter);

	app->Bind(wxEVT_SPLITTER_SASH_POS_RESIZE, &wxDockingFrame::OnSplitterSashUpdate, this);
	app->Bind(wxEVT_SPLITTER_DOUBLECLICKED, &wxDockingFrame::OnSplitterDClick, this);

	app->Bind(wxEVT_SIZE, &wxDockingFrame::OnSize, this);
	app->Bind(wxEVT_SIZING, &wxDockingFrame::OnSize, this);
}

void wxDockingFrame::UnbindEventHandlers()
{
	if (wxDockingState::GetInstance().frames[0] != this)
		return;

	wxApp *app = static_cast<wxApp *>(wxApp::GetInstance());

	app->Unbind(wxEVT_SPLITTER_SASH_POS_RESIZE, &wxDockingFrame::OnSplitterSashUpdate, this);
	app->Unbind(wxEVT_SPLITTER_DOUBLECLICKED, &wxDockingFrame::OnSplitterDClick, this);

	app->Unbind(wxEVT_SIZE, &wxDockingFrame::OnSize, this);
	app->Unbind(wxEVT_SIZING, &wxDockingFrame::OnSize, this);

	app->RemoveFilter(gEventFilter);
}

void wxDockingFrame::OnSize(wxSizeEvent &event)
{
	DoSize();
	event.Skip();
}

void wxDockingFrame::DoSize()
{
	wxDockingEntity const &root = GetRootPanel();

	if (root)
	{
		wxSize sz = GetClientSize();
		root->SetSize(sz);
	}
}

/*
wxString wxDockingFrame::SerializeLayout() const
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

wxString wxDockingFrame::SerializeFrame() const
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

void wxDockingFrame::UpdateToolbarLayout()
{
	SetSizer(nullptr, true);

/*	m_sizer = new wxGridBagSizer(0, 0);
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
	}*/
}

wxDockingEntity wxDockingFrame::AddToolBar(wxToolBar *toolbar, wxDockingInfo const &info)
{
	if (!toolbar)
		return nullptr;

	wxDockingEntity dp;

/*	wxSize tbSize = toolbar->GetSize();

	if (info.isToolbarHorizontal())
	{
		if (info.direction() == wxTOP)
		{
			if (!m_toolbarsTop)
				m_toolbarsTop = new wxDockingEntityX(this, wxID_ANY);

			dp = m_toolbarsTop;
		}
		else
		{
			if (!m_toolbarsBottom)
				m_toolbarsBottom = new wxDockingEntityX(this, wxID_ANY);

			dp = m_toolbarsBottom;
		}

		tbSize.SetWidth(-1);
	}
	else
	{
		if (info.direction() == wxLEFT)
		{
			if (!m_toolbarsLeft)
				m_toolbarsLeft = new wxDockingEntityX(this, wxID_ANY);

			dp = m_toolbarsLeft;
		}
		else
		{
			if (!m_toolbarsRight)
				m_toolbarsRight = new wxDockingEntityX(this, wxID_ANY);

			dp = m_toolbarsRight;
		}

		tbSize.SetHeight(-1);
	}

	toolbar->Reparent(dp);

	// We need to call Realize again after reparenting, otherwise the
	// size of the toolbar will be wrong in some cases.
	dp->SetMinSize(tbSize);
	dp->SetMaxSize(tbSize);
	toolbar->Realize();
*/
	UpdateToolbarLayout();

	return dp;
}

/*bool wxDockingFrame::DoHideToolbar(wxDockingEntity const &toolbar)
{
	if (toolbar)
	{
		RemoveChild(toolbar);
		delete toolbar;
		toolbar = nullptr;
		return true;
	}

	return false;
}*/

bool wxDockingFrame::RemoveToolBar(wxToolBar *toolbar, wxDockingInfo const &info)
{
	bool rc = false;
/*	if (info.isToolbarHorizontal())
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
		UpdateToolbarLayout();*/

	return rc;
}

wxDockingEntity wxDockingFrame::AddTabPanel(wxDockingInfo const &info, wxDockingEntity const &userWindow)
{
	wxCHECK_MSG(userWindow, nullptr, wxT("userWindow is a nullptr"));

	wxDockingEntity dockingTarget = info.GetWindow();
	if (dockingTarget.GetType() == wxDOCKING_NOTEBOOK)
	{
		// If the panel is a notebook and the user dragged into the tab, the user wants to move the window
		// as a page into the notebook itself. Otherwise the window is the real target and should be replaced
		// with a new notebook.
		if (!info.IsTabArea() && info.GetWindow())
			dockingTarget = info.GetWindow();
	}
	else
		dockingTarget = info.GetDockingEntity();

	if (dockingTarget == nullptr)
		dockingTarget.Set(this, wxDOCKING_FRAME);

	wxDockingEntity nbp;
	wxNotebook *nb = dockingTarget.GetNotebook();

	if (!nb)
	{
		nbp = CreateTabPanel(info, nullptr);

		// Replace the current dockingTarget with our new notebook.
		ReplaceWindow(dockingTarget, nbp, info.GetTitle());

		// Insert the old dockingTarget as a page into the new notebook we just replaced it with
		AttachTabPage(info, nbp, dockingTarget);
	}
	else
		nbp = dockingTarget;

	AttachTabPage(info, nbp, userWindow);

	return nbp;
}

wxDockingEntity wxDockingFrame::CreateTabPanel(wxDockingInfo const &info, wxDockingEntity const &userWindow, wxDockingEntity parent)
{
	if (!parent)
		parent = this;

	wxDockingInfo inf = CreatePanel(info, parent, wxDOCKING_NOTEBOOK);
	wxDockingEntity &nbp = inf.GetWindow();

	if (userWindow)
		AttachTabPage(info, nbp, userWindow);

	return nbp;
}

bool wxDockingFrame::AttachTabPage(wxDockingInfo const &info, wxDockingEntity const &notebook, wxDockingEntity const &userWindow) const
{
	wxDockingState const &gs = wxDockingState::GetInstance();
	wxNotebook *nb = notebook.GetNotebook();

	wxCHECK_MSG(nb, false, wxT("Notebook may not be nullptr"));
	wxCHECK_MSG(userWindow, false, wxT("UserWindow may not be nullptr"));

	size_t i = info.GetPage();
	if (i == wxNOT_FOUND || i > nb->GetPageCount())
		i = nb->GetPageCount();

	// If the user window is a notebook, we have to move the tabs to the new notebook
	if (userWindow.GetType() == wxDOCKING_NOTEBOOK)
	{
		wxNotebook *nbs = userWindow.GetNotebook();
		while (nbs->GetPageCount())
		{
			wxWindow *w = nbs->GetPage(0);
			nbs->RemovePage(0);

			w->Reparent(nb);
			nb->InsertPage(i, w, gs.PanelState(w).GetTitle(), info.isActive());

			i++;
		}
	}
	else
	{
		userWindow->Reparent(nb);
		nb->InsertPage(i, userWindow, gs.PanelState(userWindow).GetTitle(), info.isActive());
	}

	return true;
}

wxDockingEntity wxDockingFrame::AddSplitPanel(wxDockingInfo const &info, wxDockingEntity userWindow)
{
	wxDockingEntity dockingTarget = info.GetWindow();

	if (dockingTarget == nullptr)
		dockingTarget = info.GetWindow();

	if (dockingTarget == nullptr)
		dockingTarget.Set(this, wxDOCKING_FRAME);

	if (dockingTarget.GetType() == wxDOCKING_FRAME)
		return AddFramePanel(info, userWindow);

	wxDockingEntity p;
	wxDirection direction = info.GetDirection();
	wxDockingEntity parent = dockingTarget->GetParent();

	wxDockingEntity firstWindow;
	wxDockingEntity secondWindow;

	if (direction == wxLEFT || direction == wxUP)
	{
		firstWindow = userWindow;
		secondWindow = dockingTarget;
	}
	else if (direction == wxRIGHT || direction == wxDOWN)
	{
		firstWindow = dockingTarget;
		secondWindow = userWindow;
	}
	else
	{
		if (info.GetWindow().GetType() == wxDOCKING_PLACEHOLDER)
		{
			ReplaceWindow(dockingTarget, userWindow, info.GetTitle(), parent);
			SendReleasePanel(info.GetWindow());
			wxDockingInfo &inf = const_cast<wxDockingInfo &>(info);
			inf.SetWindow(nullptr);

			return info.GetPanel();
		}
		else
		{
			wxCHECK_MSG(false, p, wxT("Invalid direction for splitter"));
		}
	}

	p = CreateSplitPanel(info, true, firstWindow, secondWindow, parent);
	ReplaceWindow(dockingTarget, p, info.GetTitle(), parent);

	return p;
}

wxDockingEntity wxDockingFrame::CreateSplitPanel(wxDockingInfo const &info, bool doSplit, wxDockingEntity firstWindow, wxDockingEntity secondWindow, wxDockingEntity parent)
{
	wxDockingInfo inf = CreatePanel(info, parent, wxDOCKING_SPLITTER);

	wxDockingEntity spp = inf.GetWindow();
	wxSplitterWindow *sp = spp.GetSplitter();
	wxDockingState::GetInstance().panels.append(spp);

	if (firstWindow == nullptr)
	{
		inf.SetWindow(spp);
		SendCreatePanel(inf, wxDOCKING_PLACEHOLDER);
		firstWindow = inf.GetWindow();
	}
	firstWindow->Reparent(sp);
	firstWindow->Show();

	if (secondWindow == nullptr)
	{
		inf.SetWindow(spp);
		SendCreatePanel(inf, wxDOCKING_PLACEHOLDER);
		secondWindow = inf.GetWindow();
	}
	secondWindow->Reparent(sp);
	secondWindow->Show();

	if (doSplit)
		AttachSplitPanel(info, sp, firstWindow, secondWindow);
	else
		sp->Initialize(firstWindow);

	return spp;
}

bool wxDockingFrame::AttachSplitPanel(wxDockingInfo const &info, wxSplitterWindow *splitter, wxDockingEntity firstWindow, wxDockingEntity secondWindow)
{
	wxSize sz = info.GetSize();

	// We initialize the splitter with the size of the parent client area, otherwise the sash position will be flipped around later when the
	// splitter is resized and we loose the information what size was actually requested by the user.
	wxSize spz = splitter->GetParent()->GetClientSize();
	splitter->SetSize(spz);

	wxDirection direction = info.GetDirection();

	int sashPos = -1;
	if (direction == wxLEFT || direction == wxRIGHT)
	{
		sashPos = sz.x;
		if (sashPos == -1)
			sashPos = spz.x / 2;

		if (direction == wxRIGHT)
			sashPos = spz.x - sashPos;

		splitter->SplitVertically(firstWindow, secondWindow, sashPos);
	}
	else
	{
		sashPos = sz.y;
		if (sashPos == -1)
			sashPos = spz.y / 2;

		if (direction == wxDOWN)
			sashPos = spz.y - sashPos;

		splitter->SplitHorizontally(firstWindow, secondWindow, sashPos);
	}
	splitter->SetSashPosition(sashPos);

	return true;
}

wxNotebook *wxDockingFrame::ReplaceNotebookPage(wxNotebook *notebook, wxDockingEntity const &oldPage, int &index, wxDockingInfo const &info)
{
	if (!notebook)
	{
		index = wxNOT_FOUND;
		return nullptr;
	}

	index = notebook->FindPage(oldPage);
	if (index == wxNOT_FOUND)
		return nullptr;

	wxString title = notebook->GetPageText(index);

	wxDockingInfo inf;
	inf.SetWindow(notebook);
	inf.SetTabDirection(info.GetTabDirection());
	SendCreatePanel(inf, wxDOCKING_NOTEBOOK);

	wxNotebook *nb = inf.GetWindow().GetNotebook();
	notebook->RemovePage(index);
	oldPage->Reparent(nb);
	notebook->InsertPage(index, nb, title, true);
	nb->AddPage(oldPage, title, true);

	return nb;
}

wxDockingEntity wxDockingFrame::AddFramePanel(wxDockingInfo const &info, wxDockingEntity const &userWindow)
{
	wxDockingEntity p(this, wxDOCKING_FRAME);

	if (userWindow)
		userWindow->Reparent(this);

	bool createPanel = false;
	bool replacePanel = false;
	wxDockingEntity first;
	wxDockingEntity second;
	wxDockingEntity root = GetRootPanel();

	if (root == nullptr)
	{
		// If the user set force_split but not force_panel then it is treated as if it were set.
		createPanel = info.IsForcePanel();
		if (!createPanel)
		{
			switch (info.GetDirection())
			{
				case wxLEFT:
				case wxUP:
					createPanel = true;
					first = userWindow;
				break;

				case wxRIGHT:
				case wxDOWN:
					createPanel = true;
					second = userWindow;
				break;
			}
		}
	}
	else
	{
		createPanel = true;
		replacePanel = true;

		switch (info.GetDirection())
		{
			case wxLEFT:
			case wxUP:
			{
				first = userWindow;
				second = root;
			}
			break;

			case wxRIGHT:
			case wxDOWN:
			{
				first = root;
				second = userWindow;
			}
			break;
		}
	}

	if (createPanel)
	{
		switch (info.GetDirection())
		{
			case wxCENTRAL:
				p = CreateTabPanel(info, userWindow, p);
			break;

			case wxLEFT:
			case wxDOWN:
			case wxRIGHT:
			case wxUP:
				p = CreateSplitPanel(info, true, first, second, p);
			break;
		}
	}

	if (p.GetType() == wxDOCKING_FRAME)
		p = userWindow;

	if (p.GetType() != wxDOCKING_FRAME)
		SetRootPanel(p);

	if (p)
		p->Show();

	return p;
}

wxDockingEntity wxDockingFrame::AddFloatPanel(wxDockingInfo const &info, wxDockingEntity userWindow)
{
	wxDockingEntity parent(nullptr, wxDOCKING_FRAME);

	wxDockingInfo inf = CreatePanel(info, parent, wxDOCKING_FRAME);
	wxDockingEntity p = inf.GetWindow();
	wxDockingFrame *frame = p.GetFrame();

	userWindow->Reparent(frame);
	inf.Clear();
	inf.SetTitle(wxDockingState::GetInstance().PanelState(userWindow).GetTitle());
	inf.SetWindow(nullptr);
	inf.SetForcePanel(true);

	wxRect r = info.GetRect();
	frame->SetSize(r);
	frame->AddPanel(inf, userWindow);
	frame->Show();
	frame->SendSizeEvent();

	// Now the frame is as big as the desired size of the user window, which means that
	// the size of the user window is much smaller than it should be, due to decorations
	// of the frame (border, titlebar, etc.). Since we don't know how big all this is, we
	// adjust the size of the frame, so that the user window gets the desired size.
	wxRect wr;
	userWindow->GetScreenPosition(&wr.x, &wr.y);
	userWindow->GetSize(&wr.width, &wr.height);

	r.x -= wr.x - r.x;
	r.y -= wr.y - r.y;
	r.width += r.width - wr.width;
	r.height += r.height - wr.height;
	frame->SetSize(r);

	return p;
}

void wxDockingFrame::OnCreateDockingPanel(wxDockingEvent &event)
{
	wxDockingInfo const &src = event.GetSource();
	wxDockingInfo &tgt = event.GetTarget();
	wxDockingEntity const &srcWindow = src.GetWindow();
	wxDockingEntity &window = tgt.GetWindow();

	switch (window.GetType())
	{
		case wxDOCKING_NOTEBOOK:
		{
			wxNotebook *w = new wxNotebook(srcWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, src.GetTabStyle());
			window.SetNotebook(w);
		}
		break;

		case wxDOCKING_SPLITTER:
		{
			wxSplitterWindow *w = new wxSplitterWindow(srcWindow, wxID_ANY, wxDefaultPosition, wxSize(1, 1));
			w->SetSashGravity(0.5f);
			window.SetSplitter(w);
		}
		break;

		case wxDOCKING_PLACEHOLDER:
		{
			wxDockingPlaceholder *w = new wxDockingPlaceholder(srcWindow);
			window.SetPlaceholder(w);
		}
		break;

		case wxDOCKING_FRAME:
		{
			// TODO: If it is a floating frame we want to set the small caption but this causes problems.
			long style = wxDEFAULT_FRAME_STYLE;
			//if (info.GetDirection() == wxFLOATING)
			//	style = wxSYSTEM_MENU | wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX | wxFRAME_TOOL_WINDOW | wxCLIP_CHILDREN;
				//style = wxRESIZE_BORDER | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX | wxTINY_CAPTION | wxCLIP_CHILDREN;

			wxDockingFrame *w = new wxDockingFrame(srcWindow, wxID_ANY, src.GetTitle(), src.GetPosition(), src.GetSize(), style);
			window.SetFrame(w);
		}
		break;

		default:
			wxCHECK_MSG(false, (void)0, wxT("Type not implemented"));
	}
}

void wxDockingFrame::OnReleaseDockingPanel(wxDockingEvent &event)
{
	wxDockingEntity &p = event.GetTarget().GetWindow();

	p.GetRawWindow()->Destroy();
	p.SetWindow(nullptr);
}

wxDockingEntity wxDockingFrame::AddPanel(wxDockingInfo const &info, wxDockingEntity const &panel)
{
	wxDockingEntity p;
	wxDockingEntity const &dockingTarget = info.GetWindow();

	// A toolbar can not be docked to.
	if (dockingTarget.GetType() == wxDOCKING_TOOLBAR)
		return p;

	if (panel)
	{
		wxDockingState &gs = wxDockingState::GetInstance();
		wxDockingEntityState &st = gs.panels.append(panel);
		st.SetTitle(info.GetTitle());
		panel->Show();
	}

	// If the frame is currently empty, we simply add the window to it.
	if (info.GetDirection() != wxFLOATING)
	{
		if (GetRootPanel() == nullptr || dockingTarget.GetType() == wxDOCKING_FRAME)
			return AddFramePanel(info, panel);
	}

	switch (info.GetDirection())
	{
		case wxALL:
		case wxCENTRAL:
		{
			if (dockingTarget.GetType() == wxDOCKING_SPLITTER && info.GetWindow().GetType() == wxDOCKING_PLACEHOLDER)
				p = AddSplitPanel(info, panel);
			else
				p = AddTabPanel(info, panel);
		}
		break;

		case wxLEFT:
		case wxDOWN:
			p = AddSplitPanel(info, panel);
		break;

		case wxRIGHT:
		case wxUP:
			p = AddSplitPanel(info, panel);
		break;

		case wxFLOATING:
			p = AddFloatPanel(info, panel);
		break;
	}

	// TODO: View doesn't properly update without that. :(
	wxSize sz = GetSize();
	sz.x--;
	SetSize(sz);
	sz.x++;
	SetSize(sz);

	return p;
}

void wxDockingFrame::SendMovePanel(wxDockingEvent &event)
{
	event.SetEventType(wxEVT_DOCKING_MOVE_PANEL);
	GetEventHandler()->ProcessEvent(event);
}

void wxDockingFrame::SendCreatePanel(wxDockingInfo &info, wxDockingEntityType type)
{
	wxDockingEvent evt(wxEVT_DOCKING_CREATE_PANEL);
	evt.SetSource(info);
	evt.GetTarget().SetWindow(nullptr, type);

	GetEventHandler()->ProcessEvent(evt);
	info = evt.GetTarget();
}

void wxDockingFrame::SendReleasePanel(wxDockingEntity const &window)
{
	wxDockingEvent evt(wxEVT_DOCKING_RELEASE_PANEL);
	evt.GetTarget().GetWindow() = window;
	GetEventHandler()->QueueEvent(evt.Clone());
}

bool wxDockingFrame::SendTryRemovePanel(wxDockingInfo const &info) const
{
	wxDockingEvent evt(wxEVT_DOCKING_TRY_REMOVE_PANEL);

	evt.GetTarget() = info;
	GetEventHandler()->ProcessEvent(evt);

	return evt.IsAllowed();
}

void wxDockingFrame::OnMovePanel(wxDockingEvent &event)
{
	if (event.IsAllowed() && event.IsDockingAllowed())
		MovePanel(event.GetSource(), event.GetTarget());
}

bool wxDockingFrame::MovePanel(wxDockingInfo const &src, wxDockingInfo const &tgt)
{
	wxDockingEntity sw = src.GetWindow();
	wxCHECK_MSG(src.GetWindow() != nullptr, false, wxT("Source panel missing"));

	// Preserve the title
	wxString title = wxDockingState::GetInstance().PanelState(sw).GetTitle();
	sw = RemovePanel(src, false);
	wxCHECK_MSG(sw != nullptr, false, wxT("Failed to remove source panel!"));

	wxDockingInfo info;
	info = tgt;
	info.SetTitle(title);
	info.SetActivate();

	return AddPanel(info, sw);
}

wxDockingEntity wxDockingFrame::RemovePanel(wxDockingInfo const &src, bool unlinkPages)
{
	wxDockingEntity removed;

	switch (src.GetWindow().GetType())
	{
		case wxDOCKING_FRAME:
			RemoveFromFrame(wxDockingEntity(), wxDockingEntity());
		break;

		case wxDOCKING_SPLITTER:
			RemoveFromSplitter(wxDockingEntity(), wxDockingEntity());
		break;

		case wxDOCKING_NOTEBOOK:
			return RemoveNotebook(src, unlinkPages);
	}

	return wxDockingEntity();
}

wxDockingEntity wxDockingFrame::RemoveNotebook(wxDockingInfo const &src, bool unlinkPages)
{
	wxDockingEntity const &notebook = src.GetWindow();
	wxNotebook *nb = notebook.GetNotebook();
	if (!nb)
		return false;

	int pageIndex = nb->FindPage(page);
	if (pageIndex == wxNOT_FOUND)
		return false;

	wxDockingState const &gs = wxDockingState::GetInstance();

	nb->RemovePage(pageIndex);
	page->Reparent(this);

	if (nb->GetPageCount() <= 1)
	{
		// If the pagecount is == 1, then we try to convert the last page back to a normal window without
		// a tab. In order to do this, we have to check if the tabctrl is persistent or not by asking
		// the user. If the pagecount is already 0 we can remove the tabcontrol if the client allows it.
		wxDockingInfo info;

		info.SetWindow(notebook);
		if (gs.IsLocked(notebook))
		{
			nb->Refresh();
			return true;
		}

		if (!SendTryRemovePanel(info))
		{
			nb->Refresh();
			return true;
		}

		if (notebook->GetPageCount() == 1)
		{
			wxDockingEntity pg = notebook->GetPage(0);
			notebook->RemovePage(0);
			pg->Reparent(this);
			ReplaceWindow(notebookPanel, pg, gs.PanelState(pg).GetTitle());
			SendReleasePanel(notebookPanel);
		}
		else
			RemovePanel(notebook->GetParent(), notebook);
	}
	else
		success = true;

	return success;
}

bool wxDockingFrame::RemoveFromFrame(wxDockingEntity &frame, wxDockingEntity const &userWindow)
{
	wxDockingFrame *f = frame.GetFrame();
	wxCHECK_MSG(f != this, false, wxT("Can not remove the frame from itself. It needs to be called from a different frame."));

	wxDockingEntity root = f->GetRootPanel();
	wxCHECK_MSG(root && userWindow != nullptr && userWindow == root, false, wxT("Invalid window for frame"));

	f->SetRootPanel(nullptr);

	wxDockingInfo info;
	info.SetWindow(frame);

	// Check if the frame can be removed.
	if (SendTryRemovePanel(info))
		SendReleasePanel(frame);

	return true;
}

bool wxDockingFrame::ReplaceWindow(wxDockingEntity const &oldWindow, wxDockingEntity const &newWindow, wxString const &title, wxDockingEntity parent)
{
	if (parent == nullptr)
		parent = oldWindow->GetParent();

	switch (parent.GetType())
	{
		case wxDOCKING_FRAME:
		{
			newWindow->Reparent(this);
			newWindow->Show();
			SetRootPanel(newWindow);
			wxFrame *frame = parent.GetFrame();
			frame->SetTitle(title);
		}
		break;

		case wxDOCKING_SPLITTER:
		{
			wxSplitterWindow *splitter = parent.GetSplitter();

			wxWindow *otherWindow = splitter->GetWindow1();
			if (otherWindow == oldWindow)
				otherWindow = splitter->GetWindow2();

			newWindow->Reparent(splitter);
			newWindow->Show();
			splitter->ReplaceWindow(oldWindow, newWindow);
		}
		break;

		case wxDOCKING_NOTEBOOK:
		{
			wxNotebook *nb = parent.GetNotebook();

			int i = nb->FindPage(oldWindow);
			if (i == wxNOT_FOUND)
				return false;

			newWindow->Reparent(nb);
			nb->RemovePage(i);
			oldWindow->Show();
			newWindow->Show();
			nb->InsertPage(i, newWindow, title, true);
		}
		break;

		// TODO: Toolbar missing

		default:
		{
			wxCHECK_MSG(false, false, wxT("Invalid type"));
			//return false;
		}
	}

	return true;
}

bool wxDockingFrame::RemoveFromSplitter(wxDockingEntity const &splitterPanel, wxDockingEntity const &window)
{
	wxSplitterWindow *splitter = splitterPanel.GetSplitter();
	if (!splitter)
	{
		wxCHECK_MSG(false, false, wxT("Panel is not a splitter"));
	}

	wxDockingState const &gs = wxDockingState::GetInstance();
	wxDockingEntity otherWindow = splitter->GetWindow1();
	if (otherWindow == window)
		otherWindow = splitter->GetWindow2();

	wxDockingInfo info;
	info.SetWindow(splitterPanel);

	wxDockingEntity panel = splitterPanel;

	bool release = false;
	if (!gs.IsLocked(splitterPanel) && SendTryRemovePanel(info))
	{
		// Detach from splitter so the user window wont get deleted when the splitter gets deleted.
		// The caller is responsible for discarding the window if appropriate.
		splitter->Unsplit(window);
		release = true;
	}
	else
	{
		// If the splitter should be kept, we need a placeholder to keep the splitter intact.
		wxDockingInfo inf;

		inf.SetWindow(splitterPanel);
		SendCreatePanel(inf, wxDOCKING_PLACEHOLDER);

		// Now replace the current window with the placeholder.
		otherWindow = inf.GetWindow();
		panel = window;
	}

	if (!ReplaceWindow(panel, otherWindow, gs.PanelState(otherWindow).GetTitle()))
	{
		wxCHECK_MSG(false, nullptr, wxT("Failed to replace window"));
		return false;
	}

	if (release)
		SendReleasePanel(splitterPanel);

	return true;
}

bool wxDockingFrame::MoveToPanel(wxDockingEntity const &newPanel, wxDockingEntity const &panel, wxWindow *window)
{
	window->Reparent(newPanel);

	switch (newPanel.GetType())
	{
		case wxDOCKING_FRAME:
			SetRootPanel(window);
		break;

		case wxDOCKING_NOTEBOOK:
		{
			wxNotebook *parent = newPanel.GetNotebook();
			size_t sel = parent->FindPage(panel);
			wxString title = wxDockingState::GetInstance().PanelState(window).GetTitle();
			parent->RemovePage(sel);
			parent->InsertPage(sel, window, title, false);
		}
		break;

		case wxDOCKING_SPLITTER:
		{
			wxSplitterWindow *sp = newPanel.GetSplitter();
			sp->ReplaceWindow(panel, window);
		}
		break;

		default:
		{
			wxCHECK_MSG(false, false, wxT("Unknown type"));
			//return false;
		}
	}

	SendReleasePanel(panel);

	return true;
}

bool wxDockingFrame::DoMoveSplitter(wxDockingInfo const &src, wxDockingInfo const &tgt)
{
	wxDockingEntity const &tp = tgt.GetPanel();
	if (tp.GetType() != wxDOCKING_SPLITTER)
		return false;

	wxSplitterWindow *sp = tp.GetSplitter();

	wxOrientation orientation = src.GetOrientation();
	wxDirection td = tgt.GetDirection();

	if (
		((td == wxLEFT || td == wxRIGHT) && orientation == wxHORIZONTAL)
		|| ((td == wxTOP || td == wxBOTTOM) && orientation == wxVERTICAL)
		)
	{
		// We have to change the orientation
		if (orientation == wxHORIZONTAL)
			sp->SetSplitMode(wxSPLIT_VERTICAL);
		else
			sp->SetSplitMode(wxSPLIT_HORIZONTAL);

		sp->Refresh();
		sp->Update();
	}

	// Check if the new direction is the same as before. If not, we have to switch the windows
	wxDirection sd = src.GetDirection();
	bool sw1 = (sd == wxLEFT || sd == wxTOP) ? true : false;
	bool sw2 = (td == wxLEFT || td == wxTOP) ? true : false;
	if (sw1 != sw2)
	{
		wxWindow *w1 = sp->GetWindow1();
		wxWindow *w2 = sp->GetWindow2();
		sp->ReplaceWindow(w1, w2);
		sp->ReplaceWindow(w2, w1);
	}

	return true;
}

void wxDockingFrame::OnSplitterSashUpdate(wxSplitterEvent &event)
{
	wxSplitterWindow *splitter = wxDynamicCast(event.GetEventObject(), wxSplitterWindow);

	int oldPos = splitter->GetSashPosition();
	if (oldPos == 0)
	{
		event.Skip();

		return;
	}

	// If the sash should be kept at the same relative position as it was
	// before the resize, we have to calculate the new position based on the ratio.
	int oldSize = event.GetOldSize();
	int newSize = event.GetNewSize();

	float ratio = (float)oldPos / (float)oldSize;
	int pos = newSize * ratio;
	if (pos <= 1)
		pos = oldPos;

	event.SetSashPosition(pos);
}

void wxDockingFrame::OnSplitterDClick(wxSplitterEvent &event)
{
	// Disable the double click on a splitter. The default behavior is
	// to remove the sash, which we don't want.
	event.Veto();
}

bool wxDockingFrame::CheckChildState(wxDockingInfo const &src, wxDockingInfo const &tgt)
{
	// Check if we try to dock a window into a child of itself.
	if (wxDockingUtils::IsParentOf(src.GetDockingEntity().GetRawWindow(), tgt.GetDockingEntity().GetRawWindow()))
		return false;

	return true;
}

bool wxDockingFrame::SendDockingStart()
{
	wxDockingState &gs = wxDockingState::GetInstance();

	gs.event.SetEventType(wxEVT_DOCKING_START);
	gs.event.Allow();

	GetEventHandler()->ProcessEvent(gs.event);

	return gs.event.IsAllowed();
}

void wxDockingFrame::OnDockingStart(wxDockingEvent &event)
{
	if (!DockingStartCondition(event.GetEventPos()))
	{
		event.Veto();
		return;
	}

	BeginTracking(event);
}

void wxDockingFrame::SendDockingEnd()
{
	wxDockingState &gs = wxDockingState::GetInstance();

	gs.event.SetEventType(wxEVT_DOCKING_END);
	gs.event.Allow();
	GetEventHandler()->ProcessEvent(gs.event);
}

void wxDockingFrame::OnDockingEnd(wxDockingEvent &event)
{
	EndTracking(event);
}

void wxDockingFrame::SendTrackMove(wxDockingEvent &event)
{
	event.SetEventType(wxEVT_DOCKING_TRACK);
	GetEventHandler()->ProcessEvent(event);
}

void wxDockingFrame::SendUpdateHint(wxDockingEvent &event)
{
	event.SetEventType(wxEVT_DOCKING_HINT);
	GetEventHandler()->ProcessEvent(event);
}

void wxDockingFrame::InitTarget(wxDockingEvent &event)
{
	wxDockingInfo &tgt = event.GetTarget();
	wxPoint mousePos = event.GetEventPos();

	tgt.FromPoint(mousePos, event.GetFrame(), m_curPanel);

	// We save the current target as a backup, so we can use it in if no appropriate target is found.
	// This can happen i.e. if the mouse is moved outside the window while dragging, which means that the current target
	// should be still valid.
	wxDockingEntity const &tw = tgt.GetDockingEntity();
	if (tw != nullptr && wxDockingState::GetInstance().IsKnownPanel(tw))
	{
		event.SetLastKnownTarget(tgt);
		tgt.SetDirection(wxALL);
	}
	else
		tgt.SetDirection(wxFLOATING);

	if (tgt.GetDockingEntity() == nullptr)
		tgt.SetFrame(this);

	event.SetDockingAllow(true);
}

void wxDockingFrame::OnTrackMove(wxDockingEvent &event)
{
	wxDockingInfo &tgt = event.GetTarget();
	wxDockingInfo &src = event.GetSource();
	wxDockingState &gs = wxDockingState::GetInstance();
	wxIDockingOverlay *overlay = gs.GetOverlayHandler();
	if (overlay)
		overlay->ProcessOverlay(event);

	// The overlay may already have decided what should happen
	wxDirection dir = tgt.GetDirection();
	wxDockingEntity tw = tgt.GetDockingEntity();
	bool allow = event.IsDockingAllowed();

	if (dir == wxALL)
		allow = UpdateDirection(src, tgt, event.GetEventPos());

	// If we are still allowed we need to check if the target is valid. If we are allready not allowed the verdict
	// wont get better and we can skip this check.
	if (allow && (allow = wxDockingUtils::ValidateTarget(src, tgt)))
	{
		// If all checks passed and we are still allowed to dock, we now ask the client if this is a valid target for docking.
		// The client can use this event for its application logic to decide if a particular target is valid.
		wxDockingEvent evt;
		event.SetDockingAllow(allow);
		evt = event;
		evt.SetEventType(wxEVT_DOCKING_TRY_REMOVE_PANEL);
		GetEventHandler()->ProcessEvent(evt);
		allow = evt.IsAllowed();
	}

	event.SetDockingAllow(allow);

	// TODO: Debugging only
	PrintDebugBar(event, wxDockingState::GetInstance());
}

bool wxDockingFrame::UpdateDirection(wxDockingInfo const &src, wxDockingInfo &tgt, wxPoint const &mousePos)
{
	wxDockingEntity const &tw = tgt.GetDockingEntity();

	if (!tw)
	{
		tgt.SetDirection(wxFLOATING);
		return true;
	}

	// Notebooks are handled in the generic validator as they require some extra handling
	tgt.SetDirection(wxCENTRAL);
	if (tw.GetType() != wxDOCKING_NOTEBOOK)
	{
		wxDockingEntity const &window = tgt.GetDockingEntity();
		wxRect cr = window->GetClientRect();
		window->ClientToScreen(&cr.x, &cr.y);

		wxSize border = window->GetWindowBorderSize();
		if (mousePos.y < (cr.y + (int)wxDOCKING_TRIGGER_HEIGHT))				// Cursor is near the top border
		{
			tgt.SetDirection(wxUP);
			return true;
		}

		if (mousePos.y > (cr.y + (cr.height - wxDOCKING_TRIGGER_HEIGHT)))		// Cursor is at the bottom border
		{
			tgt.SetDirection(wxDOWN);
			return true;
		}

		wxPoint clPos = window->ScreenToClient(mousePos);
		if (clPos.x < wxDOCKING_TRIGGER_WIDTH)									// Cursor is at the left border
		{
			tgt.SetDirection(wxLEFT);
			return true;
		}

		if (clPos.x > (cr.width - wxDOCKING_TRIGGER_WIDTH))						// Cursor is at the right border
		{
			tgt.SetDirection(wxRIGHT);
			return true;
		}
	}

	return true;
}

void wxDockingFrame::OnUpdateHint(wxDockingEvent &event)
{
	wxDockingInfo &tgt = event.GetTarget();

	size_t x = tgt.GetPage();
	if (x == 1)
		x = 1;

	bool allow = event.IsDockingAllowed();

	if (tgt.IsTabArea())
		UpdateTabHint(tgt, event.GetEventPos(), allow);
	else
	{
		wxDockingEntity tw = tgt.GetDockingEntity();
		wxDockingEntity sw = event.GetSource().GetDockingEntity();
		if (UpdateWindowHint(sw, tw, event.GetEventPos(), tgt.GetDirection(), allow))
		{

			// Since we should not paint over the overlay, we may have to refresh it.
			// // TODO: Refresh window after painting?
			//if (event.GetOverlayHandler())
			//	event.GetOverlayHandler()->Refresh();
		}
	}
}

void wxDockingFrame::OnMouseCaptureLost(wxMouseCaptureLostEvent &WXUNUSED(event))
{
	wxDockingState &gs = wxDockingState::GetInstance();

	// In case the mouse capture was lost, the docking handling is canceled. We still send the
	// end event, so the user may have to change to do some cleanup in case it relies on it.
	// The target is cleared to indicate that there is nothing to dock to.
	gs.event.GetTarget().Clear();
	gs.event.SetDockingAllow(false);

	SendDockingEnd();

	// If we loose the capture, we want to make sure that the button has been released, so we don't trigger
	// a new docking event if the moose is over one of our windows.
	gs.SetWaitMouseBtnUp(true);
	gs.SetMouseCaptured(false);
	gs.ResetDragging();
}

int wxDockingFrame::OnMouseLeftDown(wxMouseEvent &WXUNUSED(event))
{
	wxDockingState &gs = wxDockingState::GetInstance();

	// Record where the mouse was pressed, so we can determine wether the mouse will be dragged.
	gs.draggingPos = ::wxGetMousePosition();

	// When the mouse button was pressed, we need to check if a draggin starts. This should only happen once per
	// mouse click, otherwise we may falsly report dragging operations.
	gs.EnableDragging();

	return wxEventFilter::Event_Skip;
}

int wxDockingFrame::OnMouseLeftUp(wxMouseEvent &WXUNUSED(event))
{
	wxDockingState &gs = wxDockingState::GetInstance();

	gs.SetIgnoreDocking(false);
	gs.SetWaitMouseBtnUp(false);

	gs.ResetDragging();

	wxPoint mousePos = ::wxGetMousePosition();
	gs.event.SetEventPos(mousePos);

	if (gs.IsMouseCaptured())
		EndDocking(mousePos, true);

	return wxEventFilter::Event_Skip;
}

void wxDockingFrame::EndDocking(wxPoint const &mousePos, bool affirm)
{
	wxDockingState &gs = wxDockingState::GetInstance();

	wxDockingEvent &event = gs.event;
	wxDockingInfo const &src = event.GetSource();
	wxDockingInfo &tgt = event.GetTarget();

	wxDockingEntity w = src.GetDockingEntity();
	tgt.SetTitle(gs.PanelState(w).GetTitle());

	if (tgt.GetDirection() == wxFLOATING)
	{
		wxRect r;
		r.x = mousePos.x;
		r.y = mousePos.y;

		w->GetSize(&r.width, &r.height);
		tgt.SetRect(r);
	}

	if (affirm)
		SendMovePanel(event);

	SendDockingEnd();

	ReleaseMouse();
	gs.SetMouseCaptured(false);
}

int wxDockingFrame::OnMouseMove(wxMouseEvent &event)
{
	wxDockingState &gs = wxDockingState::GetInstance();

	if (gs.WaitMouseBtnUp())
	{
		if (event.ButtonIsDown(wxMOUSE_BTN_LEFT))
		{
			event.Skip();
			return wxEventFilter::Event_Skip;
		}

		gs.SetWaitMouseBtnUp(false);
	}

	wxPoint mousePos = ::wxGetMousePosition();
	if (gs.IgnoreDocking() || !(gs.IsDragging() && event.ButtonIsDown(wxMOUSE_BTN_LEFT)))
	{
		event.Skip();
		return wxEventFilter::Event_Skip;
	}

	gs.event.SetDockingAllow(false);
	if (gs.IsMouseCaptured())
	{
		// For debugging set breakpoint here and enter the desired mouse coordinates. Window must be uncovered
		// before continuing, otherwise FromPoint will not work as it only finds the debugger window.
		gs.event.SetEventPos(mousePos);
		InitTarget(gs.event);
		SendTrackMove(gs.event);
		SendUpdateHint(gs.event);
	}
	else
	{
		wxDockingInfo &src = gs.event.GetSource();

		gs.event.Reset();
		gs.event.SetEventPos(mousePos);
        wxWindow *cw = GetCapture();

		// If the mouse is already captured by some other component, we let it pass through.
		// This happens i.E. when the splitter handle is grabbed for resizing.
		if (cw && cw != this)
		{
			event.Skip();
			gs.SetIgnoreDocking(true);
			return wxEventFilter::Event_Skip;
		}

		wxDockingEntity empty;
		if (!src.FromPoint(mousePos, nullptr, empty))
		{
			event.Skip();
			gs.SetMouseCaptured(false);
			gs.SetIgnoreDocking(true);
			return wxEventFilter::Event_Skip;
		}

		gs.SetMouseCaptured(true);
		CaptureMouse();

		gs.event.SetFrame(src.GetFrame());
		InitTarget(gs.event);

		if (!SendDockingStart())
		{
			ReleaseMouse();
			event.Skip();
			gs.SetMouseCaptured(false);
			gs.SetIgnoreDocking(true);
			return wxEventFilter::Event_Skip;
		}
	}

	//event.Skip();
	return wxEventFilter::Event_Processed;
}

bool wxDockingFrame::DockingStartCondition(wxPoint const &mousePos) const
{
	wxDockingState const &gs = wxDockingState::GetInstance();
	wxDockingInfo const &src = gs.event.GetSource();
	wxDockingEntity const &dockingSource = src.GetDockingEntity();

	if (!gs.IsDraggable(dockingSource))
		return false;

	if (dockingSource && (mousePos.y - dockingSource->GetScreenPosition().y) <= (int)m_dockingWidth)
		return true;

	return false;
}

void wxDockingFrame::BeginTracking(wxDockingEvent &event)
{
	wxDockingState &gs = wxDockingState::GetInstance();

	m_curPanel = wxDockingEntity();
	m_curDirection = wxALL;
	m_curAllow = true;
	m_mousePos = event.GetEventPos();

	if (!gs.HasOverlayHandler())
		gs.SetOverlayHandler(new wxDockingButtonOverlay(this));

	wxWindow *w = event.GetSource().GetWindow();
	wxClientDC dc(w);
	wxRect rect = w->GetClientRect();
	wxDockingUtils::GrabScreenshot(dc, rect, m_sourceWindow);
	m_screenWindow = std::make_unique<wxFrame>(nullptr, wxID_ANY, gs.PanelState(w).GetTitle());

	wxScreenDC::StartDrawingOnTop();
}

void wxDockingFrame::EndTracking(wxDockingEvent &)
{
	wxDockingState &gs = wxDockingState::GetInstance();
	gs.SetOverlayHandler(nullptr, false);

	wxRect r;
	wxDockingEntity p;
	RestoreWindow(p, wxALL, true);
	wxScreenDC::EndDrawingOnTop();
	m_sourceWindow = wxBitmap();
	m_screenWindow.reset();
}

bool wxDockingFrame::RestoreWindow(wxDockingEntity &window, wxDirection dir, bool allow)
{
	bool rc = false;

	if (m_curPanel)
	{
		// Check if anything relevant has changed, in which case we need to clear the current hint.
		if (m_curPanel != window || m_curDirection != dir || m_curAllow != allow)
		{
			m_curPanel->Refresh(true);
			m_curPanel->Update();

			rc = true;
		}
	}
	else
		rc = true;

	m_curPanel = window;
	m_curDirection = dir;
	m_curAllow = allow;

	return rc;
}

void wxDockingFrame::PaintFilterOverlay(wxRect &rectangle, bool allowed, wxDockingEntity &window, wxColor rgb)
{
	wxScreenDC dc;
	wxRect rect = rectangle;
	window->ClientToScreen(&rect.x, &rect.y);

	// TODO: We need to grab the screenshot from the affected window instead of the screen. so that the overlay gets not captured, or overlapping frames.
	wxBitmap windowBmp;
	wxDockingUtils::GrabScreenshot(dc, rect, windowBmp);

	wxImage img(rect.width, rect.height);
	if (!allowed)
		rgb = wxColor(wxDOCKING_HINT_DISALLOW_RED, wxDOCKING_HINT_DISALLOW_GREEN, wxDOCKING_HINT_DISALLOW_BLUE);

	wxDockingUtils::ApplyFilter(rgb, windowBmp, wxDOCKING_HINT_OVERLAY_ALPHA);
	dc.DrawBitmap(windowBmp, rect.x, rect.y);
}

void wxDockingFrame::ShowGhostFrame(wxPoint pos, wxSize size)
{
	m_screenWindow->Move(pos.x + 1, pos.y + 1);
	m_screenWindow->SetSize(size);
	m_screenWindow->Show();
	m_screenWindow->Raise();
}

bool wxDockingFrame::UpdateWindowHint(wxDockingEntity &srcWindow, wxDockingEntity &targetWindow, wxPoint pos, wxDirection direction, bool allow)
{
	wxDockingEntity window;
	wxRect wr;

	// If the target is a frame, we can't paint on it, so we paint on the top window covering the whole area.
	if (targetWindow.GetType() == wxDOCKING_FRAME)
	{
		wxDockingFrame *frame = targetWindow.GetFrame();
		window = frame->GetRootPanel();
	}
	else
		window = targetWindow;

	if (window)
	{
		if (direction != wxFLOATING)
			wr = window->GetClientRect();
	}

	switch (direction)
	{
		case wxUP:
			wr.height = wxDOCKING_TRIGGER_WIDTH;
		break;

		case wxDOWN:
		{
			wr.height -= wxDOCKING_TRIGGER_WIDTH;
			wr.y += wr.height;
			wr.height = wxDOCKING_TRIGGER_WIDTH;
		}
		break;

		case wxLEFT:
			wr.width = wxDOCKING_TRIGGER_WIDTH;
		break;

		case wxRIGHT:
		{
			wr.width -= wxDOCKING_TRIGGER_WIDTH;
			wr.x += wr.width;
			wr.width = wxDOCKING_TRIGGER_WIDTH;
		}
		break;

		case wxCENTRAL:
		{
			wr.x += wxDOCKING_TRIGGER_WIDTH;
			wr.y += wxDOCKING_TRIGGER_HEIGHT;
			wr.width -= (wxDOCKING_TRIGGER_WIDTH * 2);
			wr.height -= (wxDOCKING_TRIGGER_WIDTH * 2);
		}
		break;

		case wxFLOATING:
		{
			bool repaint = RestoreWindow(window, wxFLOATING, allow);
			if (window)
			{
				wr = window->GetClientRect();
				//wr.x += wxDOCKING_TRIGGER_WIDTH;
				//wr.y += wxDOCKING_TRIGGER_HEIGHT;
				//wr.width -= (wxDOCKING_TRIGGER_WIDTH * 2);
				//wr.height -= (wxDOCKING_TRIGGER_WIDTH * 2);

				if (repaint)
					PaintFilterOverlay(wr, allow, window, gColorBlue);
			}
			else
				ShowGhostFrame(pos, srcWindow->GetSize());

			return false;
		}
		break;

		default:
		{
			if (m_curPanel)
			{
				m_curPanel->Refresh(true);
				m_curPanel->Update();
			}
			//wxCHECK_MSG(false, false, wxT("Unknown direction"));
			//return false;
		}
	}

	bool rc = false;
	if (RestoreWindow(window, direction, allow))
	{
		PaintFilterOverlay(wr, allow, window, gColorGreen);
		rc = true;
	}

	if (direction != wxFLOATING)
		m_screenWindow->Hide();

	return rc;
}

void wxDockingFrame::UpdateTabHint(wxDockingInfo &target, wxPoint const &mousePos, bool allow)
{
	wxDockingEntity &notebook = target.GetWindow();
	wxNotebook *nb = notebook.GetNotebook();
	if (!nb)
		return;

	wxRect wr;		// window bar
	size_t pageCnt = nb->GetPageCount();
	size_t page = target.GetPage();

    wxDirection dir = nb->GetTabOrientation();
	if (pageCnt)
	{
		bool outside = false;

		if (page == wxNOT_FOUND)
		{
			outside = true;
			page = pageCnt-1;
			wr = nb->GetTabRect(page);
			switch (dir)
			{
				case wxBOTTOM:
				case wxTOP:
				{
					wr.x += wr.width;
					wr.width = wxDOCKING_TAB_WIDTH;
				}
				break;

				case wxLEFT:
				case wxRIGHT:
				{
					wr.y += wr.height;
					wr.height = wxDOCKING_TAB_HEIGHT;
				}
				break;
			}
		}
		else
		{
			wr = nb->GetTabRect(page);

			switch (dir)
			{
				case wxBOTTOM:
				case wxTOP:
					wr.width = wxDOCKING_TAB_WIDTH;
				break;

				case wxLEFT:
				case wxRIGHT:
					wr.height = wxDOCKING_TAB_HEIGHT;
				break;
			}
		}

		bool onTab = target.IsOnTab();
		if (onTab)
		{
			// If we have a left/right orientation we have to check if the tabs are stacked. In this case, we don't
			// activate the page because, at least on windows, the tabs are reshuffled when activated which causes
			// a very confusing user experience.

			// TODO: We should decide if we want to activate a tab when the mouse is dragged over it.
			// The positive is, that the user can attach to any tab or page within the notebook
			// directly, without any additional clicks. If we don't activate it, it means that the
			// user has to click on the tab before he starts dragging, if he wants to target the page
			// instead of the tab. So maybe this should be changed later, or could be made configurable.
			if (dir == wxLEFT || dir == wxRIGHT)
			{
				bool multiwidth = false;
				wxRect tabArea = wxDockingUtils::GetTabArea(nb, &multiwidth);
				if (multiwidth)
					onTab = false;
			}
		}

		if (onTab)
			nb->SetSelection(page);
	}
	else
	{
		// TODO: Can the pagecount become 0?
        wxCHECK_MSG(false, (void)0, wxT("Not implemented"));
	}

	//if (RestoreWindow(notebook, dir, allow))
		PaintFilterOverlay(wr, allow, notebook, gColorGreen);

	m_mousePos = mousePos;
}

#endif // wxUSE_DOCKING
