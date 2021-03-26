// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "wx/app.h"
#include "wx/clipbrd.h"
#include "wx/menu.h"
#include "wx/toolbar.h"
#include "wx/statusbr.h"
#include "wx/msgdlg.h"
#include <wx/icon.h>
#include <wx/dcclient.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>

#include <wx/docking/docking.h>

#include "../sample.xpm"

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

class MyApp : public wxApp
{
public:
	bool OnInit() wxOVERRIDE;
};

wxDECLARE_APP(MyApp);
wxIMPLEMENT_APP(MyApp);

// -- wxSizeReportCtrl -- (an utility control that always reports it's client size)
class wxSizeReportCtrl : public wxControl
{
public:
	wxSizeReportCtrl(wxWindow* parent, wxString const &text, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		wxWindow *window = nullptr)
	: wxControl(parent, id, pos, size, wxNO_BORDER)
	, m_window(window)
	, m_text(text)
	{
		if (!m_window)
			m_window = parent;
	}

	void setWindow(wxWindow *window)
	{
		m_window = window;
	}

private:
	void OnPaint(wxPaintEvent& WXUNUSED(evt))
	{
		wxPaintDC dc(this);
		wxSize size = GetClientSize();
		wxString s;
		int h, w, height;

		wxPoint mpos = GetPosition();
		s.Printf("Pos: %d/%d Size: %d x %d ", mpos.x, mpos.y, size.x, size.y);
		s += m_text;

		dc.SetFont(*wxNORMAL_FONT);
		dc.GetTextExtent(s, &w, &height);
		height += FromDIP(3);
		dc.SetBrush(*wxWHITE_BRUSH);
		dc.SetPen(*wxWHITE_PEN);
		dc.DrawRectangle(0, 0, size.x, size.y);
		dc.SetPen(*wxLIGHT_GREY_PEN);
		dc.DrawLine(0, 0, size.x, size.y);
		dc.DrawLine(0, size.y, size.x, 0);
		dc.DrawText(s, (size.x-w)/2, ((size.y-(height*5))/2));

		if (m_window)
		{
			wxPoint pos = m_window->GetPosition();
			wxSize sz = m_window->GetSize();

			s.Printf("Position: %d/%d", pos.x, pos.y);
			dc.GetTextExtent(s, &w, &h);
			dc.DrawText(s, (size.x-w)/2, ((size.y-(height*5))/2)+(height*2));

			s.Printf("Size: %d/%d", sz.x, sz.y);
			dc.GetTextExtent(s, &w, &h);
			dc.DrawText(s, (size.x - w) / 2, ((size.y - (height * 5)) / 2) + (height * 2)+20);
		}
	}

	void OnEraseBackground(wxEraseEvent& WXUNUSED(evt))
	{
		// intentionally empty
	}

	void OnSize(wxSizeEvent& WXUNUSED(evt))
	{
		Refresh();
	}

private:
	wxWindow *m_window;
	wxString m_text;
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxSizeReportCtrl, wxControl)
	EVT_PAINT(wxSizeReportCtrl::OnPaint)
	EVT_SIZE(wxSizeReportCtrl::OnSize)
	EVT_ERASE_BACKGROUND(wxSizeReportCtrl::OnEraseBackground)
wxEND_EVENT_TABLE()

wxSizeReportCtrl *rightWatcher;
wxSizeReportCtrl *bottomWatcher;

enum
{
	ID_Exit = wxID_EXIT,
	ID_About = wxID_ABOUT,

	ID_CreateTree = wxID_HIGHEST + 1,

	ID_LayoutSerialize,
	ID_LayoutDeserialize,
	ID_LayoutCopySerialize,
	ID_LayoutSplitLeft,
	ID_LayoutSplitRight,
	ID_LayoutSplitTop,
	ID_LayoutSplitBottom,
	ID_LayoutSplitLeftBorder,
	ID_LayoutSplitRightBorder,
	ID_LayoutSplitTopBorder,
	ID_LayoutSplitBottomBorder,

	ID_ToolbarLeftAdd,
	ID_ToolbarRightAdd,
	ID_ToolbarTopAdd,
	ID_ToolbarBottomAdd,

	ID_ToolbarLeftRemove,
	ID_ToolbarRightRemove,
	ID_ToolbarTopRemove,
	ID_ToolbarBottomRemove,
};

class MyFrame : public wxDockingFrame
{
public:
	MyFrame(wxWindow *parent,
			wxWindowID id,
			const wxString &title,
			const wxPoint &pos = wxDefaultPosition,
			const wxSize &size = wxDefaultSize,
			long style = wxDEFAULT_FRAME_STYLE | wxSUNKEN_BORDER);

public:
	void OnExit(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);

