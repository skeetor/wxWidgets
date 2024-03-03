// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include <wx/app.h>
#include <wx/clipbrd.h>
#include <wx/menu.h>
#include <wx/toolbar.h>
#include <wx/statusbr.h>
#include <wx/msgdlg.h>
#include <wx/icon.h>
#include <wx/dcclient.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include <wx/dcscreen.h>

#include <wx/docking/docking.h>

#include "../../sample.xpm"
#include "custom_buttonoverlay.h"

#if USE_XPM_BITMAPS
#include "bitmaps/new.xpm"
#include "bitmaps/open.xpm"
#include "bitmaps/save.xpm"
#include "bitmaps/copy.xpm"
#include "bitmaps/cut.xpm"
#include "bitmaps/past.xpm"
#include "bitmaps/help.xpm"
#endif // USE_XPM_BITMAPS

enum
{
	Tool_New,
	Tool_Open,
	Tool_Save,
	Tool_Copy,
	Tool_Cut,
	Tool_Paste,
	Tool_Help,
	Tool_Max
};

// -- application --
static const long ToolbarDefaultStyle = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT | wxTB_NO_TOOLTIPS;
static wxBitmap toolBarBitmaps[Tool_Max];
class MyFrame;

wxString PanelTypeToString(wxDockingEntity const &p)
{
	wxString s;
	switch (p.GetType())
	{
		case wxDOCKING_FRAME:
			s = "Frame";
		break;

		case wxDOCKING_SPLITTER:
			s = "Splitter";
		break;

		case wxDOCKING_NOTEBOOK:
			s = "Notebook";
		break;

		default:
			s = "UNKNOWN";
		break;
	}

	return s;
}

// Since we want to track the active window (the one where the user clicks in),
// we have to install a global mouse filter.
// EVT_LEFT_MOUSE will not work for wxFrame, so we have to use an event handler.
// Another option would be to implement the mouse event in each and every child
// window and propagate it upwards, but that is quite cumbersome.
// https://forums.wxwidgets.org/viewtopic.php?t=39965
// https://forums.wxwidgets.org/viewtopic.php?t=39824
class MouseEventFilter
	: public wxEventFilter
{
public:
	MouseEventFilter(MyFrame *frame)
	: m_frame(frame)
	{
	}

	~MouseEventFilter() wxOVERRIDE
	{
	}

	int FilterEvent(wxEvent &event) wxOVERRIDE;

private:
	MyFrame *m_frame;
};

class MyApp : public wxApp
{
public:
	MyApp()
	: m_eventFilter(nullptr)
	{
	}

	~MyApp() wxOVERRIDE
	{
		RemoveFilter((wxEventFilter *)m_eventFilter);
		delete m_eventFilter;
	}

	bool OnInit() wxOVERRIDE;

	MouseEventFilter *m_eventFilter;
};

wxDECLARE_APP(MyApp);
wxIMPLEMENT_APP(MyApp);

// --------------------------------------------------------------------------------
// -- wxSizeReportCtrl -- (an utility control that always reports it's client size)
class wxSizeReportCtrl : public wxControl
{
public:
	wxSizeReportCtrl()
	: m_text("")
	, m_frame(nullptr)
	, m_isHighlighted(false)
	, m_isDockable(true)
	{
	}

	wxSizeReportCtrl(wxWindow *parent, wxString const &text, wxDockingFrame *frame, wxWindowID id = wxID_ANY,
		const wxPoint &pos = wxDefaultPosition,
		const wxSize &size = wxDefaultSize
		)
	: wxControl(parent, id, pos, size, wxNO_BORDER)
	, m_text(text)
	, m_frame(frame)
	, m_isHighlighted(false)
	, m_isDockable(true)
	{
	}

	~wxSizeReportCtrl() {}

public:
	bool IsHighlighted() const { return m_isHighlighted; }
	void SetHighlighted(bool isHighlighted) { m_isHighlighted = isHighlighted; }

	// Other windows can dock here
	bool IsDockable() const { return m_isDockable; }
	void SetDockable(bool isDockable) { m_isDockable = isDockable; }

private:
	void OnPaint(wxPaintEvent &WXUNUSED(evt))
	{
		wxPaintDC dc(this);
		wxSize size = GetClientSize();
		wxString s;
		int h, w, height, x, y;

		dc.SetFont(*wxNORMAL_FONT);
		dc.SetBrush(*wxWHITE_BRUSH);
		dc.SetPen(*wxWHITE_PEN);
		dc.DrawRectangle(0, 0, size.x, size.y);
		dc.SetPen(*wxLIGHT_GREY_PEN);
		dc.DrawLine(0, 0, size.x, size.y);
		dc.DrawLine(0, size.y, size.x, 0);

		s = GetWindowDescription(dc, this, w, height)+m_text;
		dc.GetTextExtent(s, &w, &height);

		height += FromDIP(3);
		x = (size.x - w) / 2;
		y = ((size.y - (height * 5)) / 2);
		dc.DrawText(s, x, y);
		y += height+2;

		wxWindow *window = GetParent();
		if (window)
		{
			s = GetWindowDescription(dc, window, w, h);
			dc.DrawText(s, x, y);
			y += height + 2;

			window = window->GetParent();
			if (window)
			{
				s = GetWindowDescription(dc, window, w, h);
				dc.DrawText(s, x, y);
				y += height + 2;
			}
		}

		if (IsHighlighted())
		{
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.SetPen(wxPen(wxColor(0, 0, 0), 5)); // 5-pixels-thick black outline
			wxRect r(0, 0, size.GetWidth(), size.GetHeight());
			dc.DrawRectangle(r);
		}
	}

