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
#include <wx/docking/docking.h>

#include "../sample.xpm"

// -- application --

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
		wxWindow *window = NULL)
	: wxControl(parent, id, pos, size, wxNO_BORDER)
	, m_window(window)
	, m_text(text)
	{
	}

private:
	void OnPaint(wxPaintEvent& WXUNUSED(evt))
	{
		wxPaintDC dc(this);
		wxSize size = GetClientSize();
		wxString s;
		int h, w, height;

		s.Printf("Size: %d x %d ", size.x, size.y);
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
			dc.DrawText(s, (size.x - w) / 2, ((size.y - (height * 5)) / 2) + (height * 2));
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

class MyFrame : public wxDockingFrame
{
public:
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
		ID_LayoutSplitBottomBorder
	};

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

protected:
	wxMenu *createDockingMenu(void);
	void createInitialLayout(void);
	wxSizeReportCtrl *createSizeReportCtrl(wxString const &text);

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
	mb->Append(helpMenu, _("&Help"));

	SetMenuBar(mb);

	CreateStatusBar();
	GetStatusBar()->SetStatusText(_("Ready"));

	SetMinSize(FromDIP(wxSize(400,300)));
	createInitialLayout();
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

wxSizeReportCtrl *MyFrame::createSizeReportCtrl(wxString const &text)
{
	return new wxSizeReportCtrl(this, text);
}

void MyFrame::OnExit(wxCommandEvent& WXUNUSED(event))
{
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(_("wxDockingPanel Demo\nAdemonstration for dockable windows in wxWidgets\n(c) Copyright by Gerhard Gruber"),
				 _("About wxDockingPanel Demo"), wxOK, this);
}

void MyFrame::OnLayoutCopySerialize(wxCommandEvent &)
{
/*	wxString s = SerializeLayout() + "\n\n";
	wxSize csz = GetClientSize();
	s << "FrameSize: " << csz.x << "/" << csz.y << "\n";
	s << "TabCtrlInfo:\n";

	for (int i = 0; i < nb->GetPageCount(); i++)
	{
		wxWindow *page = nb->GetPage(i);
		wxAuiTabCtrl *ctrl;
		int tabIndex;
		nb->FindTab(page, &ctrl, &tabIndex);

		wxSize sz = page->GetClientSize();
		wxPoint pt = page->GetPosition();
		wxSize tsz = ctrl->GetClientSize();
		wxPoint tpt = ctrl->GetPosition();

		s << "Name: " + nb->GetPageText(i);
		s << " TabCtrl: " << (void *)ctrl;
		s << " Window: " << (void *)page;
		s << " PageIndex: " << i;
		s << " TabIndex: " << tabIndex;
		s << " TabPosition: " << tpt.x << "/" << tpt.y;
		s << " TabSize: " << tsz.x << "/" << tsz.y;
		s << " PagePosition: " << pt.x << "/" << pt.y;
		s << " PageSize: " << sz.x << "/" << sz.y;
		s << "\n";
	}

#if wxUSE_CLIPBOARD
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxTextDataObject(s));
		wxTheClipboard->Close();
	}
#endif*/
}

void MyFrame::OnLayoutSerialize(wxCommandEvent &)
{
/*	wxAuiNotebook *nb = FindNotebook();
	if (!nb)
		return;

	m_notebookLayout = nb->SerializeLayout();*/
}

void MyFrame::OnLayoutDeserialize(wxCommandEvent &)
{
/*	wxAuiNotebook *nb = FindNotebook();
	if (!nb)
		return;

	nb->DeserializeLayout(m_notebookLayout);*/
}

void MyFrame::OnLayoutSplit(wxCommandEvent &evt)
{
/*	wxAuiNotebook *nb = FindNotebook();
	if (!nb)
		return;

	int direction = 0;
	bool border = false;
	switch (evt.GetId())
	{
	case ID_NotebookSplitLeftBorder:
		direction = wxLEFT;
		border = true;
		break;

	case ID_NotebookSplitLeft:
		direction = wxLEFT;
		break;

	case ID_NotebookSplitRightBorder:
		direction = wxRIGHT;
		border = true;
		break;

	case ID_NotebookSplitRight:
		direction = wxRIGHT;
		break;

	case ID_NotebookSplitTopBorder:
		direction = wxTOP;
		border = true;
		break;

	case ID_NotebookSplitTop:
		direction = wxTOP;
		break;

	case ID_NotebookSplitBottomBorder:
		direction = wxBOTTOM;
		border = true;
		break;

	case ID_NotebookSplitBottom:
		direction = wxBOTTOM;
		break;

	default:
		return;
	}

	int page = nb->GetSelection();
	wxAuiTabCtrl *ctrl = nullptr;
	if (!border)
		ctrl = nb->FindTab(nb->GetPage(nb->GetSelection()));

	nb->Split(page, direction, ctrl, border);*/
}

void MyFrame::createInitialLayout(void)
{
	// For the first panel it doesn't really matter, which direction is specified
	// but wxCENTRAL should be used.
	wxDockingPanel *p = AddPanel(createSizeReportCtrl("Ctrl1"), "Size Report 1", wxCENTRAL);
	p = SplitPanel(createSizeReportCtrl("Ctrl2"), "Size Report 2", wxRIGHT, p);
	SplitPanel(createSizeReportCtrl("Ctrl3"), "Size Report 3", wxDOWN, p);
}