	void OnLayoutSplit(wxCommandEvent &evt);
	void OnLayoutSerialize(wxCommandEvent &evt);
	void OnLayoutDeserialize(wxCommandEvent &evt);
	void OnLayoutCopySerialize(wxCommandEvent &evt);

	void OnToolbar(wxCommandEvent &evt);
	wxToolBar *CreateDockingToolbar(bool top, bool left, wxDockingInfo &info);

	wxSizeReportCtrl *createSizeReportCtrl(wxString const &text, wxWindow *window = nullptr);

protected:
	wxMenu *createDockingMenu(void);
	wxMenu *createToolbarMenu(void);
	void createInitialLayout(void);
	wxToolBar *horizontalToolBar(bool top);
	wxToolBar *verticalToolBar(bool left);

private:
	wxDECLARE_EVENT_TABLE();
};

bool MyApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	wxFrame *frame = new MyFrame(NULL,
								 wxID_ANY,
								 "wxDocking Sample Application",
								 wxDefaultPosition,
								 wxWindow::FromDIP(wxSize(800, 600), NULL));
	frame->Show();

	return true;
}

wxBEGIN_EVENT_TABLE(MyFrame, wxDockingFrame)
	EVT_MENU(ID_Exit, MyFrame::OnExit)
	EVT_MENU(ID_About, MyFrame::OnAbout)

	EVT_MENU(ID_LayoutSerialize, MyFrame::OnLayoutSerialize)
	EVT_MENU(ID_LayoutDeserialize, MyFrame::OnLayoutDeserialize)
	EVT_MENU(ID_LayoutCopySerialize, MyFrame::OnLayoutCopySerialize)
	EVT_MENU(ID_LayoutSplitLeft, MyFrame::OnLayoutSplit)
	EVT_MENU(ID_LayoutSplitRight, MyFrame::OnLayoutSplit)
	EVT_MENU(ID_LayoutSplitTop, MyFrame::OnLayoutSplit)
	EVT_MENU(ID_LayoutSplitBottom, MyFrame::OnLayoutSplit)
	EVT_MENU(ID_LayoutSplitLeftBorder, MyFrame::OnLayoutSplit)
	EVT_MENU(ID_LayoutSplitRightBorder, MyFrame::OnLayoutSplit)
	EVT_MENU(ID_LayoutSplitTopBorder, MyFrame::OnLayoutSplit)
	EVT_MENU(ID_LayoutSplitBottomBorder, MyFrame::OnLayoutSplit)


	EVT_MENU(ID_ToolbarLeftAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarRightAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarTopAdd, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarBottomAdd, MyFrame::OnToolbar)

	EVT_MENU(ID_ToolbarLeftRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarRightRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarTopRemove, MyFrame::OnToolbar)
	EVT_MENU(ID_ToolbarBottomRemove, MyFrame::OnToolbar)

wxEND_EVENT_TABLE()

MyFrame::MyFrame(wxWindow *parent,
				 wxWindowID id,
				 const wxString &title,
				 const wxPoint &pos,
				 const wxSize &size,
				 long style)
: wxDockingFrame(parent, id, title, pos, size, style)
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

	CreateStatusBar();
	GetStatusBar()->SetStatusText(_("Ready"));

	SetMinSize(wxSize(200,150));

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

	createInitialLayout();
	Refresh();
}