	wxString GetWindowDescription(wxPaintDC &dc, wxWindow *window, int &w, int &h)
	{
		wxPoint pos = window->GetPosition();
		wxSize sz = window->GetSize();
		wxString s;

		const char *type = "";
		wxString posStr;
		wxDockingEntity p(window);
		if (p.GetSplitter())
		{
			wxSplitterWindow *sp = p.GetSplitter();
			type = "Splitter";
			posStr.Printf("Sash: %d %s %p/%p", sp->GetSashPosition(), (sp->GetSplitMode() == wxSPLIT_HORIZONTAL) ? "HORZ" : "VERT", sp->GetWindow1(), sp->GetWindow2());
		}
		else if (p.GetNotebook())
			type = "Notebook";
		else if (p.GetFrame())
			type = "Frame";

		s.Printf("%p Pos: %d/%d Size: %d x %d %s ", (void *)window, pos.x, pos.y, sz.x, sz.y, type);
		s += posStr;
		dc.GetTextExtent(s, &w, &h);

		return s;
	}

	void OnEraseBackground(wxEraseEvent &WXUNUSED(evt))
	{
		// intentionally empty
	}

	void OnSize(wxSizeEvent &WXUNUSED(evt))
	{
		Refresh();
	}

private:
	wxString m_text;
	wxDockingFrame *m_frame;
	bool m_isHighlighted:1;			// True if the window is currently highlighted as user feedback
	bool m_isDockable:1;			// True if the window can be a docking target

	wxDECLARE_EVENT_TABLE();

protected:
	// We need this here, so that the demo application can do dynamic casts. This shold not be neccessary for
	// regular client classes, unless they also want to use dynamic casts.
	wxDECLARE_DYNAMIC_CLASS(wxSizeReportCtrl);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxSizeReportCtrl, wxControl);

wxBEGIN_EVENT_TABLE(wxSizeReportCtrl, wxControl)
	EVT_PAINT(wxSizeReportCtrl::OnPaint)
	EVT_SIZE(wxSizeReportCtrl::OnSize)
	EVT_ERASE_BACKGROUND(wxSizeReportCtrl::OnEraseBackground)
wxEND_EVENT_TABLE()

enum
{
	ID_Exit = wxID_EXIT,
	ID_About = wxID_ABOUT,

	ID_CreateTree = wxID_HIGHEST + 1,

	ID_LayoutSerialize,
	ID_LayoutDeserialize,
	ID_LayoutCopySerialize,
	ID_LayoutSplitCentral,
	ID_LayoutSplitLeft,
	ID_LayoutSplitRight,
	ID_LayoutSplitTop,
	ID_LayoutSplitBottom,
	ID_LayoutSplitLeftBorder,
	ID_LayoutSplitRightBorder,
	ID_LayoutSplitTopBorder,
	ID_LayoutSplitBottomBorder,
	ID_LayoutRemoveDockedPanel,

	ID_OverlayDisable,
	ID_OverlayEnable,
	ID_OverlayCustom,
	ID_OverlayCustomHybrid,

	ID_ToggleContainerLock,
	ID_ToggleDockingPanel,
	ID_ToggleDragPanel,
	ID_ToggleDocking,

	ID_ToolbarLeftAdd,
	ID_ToolbarRightAdd,
	ID_ToolbarTopAdd,
	ID_ToolbarBottomAdd,

	ID_ToolbarLeftRemove,
	ID_ToolbarRightRemove,
	ID_ToolbarTopRemove,
	ID_ToolbarBottomRemove,

};

// --------------------------------------------------------------------------------
// Main demo class
class MyFrame : public wxDockingFrame
{
public:
	MyFrame(wxWindow *parent,
			wxWindowID id,
			const wxString &title,
			const wxPoint &pos = wxDefaultPosition,
			const wxSize &size = wxDefaultSize,
			long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);
	~MyFrame() wxOVERRIDE
	{
	}

public:
	void OnExit(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);

	void OnNewPanel(wxCommandEvent &evt);
	void OnNewPanelBorder(wxCommandEvent &evt);
	void OnLayoutSerialize(wxCommandEvent &evt);
	void OnLayoutDeserialize(wxCommandEvent &evt);
	void OnLayoutCopySerialize(wxCommandEvent &evt);
	void OnRemoveDockingPanel(wxCommandEvent &evt);
	void OnToggleContainerLock(wxCommandEvent &evt);
	void OnToggleDockingPanel(wxCommandEvent &evt);
	void OnToggleDragPanel(wxCommandEvent &evt);
	void OnToggleDocking(wxCommandEvent &evt);

	void OnOverlayDisable(wxCommandEvent &evt);
	void OnOverlayEnable(wxCommandEvent &evt);
	void OnOverlayCustom(wxCommandEvent &evt);
	void OnOverlayCustomHybrid(wxCommandEvent &evt);