wxMenu *MyFrame::createDockingMenu(void)
{
	wxMenu *menu = new wxMenu;

	wxMenu *submenu = new wxMenu;
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitLeft, wxString(wxT("Left")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitRight, wxString(wxT("Right")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitTop, wxString(wxT("Top")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitBottom, wxString(wxT("Bottom")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("Split tab"), wxEmptyString, wxITEM_NORMAL, submenu));
	submenu = new wxMenu;
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitLeftBorder, wxString(wxT("Left")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitRightBorder, wxString(wxT("Right")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitTopBorder, wxString(wxT("Top")), wxEmptyString, wxITEM_NORMAL));
	submenu->Append(new wxMenuItem(submenu, ID_LayoutSplitBottomBorder, wxString(wxT("Bottom")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(menu, wxID_ANY, wxT("Split to border"), wxEmptyString, wxITEM_NORMAL, submenu));
	menu->AppendSeparator();
	menu->Append(new wxMenuItem(submenu, ID_LayoutCopySerialize, wxString(wxT("Copy layout to Clipboard")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(submenu, ID_LayoutSerialize, wxString(wxT("Save layout")), wxEmptyString, wxITEM_NORMAL));
	menu->Append(new wxMenuItem(submenu, ID_LayoutDeserialize, wxString(wxT("Restore layout")), wxEmptyString, wxITEM_NORMAL));

	return menu;
}

wxMenu *MyFrame::createToolbarMenu(void)
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

wxSizeReportCtrl *MyFrame::createSizeReportCtrl(wxString const &text, wxWindow *window)
{
	return new wxSizeReportCtrl(this, text, wxID_ANY, wxDefaultPosition, wxDefaultSize, window);
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

void MyFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxString title = _("About wxDockingPanel Demo");
	wxString text = _("wxDockingPanel Demo\nAdemonstration for dockable windows in wxWidgets\n(c) Copyright by Gerhard Gruber");

	wxMessageBox(text, title, wxOK, this);
}

wxToolBar *MyFrame::CreateDockingToolbar(bool horizontal, bool topleft, wxDockingInfo &info)
{
	wxString title = "Toolbar ";
	wxToolBar *tb = nullptr;
	if (horizontal)
	{
		tb = horizontalToolBar(topleft);
		info.horizontal(topleft);
		if (topleft)
			title += "top";
		else
			title += "bottom";
	}
	else
	{
		tb = verticalToolBar(topleft);
		info.vertical(topleft);
		if (topleft)
			title += "left";
		else
			title += "right";
	}

	info.title(title);

	return tb;
}

void MyFrame::OnToolbar(wxCommandEvent &event)
{
	wxString title = "Toolbar ";
	wxDockingInfo info;
	bool add = true;
	bool left = true;
	bool top = true;

	switch (event.GetId())
	{
		case ID_ToolbarLeftRemove:
			add = false;
		case ID_ToolbarLeftAdd:
			info.vertical(true);
			title += "left";
		break;

		case ID_ToolbarRightRemove:
			add = false;
		case ID_ToolbarRightAdd:
			left = false;
			info.vertical(false);
			title += "right";
		break;

		case ID_ToolbarTopRemove:
			add = false;
		case ID_ToolbarTopAdd:
			info.horizontal(true);
			title += "top";
		break;

		case ID_ToolbarBottomRemove:
			add = false;
		case ID_ToolbarBottomAdd:
			top = false;
			info.horizontal(false);
			title += "bottom";
		break;
	}

	wxToolBar *tb = nullptr;
	if (add)
	{
		if (info.isHorizontal())
			tb = horizontalToolBar(top);
		else
			tb = verticalToolBar(left);

		AddToolBar(tb, info);
	}
	else
		RemoveToolBar(nullptr, info);

	if (info.isBottom())
		bottomWatcher->setWindow(tb);

	if (info.isRight())
		rightWatcher->setWindow(tb);
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

void MyFrame::OnLayoutSplit(wxCommandEvent &)
{
}

void MyFrame::createInitialLayout(void)
{
	// For the first panel it doesn't really matter, which direction is specified
	// but wxCENTRAL should be used.
	// The first panel could also be created by TabbedPanel() docking to the rootpanel.
	// Example:
	// TabbedPanel(createSizeReportCtrl("Ctrl1.1"), wxDockingInfo("Size Report 1.1").dock(GetRootPanel()));
	// 
	// The first panel can never be splitted because there is nothing to split.
	// 
	// Since a floating window is seperate, and not docked to any other window, it can be created
	// any time, but adding tabs or splits, will obviously only happen in the float and not affect
	// the main frame, which would stay empty in this case.
	// Technically there is no difference between a floating frame and the main frame, so the main window
	// can be closed if there is a suitable frame still open (which would have to take special measures though).
	// By default, the first frame is the main frame and the app closes when this frame is closed.
	wxDockingPanel *p = AddPanel(createSizeReportCtrl("Ctrl1.1"), wxDockingInfo("Size Report 1.1"));
	rightWatcher = createSizeReportCtrl("Ctrl1.2");
	TabbedPanel(rightWatcher, wxDockingInfo("Size Report 1.2").dock(p));

	bottomWatcher = createSizeReportCtrl("Ctrl2");
	p = SplitPanel(bottomWatcher, wxDockingInfo("Size Report 2").dock(p).right());
	wxDockingPanel *p1 = SplitPanel(createSizeReportCtrl("Ctrl3.0"), wxDockingInfo("Size Report 3.0").dock(p).down());
	TabbedPanel(createSizeReportCtrl("Ctrl3.1"), wxDockingInfo("Size Report 3.1").dock(p1));
	//FloatPanel(createSizeReportCtrl("Ctrl4"), wxDockingInfo("Size Report 4"));
	//AddToolBar(nullptr, wxDockingInfo("Toolbar 1 Horizontal").vertical());
}