	void OnMouseLeftDown(wxMouseEvent &event);
	void OnMouseRightUp(wxMouseEvent &event);
	void OnWindowClose(wxWindow *window);

	void OnToolbar(wxCommandEvent &evt);
	wxToolBar *CreateDockingToolbar(bool top, bool left, wxDockingInfo &info);

	wxSizeReportCtrl *createSizeReportCtrl(wxString const &text);

	void createInitialLayout();
	wxSizeReportCtrl *GetActivePanel() const { return m_activePanel; }

	bool IsActiveWindow(wxWindow *window)
	{
		return  (window != nullptr && m_activePanel == window);
	}

	void HighlightWindow(wxSizeReportCtrl *panel, bool highlight);

private:
	void EnableOverlayMenus();

protected:
	wxMenu *createDockingMenu();
	wxMenu *createToolbarMenu();
	wxToolBar *horizontalToolBar(bool top);
	wxToolBar *verticalToolBar(bool left);

	void SetActivePanel(wxSizeReportCtrl *panel);
	void UpdateStatusText(wxDockingEntity const &container, wxDockingEntity const &panel);

	void OnDockingStart(wxDockingEvent &event);
	void OnReleaseDockingPanel(wxDockingEvent &event);
	void OnTryRemoveDockingPanel(wxDockingEvent &event);

	void SetActiveMenus(bool active);

private:
	uint32_t m_newPanel;
	wxSizeReportCtrl *m_activePanel;
	wxDockingEntity m_activeContainer;
	wxMenuItem *m_borderMenu;
	wxDockingUtils::wxUniqueVector<wxDockingEntity> m_dockablePanels;	// Panels in this list can not be docked to.
	int m_overlayMode;													// Enabled, disabled or custom
	bool m_allowDocking;												// Docking globaly en-/disabled

	wxDECLARE_EVENT_TABLE();
};

int MouseEventFilter::FilterEvent(wxEvent &event)
{
	const wxEventType t = event.GetEventType();
	if (t == wxEVT_LEFT_DOWN)
	{
		wxMouseEvent *mev = static_cast<wxMouseEvent *>(&event);
		m_frame->OnMouseLeftDown(*mev);
		return Event_Skip;
	}

	if (t == wxEVT_RIGHT_UP)
	{
		wxMouseEvent *mev = static_cast<wxMouseEvent *>(&event);
		m_frame->OnMouseRightUp(*mev);
		return Event_Skip;
	}

	if (t == wxEVT_CLOSE_WINDOW)
	{
		wxWindow *w = wxDynamicCast(event.GetEventObject(), wxWindow);
		m_frame->OnWindowClose(w);
		return Event_Skip;
	}

	return Event_Skip;	// Continue regular processing
}

bool MyApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	MyFrame *frame = new MyFrame(nullptr,
								 wxID_ANY,
								 "wxDocking Sample Application",
								 wxDefaultPosition,
								 wxWindow::FromDIP(wxSize(1200, 600), nullptr));
	frame->Show();
	frame->createInitialLayout();

	MyApp *app = static_cast<MyApp *>(wxApp::GetInstance());

	app->m_eventFilter = new MouseEventFilter(frame);
	app->AddFilter(app->m_eventFilter);

	return true;
}

wxBEGIN_EVENT_TABLE(MyFrame, wxDockingFrame)
	EVT_MENU(ID_Exit, MyFrame::OnExit)
	EVT_MENU(ID_About, MyFrame::OnAbout)

	EVT_MENU(ID_LayoutSerialize, MyFrame::OnLayoutSerialize)
	EVT_MENU(ID_LayoutDeserialize, MyFrame::OnLayoutDeserialize)
	EVT_MENU(ID_LayoutCopySerialize, MyFrame::OnLayoutCopySerialize)
	EVT_MENU(ID_LayoutSplitCentral, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitLeft, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitRight, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitTop, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitBottom, MyFrame::OnNewPanel)
	EVT_MENU(ID_LayoutSplitLeftBorder, MyFrame::OnNewPanelBorder)
	EVT_MENU(ID_LayoutSplitRightBorder, MyFrame::OnNewPanelBorder)
	EVT_MENU(ID_LayoutSplitTopBorder, MyFrame::OnNewPanelBorder)
	EVT_MENU(ID_LayoutSplitBottomBorder, MyFrame::OnNewPanelBorder)
	EVT_MENU(ID_LayoutRemoveDockedPanel, MyFrame::OnRemoveDockingPanel)
	EVT_MENU(ID_ToggleContainerLock, MyFrame::OnToggleContainerLock)
	EVT_MENU(ID_ToggleDockingPanel, MyFrame::OnToggleDockingPanel)
	EVT_MENU(ID_ToggleDragPanel, MyFrame::OnToggleDragPanel)
	EVT_MENU(ID_ToggleDocking, MyFrame::OnToggleDocking)

	EVT_MENU(ID_OverlayDisable, MyFrame::OnOverlayDisable)
	EVT_MENU(ID_OverlayEnable, MyFrame::OnOverlayEnable)
	EVT_MENU(ID_OverlayCustom, MyFrame::OnOverlayCustom)
	EVT_MENU(ID_OverlayCustomHybrid, MyFrame::OnOverlayCustomHybrid)

	EVT_MENU(ID_ToolbarLeftAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarRightAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarTopAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarBottomAdd, MyFrame::OnToolbar)

	EVT_MENU(ID_ToolbarLeftRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarRightRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarTopRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarBottomRemove, MyFrame::OnToolbar)

	EVT_DOCKING_START(MyFrame::OnDockingStart)
	EVT_DOCKING_RELEASE_PANEL(MyFrame::OnReleaseDockingPanel)
	EVT_DOCKING_TRY_REMOVE_PANEL(MyFrame::OnTryRemoveDockingPanel)

wxEND_EVENT_TABLE()

MyFrame::MyFrame(wxWindow *parent,
				 wxWindowID id,
				 const wxString &title,
				 const wxPoint &pos,
				 const wxSize &size,
				 long style)
: wxDockingFrame(parent, id, title, pos, size, style)
, m_newPanel(0)
, m_activePanel(nullptr)
, m_borderMenu(nullptr)
, m_overlayMode(ID_OverlayEnable)
, m_allowDocking(true)
{
	// set frame icon
	SetIcon(wxIcon(sample_xpm));

	// create menu
	wxMenuBar *mb = new wxMenuBar;

	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_EXIT);

	wxMenu *helpMenu = new wxMenu;
	helpMenu->Append(wxID_ABOUT);

	mb->Append(fileMenu, _("&File"));
	mb->Append(createDockingMenu(), _("&Docking"));
	mb->Append(createToolbarMenu(), _("&Toolbar"));
	mb->Append(helpMenu, _("&Help"));

	SetMenuBar(mb);
	SetActiveMenus(false);

	CreateStatusBar();
	GetStatusBar()->SetStatusText(_("Ready"));

	SetMinSize(wxSize(200, 150));

	#if USE_XPM_BITMAPS
	#define INIT_TOOL_BMP(bmp) \
			toolBarBitmaps[Tool_##bmp] = wxBitmap(bmp##_xpm)
	#else // !USE_XPM_BITMAPS
	#define INIT_TOOL_BMP(bmp) \
			toolBarBitmaps[Tool_##bmp] = wxBITMAP(bmp)
	#endif // USE_XPM_BITMAPS/!USE_XPM_BITMAPS

	INIT_TOOL_BMP(New);
	INIT_TOOL_BMP(Open);
	INIT_TOOL_BMP(Save);
	INIT_TOOL_BMP(Copy);
	INIT_TOOL_BMP(Cut);
	INIT_TOOL_BMP(Paste);
	INIT_TOOL_BMP(Help);

	//createInitialLayout();
	Refresh();
}

wxMenu *MyFrame::createDockingMenu()
{
	wxMenu *menu = new wxMenu;
	wxMenu *submenu;

	submenu = new wxMenu;
	submenu->Append(ID_LayoutSplitLeftBorder, wxT("Left"), wxEmptyString, wxITEM_NORMAL);
	submenu->Append(ID_LayoutSplitRightBorder, wxT("Right"), wxEmptyString, wxITEM_NORMAL);
	submenu->Append(ID_LayoutSplitTopBorder, wxT("Top"), wxEmptyString, wxITEM_NORMAL);
	submenu->Append(ID_LayoutSplitBottomBorder, wxT("Bottom"), wxEmptyString, wxITEM_NORMAL);
	menu->Append((m_borderMenu = new wxMenuItem(menu, wxID_ANY, wxT("Split to border"), wxEmptyString, wxITEM_NORMAL, submenu)));

	menu->AppendSeparator();

	submenu = new wxMenu;
	submenu->Append(ID_OverlayDisable, wxT("Disable overlays"), wxEmptyString, wxITEM_NORMAL);
	submenu->Append(ID_OverlayEnable, wxT("Default overlay"), wxEmptyString, wxITEM_NORMAL);
	submenu->Append(ID_OverlayCustom, wxT("Custom overlay"), wxEmptyString, wxITEM_NORMAL);
	submenu->Append(ID_OverlayCustomHybrid, wxT("Hybrid custom overlay"), wxEmptyString, wxITEM_NORMAL);
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("Overlay"), wxEmptyString, wxITEM_NORMAL, submenu));
	menu->Enable(ID_OverlayEnable, false);

	menu->Append(ID_LayoutRemoveDockedPanel, wxT("Remove panel"), wxT("The active panel is removed"), wxITEM_NORMAL);
	menu->AppendCheckItem(ID_ToggleDocking, wxT("Enable docking"), wxT("Docking is globally enabled/disabled"))->Check(true);

	menu->AppendSeparator();
	menu->Append(ID_LayoutCopySerialize, wxT("Copy layout to Clipboard"), wxEmptyString, wxITEM_NORMAL);
	menu->Append(ID_LayoutSerialize, wxT("Save layout"), wxEmptyString, wxITEM_NORMAL);
	menu->Append(ID_LayoutDeserialize, wxT("Restore layout"), wxEmptyString, wxITEM_NORMAL);

	return menu;
}

wxMenu *MyFrame::createToolbarMenu()
{
	wxMenu *menu = new wxMenu;

	wxMenu *submenu = new wxMenu;
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarLeftAdd, wxString(wxT("Left")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarRightAdd, wxString(wxT("Right")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarTopAdd, wxString(wxT("Top")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarBottomAdd, wxString(wxT("Bottom")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("Add"), wxEmptyString, wxITEM_NORMAL, submenu));

	submenu = new wxMenu;
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarLeftRemove, wxString(wxT("Left")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarRightRemove, wxString(wxT("Right")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarTopRemove, wxString(wxT("Top")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_ToolbarBottomRemove, wxString(wxT("Bottom")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("Remove"), wxEmptyString, wxITEM_NORMAL, submenu));

	return menu;
}

void MyFrame::SetActiveMenus(bool active)
{
	wxMenuBar *mainBar = GetMenuBar();

	mainBar->Enable(ID_LayoutRemoveDockedPanel, active);
}

void MyFrame::HighlightWindow(wxSizeReportCtrl *panel, bool highlight)
{
	if (!panel)
		return;

	panel->SetHighlighted(highlight);
	panel->Refresh();
}

void MyFrame::UpdateStatusText(wxDockingEntity const &container, wxDockingEntity const &panel)
{
	wxDockingState const &gs = wxDockingState::GetInstance();
	wxString s;
	s
		<< "Activate "
		<< "Panel: " << wxString().Format(wxS("%p"), (void *)panel) << " "
		;

	wxDockingUtils::wxUniqueVector<wxDockingEntity>::iterator pos = std::find(m_dockablePanels.begin(), m_dockablePanels.end(), panel);
	if (pos == m_dockablePanels.end())
		s << "not ";

	s << "dockable ";

	if (gs.IsDraggable(panel))
		s << "not ";
	s << "dragable ";

	s << "Container " << PanelTypeToString(container) << wxString().Format(wxS(": %p"), (void *)container.GetRawWindow()) << " ";

	if (gs.IsLocked(panel))
		s << "locked";
	else
		s << "unlocked";

	SetStatusText(s);
}

void MyFrame::SetActivePanel(wxSizeReportCtrl *panel)
{
	bool highlight = true;
	if (panel && m_activePanel == panel)
		highlight = !panel->IsHighlighted();

	if (!panel)
	{
		if (m_activePanel)
			HighlightWindow(m_activePanel, false);

		m_activePanel = panel;

		return;
	}

	if (m_activePanel)
		HighlightWindow(m_activePanel, false);

	if (highlight)
	{
		m_activePanel = panel;
		m_activeContainer = panel->GetParent();
	}
	else
	{
		m_activePanel = nullptr;
		m_activeContainer = nullptr;
	}

	HighlightWindow(panel, highlight);
	SetActiveMenus(highlight);
}

void MyFrame::OnDockingStart(wxDockingEvent &event)
{
	// Docking can be disabled, so that nothing happens when the user starts dragging
	// the mouse. In this example we disable it globally, but of course an application
	// will probably disable this only under certain conditions. i.E. when the application
	// has it's own dragging code for some part of the gui or simply doesn't want a certain
	// window to be moved.
	if (!m_allowDocking)
	{
		event.Veto();
		return;
	}

	wxDockingState &gs = wxDockingState::GetInstance();

	if (m_overlayMode == ID_OverlayDisable)
		gs.SetOverlayHandler(nullptr);
	else if (m_overlayMode == ID_OverlayCustom)
		gs.SetOverlayHandler(new CustomOverlay(this));

	// For the default overlay we dont need to do anything because it is, well ... the default

	event.Skip();		// Proceed with default event.
}

// We need to make sure that we keep track of deleted objects. Docking panels like splitter
// and notebooks (or frames) can be deleted when no longer needed, so we need to discard such pointers
// in that case, to avoid using deleted memory.
void MyFrame::OnReleaseDockingPanel(wxDockingEvent &event)
{
	wxSizeReportCtrl *p = wxDynamicCast(event.GetTarget().GetDockingEntity().GetWindow(), wxSizeReportCtrl);

	event.Skip();		// If the window can be removed we let it pass on
}

void MyFrame::OnTryRemoveDockingPanel(wxDockingEvent &event)
{
	wxSizeReportCtrl *p = wxDynamicCast(event.GetTarget().GetDockingEntity().GetWindow(), wxSizeReportCtrl);

	// TODO: Implement
	//wxCHECK_MSG(false, (void)0, wxT("TryRemove handler not implmeneted"));

	//if (p && m_panels.HasItem(p))
	//	event.Veto();		// The panel should be persistent
	//else
	//{
	//	event.Skip();		// Allow the panel to be removed
	//}
}

wxSizeReportCtrl *MyFrame::createSizeReportCtrl(wxString const &text)
{
	return new wxSizeReportCtrl(this, text, this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
}

wxToolBar *MyFrame::verticalToolBar(bool left)
{
	long style = wxTB_VERTICAL| ToolbarDefaultStyle;

	// Even if the toolbar is at the bottom or right, we may not
	// specify this here. It must always be set to TOP/LEFT.
	if (left)
		style |= wxTB_LEFT;
	//else
	//	style |= wxTB_RIGHT;

	wxSize sz = GetClientSize();
	sz.SetWidth(20);

	wxToolBar *tb = new wxToolBar(this, wxID_ANY, wxPoint(0, 0), wxDefaultSize, style);
	tb->AddRadioTool(wxID_ANY, "First", wxBITMAP(New));
	tb->AddRadioTool(wxID_ANY, "Second", wxBITMAP(Open));
	tb->AddRadioTool(wxID_ANY, "Third", wxBITMAP(Save));
	tb->AddSeparator();
	tb->AddTool(wxID_HELP, "Help", wxBITMAP(Help));
	tb->AddTool(wxID_ANY, "Cut", wxBITMAP(Cut));
	tb->AddTool(wxID_ANY, "Paste", wxBITMAP(Paste));

	tb->Realize();

	return tb;
}

wxToolBar *MyFrame::horizontalToolBar(bool top)
{
	long style = wxTB_HORIZONTAL | ToolbarDefaultStyle;

	// Even if the toolbar is at the bottom or right, we may not
	// specify this here. It must always be set to TOP/LEFT.
	if (top)
		style |= wxTB_TOP;
	//else
	//	style |= wxTB_BOTTOM;

	wxToolBar *tb = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);

	tb->AddTool(wxID_ANY, "Copy", wxBITMAP(Copy));
	tb->AddTool(wxID_ANY, "Paste", wxBITMAP(Paste));
	tb->AddSeparator();

	wxStaticText *st;
	st = new wxStaticText(tb, wxID_ANY, wxT("Columns:"), wxDefaultPosition, wxDefaultSize, 0);
	st->Wrap(-1);
	tb->AddControl(st);

	wxComboBox *box = new wxComboBox(tb, wxID_ANY, wxT("Auto"));
	box->Append(wxT("Auto"));
	box->Append(wxT("8"));
	box->Append(wxT("16"));
	box->SetSelection(0);
	tb->AddControl(box);


	st = new wxStaticText(tb, wxID_ANY, wxT("Size"), wxDefaultPosition, wxDefaultSize, 0);
	st->Wrap(-1);
	tb->AddControl(st);
	wxTextCtrl *txt = new wxTextCtrl(tb, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_STATIC);
	tb->AddControl(txt);


	wxCheckBox *chk = new wxCheckBox(tb, wxID_ANY, wxT("Spaces"), wxDefaultPosition, wxDefaultSize, 0);
	chk->SetValue(true);
	tb->AddControl(chk);
	tb->AddSeparator();

	wxButton *btn = new wxButton(tb, wxID_ANY, wxT("New"), wxDefaultPosition, wxDefaultSize, 0);
	tb->AddControl(btn);

	tb->Realize();

	return tb;
}

void MyFrame::OnExit(wxCommandEvent &WXUNUSED(event))
{
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
	wxString title = _("About wxDockingEntityX Demo");
	wxString text = _("wxDockingEntityX Demo\nAdemonstration for dockable windows in wxWidgets\n(c) Copyright by Gerhard Gruber");

	wxMessageBox(text, title, wxOK, this);
}

wxToolBar *MyFrame::CreateDockingToolbar(bool horizontal, bool topleft, wxDockingInfo &info)
{
	wxString title = "Toolbar ";
	wxToolBar *tb = nullptr;
	if (horizontal)
	{
		tb = horizontalToolBar(topleft);
		if (topleft)
		{
			info.SetDirection(wxTOP);
			title += "top";
		}
		else
		{
			info.SetDirection(wxBOTTOM);
			title += "bottom";
		}
	}
	else
	{
		tb = verticalToolBar(topleft);
		if (topleft)
		{
			info.SetDirection(wxLEFT);
			title += "left";
		}
		else
		{
			info.SetDirection(wxRIGHT);
			title += "right";
		}
	}

	info.SetTitle(title);

	return tb;
}

void MyFrame::OnToolbar(wxCommandEvent &event)
{
	wxString title = "Toolbar ";
	wxDockingInfo info;
	bool add = true;

	switch (event.GetId())
	{
		case ID_ToolbarLeftRemove:
			add = false;
			wxFALLTHROUGH;
		case ID_ToolbarLeftAdd:
			info.SetDirection(wxLEFT);
			title += "left";
		break;

		case ID_ToolbarRightRemove:
			add = false;
			wxFALLTHROUGH;
		case ID_ToolbarRightAdd:
			info.SetDirection(wxRIGHT);
			title += "right";
		break;

		case ID_ToolbarTopRemove:
			add = false;
			wxFALLTHROUGH;
		case ID_ToolbarTopAdd:
			info.SetDirection(wxTOP);
			title += "top";
		break;

		case ID_ToolbarBottomRemove:
			add = false;
			wxFALLTHROUGH;
		case ID_ToolbarBottomAdd:
			info.SetDirection(wxBOTTOM);
			title += "bottom";
		break;
	}

	wxToolBar *tb = nullptr;
	if (add)
	{
#pragma WARNING("Toolbar code missing")
/*		bool top = info.GetDirection() == wxTOP;
		bool left = info.GetDirection() == wxLEFT;

		tb = horizontalToolBar(top);
		tb = verticalToolBar(left);

		AddToolBar(tb, info);*/
	}
	//else
	//	RemoveToolBar(nullptr, info);
}

void MyFrame::OnLayoutCopySerialize(wxCommandEvent &)
{
#if wxUSE_CLIPBOARD
	//if (wxTheClipboard->Open())
	//{
	//	wxTheClipboard->SetData(new wxTextDataObject(s));
	//	wxTheClipboard->Close();
	//}
#endif
}

void MyFrame::OnLayoutSerialize(wxCommandEvent &)
{
}

void MyFrame::OnLayoutDeserialize(wxCommandEvent &)
{
}

void MyFrame::OnWindowClose(wxWindow *window)
{
	if (m_activePanel == window)
		m_activePanel = nullptr;
}

void MyFrame::OnMouseLeftDown(wxMouseEvent &event)
{
	wxPoint mousePos = ::wxGetMousePosition();
	wxWindow *w = wxFindWindowAtPoint(mousePos);

	wxSizeReportCtrl *p = wxDynamicCast(w, wxSizeReportCtrl);
	SetActivePanel(p);

	// Pass to the original mouse handler.
	event.Skip();
}

void MyFrame::OnMouseRightUp(wxMouseEvent &event)
{
	wxPoint mousePos = ::wxGetMousePosition();
	wxWindow *w = wxFindWindowAtPoint(mousePos);
	wxWindow *dockingChild = nullptr;
	wxDockingEntity p = wxDockingUtils::FindParentPanel(w, &dockingChild);
	wxSizeReportCtrl *ctrl = wxDynamicCast(dockingChild, wxSizeReportCtrl);

	wxMenu menu;
	wxString s;

	if (p.GetType() == wxDOCKING_FRAME)
		s = "Frame";
	else if (p.GetType() == wxDOCKING_NOTEBOOK)
		s = "Notebook";
	else if (p.GetType() == wxDOCKING_SPLITTER)
		s = "Splitter";
	else
		s = "UNKNOWN";

	wxDockingState const &gs = wxDockingState::GetInstance();
	wxDockingEntityState const &ps = gs.PanelState(p);
	wxDockingEntityState const &ctrls = gs.PanelState(ctrl);
	if (!s.empty())
	{
		if (ps.IsLocked())
			s = "Unlock "+s;
		else
			s = "Lock "+s;
		menu.Append(ID_ToggleContainerLock, s, wxT("The container of the panel (frame, splitter or notebook) is persistent"));
	}

	if (dockingChild != m_activePanel)
		SetActivePanel(ctrl);

	if (ctrl)
	{
		wxDockingUtils::wxUniqueVector<wxDockingEntity>::iterator pos = std::find(m_dockablePanels.begin(), m_dockablePanels.end(), p);
		if (pos == m_dockablePanels.end())
			s = "Disable docking";
		else
			s = "Disable docking";
		menu.Append(ID_ToggleDockingPanel, s, wxT("Enable/disable docking to this panel"));

		if (ctrls.IsDraggable())
			s = "Disable dragging";
		else
			s = "Enable dragging";
		menu.Append(ID_ToggleDragPanel, s);
	}

	UpdateStatusText(p, ctrl);
	mousePos = ScreenToClient(mousePos);
	PopupMenu(&menu, mousePos);

	// Pass to the original mouse handler.
	event.Skip();
}

void MyFrame::OnNewPanel(wxCommandEvent &event)
{
	wxDockingEntity const &active = GetActivePanel();
	if (!active)
		return;

	wxString title = "New Panel: ";
	title << ++m_newPanel;
	wxDockingInfo info(title);
	info.SetPanel(active);

	switch (event.GetId())
	{
		case ID_LayoutSplitCentral:
			info.SetDirection(wxCENTRAL);
		break;

		case ID_LayoutSplitLeft:
			info.SetDirection(wxLEFT);
		break;

		case ID_LayoutSplitRight:
			info.SetDirection(wxRIGHT);
		break;

		case ID_LayoutSplitTop:
			info.SetDirection(wxTOP);
		break;

		case ID_LayoutSplitBottom:
			info.SetDirection(wxBOTTOM);
		break;
	}

	// Create our new window that should be added.
	AddPanel(info, createSizeReportCtrl(title));
}

void MyFrame::OnNewPanelBorder(wxCommandEvent &event)
{
	wxString title = "New Borderpanel: ";
	title << ++m_newPanel;
	wxDockingInfo info(title);
	info.SetPanel(nullptr);		// nullpointer means to dock to the border

	switch (event.GetId())
	{
		case ID_LayoutSplitLeftBorder:
			info.SetDirection(wxLEFT);
		break;

		case ID_LayoutSplitRightBorder:
			info.SetDirection(wxRIGHT);
		break;

		case ID_LayoutSplitTopBorder:
			info.SetDirection(wxUP);
		break;

		case ID_LayoutSplitBottomBorder:
			info.SetDirection(wxDOWN);
		break;
	}

	AddPanel(info, createSizeReportCtrl(title));
}

void MyFrame::EnableOverlayMenus()
{
	wxMenuBar *menu = GetMenuBar();

	menu->Enable(ID_OverlayDisable, true);
	menu->Enable(ID_OverlayEnable, true);
	menu->Enable(ID_OverlayCustom, true);
	menu->Enable(ID_OverlayCustomHybrid, true);
}

void MyFrame::OnOverlayDisable(wxCommandEvent &WXUNUSED(evt))
{
	EnableOverlayMenus();

	wxMenuBar *menu = GetMenuBar();
	m_overlayMode = ID_OverlayDisable;
	menu->Enable(ID_OverlayDisable, false);
}

void MyFrame::OnOverlayEnable(wxCommandEvent &WXUNUSED(evt))
{
	EnableOverlayMenus();

	wxMenuBar *menu = GetMenuBar();
	m_overlayMode = ID_OverlayEnable;
	menu->Enable(ID_OverlayEnable, false);
}

// The custom overlay disables docking vie the windows and allows only docking using the overlay buttons
void MyFrame::OnOverlayCustom(wxCommandEvent &WXUNUSED(evt))
{
	EnableOverlayMenus();

	wxMenuBar *menu = GetMenuBar();
	m_overlayMode = ID_OverlayCustom;
	menu->Enable(ID_OverlayCustom, false);
}

// The hybrid custom overlay enables docking via buttons or windows
void MyFrame::OnOverlayCustomHybrid(wxCommandEvent &WXUNUSED(evt))
{
	EnableOverlayMenus();

	wxMenuBar *menu = GetMenuBar();
	m_overlayMode = ID_OverlayCustomHybrid;
	menu->Enable(ID_OverlayCustomHybrid, false);

	// Only menu is implemented, functionality missing
	//wxCHECK_MSG(false, (void)0, wxT("Not implemented"));
}

void MyFrame::OnToggleDockingPanel(wxCommandEvent &WXUNUSED(evt))
{
	m_dockablePanels;
	wxSizeReportCtrl *panel = GetActivePanel();
	if (panel)
		panel->SetDockable(!panel->IsDockable());
}

void MyFrame::OnToggleDragPanel(wxCommandEvent &WXUNUSED(evt))
{
	wxSizeReportCtrl *panel = GetActivePanel();
	if (panel)
	{
		wxDockingState &gs = wxDockingState::GetInstance();
		wxDockingEntityState &ps = gs.PanelState(m_activePanel);
		ps.SetDraggable(!ps.IsDraggable());
	}
}

void MyFrame::OnToggleDocking(wxCommandEvent &WXUNUSED(event))
{
	m_allowDocking = !m_allowDocking;
}

void MyFrame::OnToggleContainerLock(wxCommandEvent &WXUNUSED(evt))
{
	if (!m_activeContainer)
		return;

	wxDockingState &gs = wxDockingState::GetInstance();
	wxDockingEntityState &ps = gs.PanelState(m_activeContainer);
	ps.SetLocked(!ps.IsLocked());
}

void MyFrame::OnRemoveDockingPanel(wxCommandEvent &WXUNUSED(evt))
{
	if (!m_activePanel)
		return;

	wxSizeReportCtrl *w = m_activePanel;

	// If the panel was successfully removed we can delete our window.
	if(RemovePanel(w->GetParent(), w))
		delete w;

	m_activePanel = nullptr;
}

void MyFrame::createInitialLayout()
{
	wxDockingInfo::GetDefaults()
		.SetTabDirection(wxTOP)				// This is not really needed as it is default
		//.SetTabDirection(wxBOTTOM)
		//.SetTabDirection(wxLEFT)
		//.SetTabDirection(wxRIGHT)
	;

	// The first one is attached to the root panel, so we don't need to specify it.
	wxDockingEntity rootTab = AddPanel(wxDockingInfo("Size Report 1.1")
		.SetForcePanel(true)
		.SetDirection(wxCENTRAL)
		//.SetDirection(wxRIGHT)
		//.SetForceSplit(true)				// This will cause the initial panel to be already shown in a splitted state with a placeholder window on the free side.
		//.SetSize(150, -1)
		, createSizeReportCtrl("Ctrl1.1")
	);

	wxDockingState &gs = wxDockingState::GetInstance();
	gs.SetLock(rootTab);
	rootTab = AddPanel(wxDockingInfo("Size Report 1.2")
		.SetPanel(rootTab)
		, createSizeReportCtrl("Ctrl1.2")
	);
	//rootTab = AddPanel(wxDockingInfo("Size Report 1.3")
	//	.SetPanel(rootTab)
	//	, createSizeReportCtrl("Ctrl1.3")
	//);
	//rootTab = AddPanel(wxDockingInfo("Size Report 1.4")
	//	.SetPanel(rootTab)
	//	, createSizeReportCtrl("Ctrl1.4")
	//);

	wxDockingEntity l = AddPanel(wxDockingInfo("Size Report 2.0")
		.SetPanel(rootTab)
		.SetDirection(wxRIGHT)
		.SetSize(150, -1)
		, createSizeReportCtrl("Ctrl2.0")
	);

	// DockingPanel is nullptr, so we split at the right side of the frame.
	AddPanel(wxDockingInfo("Size Report 3.0")
		.SetDirection(wxLEFT)
		.SetSize(150, -1)
		, createSizeReportCtrl("Ctrl3.0")
	);
	AddPanel(wxDockingInfo("Size Report 4.0")
		.SetPanel(l)
		.SetDirection(wxDOWN)
		.SetSize(-1, 100)
		, createSizeReportCtrl("Ctrl4.0")
	);
	AddPanel(wxDockingInfo("Size Report 5.0")
		.SetPanel(rootTab)
		.SetDirection(wxDOWN)
		.SetSize(-1, 100)
		, createSizeReportCtrl("Ctrl5.0")
	);
	AddPanel(wxDockingInfo("Size Report 6.0")
		.SetDirection(wxUP)
		.SetSize(-1, 100)
		, createSizeReportCtrl("Ctrl6.0")
	);
}
