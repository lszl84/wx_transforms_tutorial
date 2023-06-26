#include <wx/wx.h>
#include <wx/wrapsizer.h>
#include <wx/splitter.h>

#include <wx/config.h>
#include <wx/filehistory.h>

#include <string>
#include <vector>
#include <tuple>

#include "myapp.h"

#include "panes/colorpane.h"
#include "panes/pensizepane.h"
#include "panes/toolselectionpane.h"

#include "canvas/drawingcanvas.h"
#include "drawingdocument.h"
#include "drawingview.h"

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxDocParentFrame
{
public:
    MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString &title,
            const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

    void SetupCanvasForView(DrawingView *view);

private:
    wxScrolled<wxPanel> *BuildControlsPanel(wxWindow *parent);

    void SetupColorPanes(wxWindow *parent, wxSizer *sizer);
    void SetupPenPanes(wxWindow *parent, wxSizer *sizer);
    void SetupToolPanes(wxWindow *parent, wxSizer *sizer);

    void SelectColorPane(ColorPane *pane);
    void SelectPenPane(PenSizePane *pane);
    void SelectToolPane(ToolSelectionPane *pane);

    void BuildMenuBar();

    std::vector<ColorPane *> colorPanes{};
    std::vector<PenSizePane *> penPanes{};
    std::vector<ToolSelectionPane *> toolPanes{};

    wxPanel *docPanel;
    wxScrolled<wxPanel> *controlsPanel;

    // For hiding/showing pen width controls.
    // Can't use a single sizer because it will mess up the layout: https://github.com/wxWidgets/wxWidgets/issues/23352
    wxStaticText *penWidthLabel;
    wxSizer *penWidthPanesSizer;

    const std::vector<std::string> niceColors = {"#000000", "#ffffff", "#fd7f6f",
                                                 "#7eb0d5", "#b2e061", "#bd7ebe",
                                                 "#ffb55a", "#ffee65", "#beb9db",
                                                 "#fdcce5", "#8bd3c7"};

    const int penCount = 6;

    const std::string lightBackground = "#f4f3f3";
    const std::string darkBackground = "#2c2828";
};

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();

    SetAppName("PaintAppWx");
    SetAppDisplayName("Paint App");

    docManager.reset(new wxDocManager);
    docManager->SetMaxDocsOpen(1);

    new wxDocTemplate(docManager.get(), "Drawing",
                      "*.pxz", "", "pxz", "DrawingDocument", "DrawingView",
                      CLASSINFO(DrawingDocument),
                      CLASSINFO(DrawingView));

    frame = new MyFrame(docManager.get(), nullptr, wxID_ANY, wxGetApp().GetAppDisplayName());
    frame->Show(true);
    return true;
}

int MyApp::OnExit()
{
    docManager->FileHistorySave(*wxConfig::Get());
    return wxApp::OnExit();
}

ToolSettings &MyApp::GetToolSettings()
{
    return wxGetApp().toolSettings;
}

void MyApp::SetupCanvasForView(DrawingView *view)
{
    wxGetApp().frame->SetupCanvasForView(view);
}

void MyFrame::SetupCanvasForView(DrawingView *view)
{
    if (docPanel->GetChildren().size() > 0)
    {
        docPanel->GetSizer()->Clear(true);
    }

    if (view != nullptr)
    {
        auto canvas = new DrawingCanvas(docPanel, view, wxID_ANY, wxDefaultPosition, wxDefaultSize);
        docPanel->GetSizer()->Add(canvas, 1, wxEXPAND);

        view->SetFrame(this);
    }
    else
    {
        this->SetTitle(wxGetApp().GetAppDisplayName());
    }

    docPanel->Layout();
}

void MyFrame::SetupColorPanes(wxWindow *parent, wxSizer *sizer)
{
    for (const auto &color : niceColors)
    {
        auto colorPane = new ColorPane(parent, wxID_ANY, wxColour(color));

        colorPane->Bind(wxEVT_LEFT_DOWN, [this, colorPane](wxMouseEvent &event)
                        { SelectColorPane(colorPane); });

        colorPanes.push_back(colorPane);
        sizer->Add(colorPane, 0, wxRIGHT | wxBOTTOM, FromDIP(5));
    }
}

void MyFrame::SetupPenPanes(wxWindow *parent, wxSizer *sizer)
{
    for (int i = 0; i < penCount; i++)
    {
        auto penPane = new PenSizePane(parent, wxID_ANY, i * FromDIP(4) + FromDIP(1));

        penPane->Bind(wxEVT_LEFT_DOWN, [this, penPane](wxMouseEvent &event)
                      { SelectPenPane(penPane); });

        penPanes.push_back(penPane);
        sizer->Add(penPane, 0, wxRIGHT | wxBOTTOM, FromDIP(5));
    }
}

void MyFrame::SetupToolPanes(wxWindow *parent, wxSizer *sizer)
{
    for (const auto toolType : {ToolType::Pen, ToolType::Rect, ToolType::Circle})
    {
        auto toolPane = new ToolSelectionPane(parent, wxID_ANY, toolType);

        toolPane->Bind(wxEVT_LEFT_DOWN, [this, toolPane](wxMouseEvent &event)
                       { SelectToolPane(toolPane); });

        toolPanes.push_back(toolPane);
        sizer->Add(toolPane, 0, wxRIGHT | wxBOTTOM, FromDIP(5));
    }
}

wxScrolled<wxPanel> *MyFrame::BuildControlsPanel(wxWindow *parent)
{
    auto panel = new wxScrolled<wxPanel>(parent, wxID_ANY);
    panel->SetScrollRate(0, FromDIP(10));

    bool isDark = wxSystemSettings::GetAppearance().IsDark();
    panel->SetBackgroundColour(wxColour(isDark ? darkBackground : lightBackground));

    auto mainSizer = new wxBoxSizer(wxVERTICAL);

    auto addGroup = [this, panel, mainSizer](const wxString &title, const auto factoryFunction)
    {
        auto text = new wxStaticText(panel, wxID_ANY, title);
        mainSizer->Add(text, 0, wxALL, FromDIP(5));

        auto wrapSizer = new wxWrapSizer(wxHORIZONTAL);
        (this->*factoryFunction)(panel, wrapSizer);
        mainSizer->Add(wrapSizer, 0, wxALL, FromDIP(5));

        return std::make_pair(text, wrapSizer);
    };

    addGroup("Color", &MyFrame::SetupColorPanes);
    addGroup("Tool", &MyFrame::SetupToolPanes);

    std::tie(penWidthLabel, penWidthPanesSizer) = addGroup("Width", &MyFrame::SetupPenPanes);

    panel->SetSizer(mainSizer);

    return panel;
}

MyFrame::MyFrame(wxDocManager *manager, wxFrame *frame, wxWindowID id, const wxString &title,
                 const wxPoint &pos, const wxSize &size)
    : wxDocParentFrame(manager, frame, id, title, pos, size)
{
    wxSplitterWindow *splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);

    splitter->SetMinimumPaneSize(FromDIP(220));

    controlsPanel = BuildControlsPanel(splitter);
    docPanel = new wxPanel(splitter, wxID_ANY);
    docPanel->SetSizer(new wxBoxSizer(wxVERTICAL));

    splitter->SplitVertically(controlsPanel, docPanel);
    splitter->SetSashPosition(FromDIP(240));

    this->SetSize(FromDIP(800), FromDIP(600));
    this->SetMinSize({FromDIP(400), FromDIP(500)});

    SelectColorPane(colorPanes[0]);
    SelectPenPane(penPanes[0]);
    SelectToolPane(toolPanes[0]);

    BuildMenuBar();
}

void MyFrame::SelectColorPane(ColorPane *pane)
{
    for (auto colorPane : colorPanes)
    {
        colorPane->selected = (colorPane == pane);
        colorPane->Refresh();
    }

    MyApp::GetToolSettings().currentColor = pane->color;
}

void MyFrame::SelectPenPane(PenSizePane *pane)
{
    for (auto penPane : penPanes)
    {
        penPane->selected = (penPane == pane);
        penPane->Refresh();
    }

    MyApp::GetToolSettings().currentWidth = pane->penWidth;
}

void MyFrame::SelectToolPane(ToolSelectionPane *pane)
{
    for (auto toolPane : toolPanes)
    {
        toolPane->selected = (toolPane == pane);
        toolPane->Refresh();
    }

    MyApp::GetToolSettings().currentTool = pane->toolType;

    if (pane->toolType == ToolType::Pen)
    {
        controlsPanel->GetSizer()->Show(penWidthLabel);
        controlsPanel->GetSizer()->Show(penWidthPanesSizer);
    }
    else
    {
        controlsPanel->GetSizer()->Hide(penWidthLabel);
        controlsPanel->GetSizer()->Hide(penWidthPanesSizer);
    }

    controlsPanel->Layout();
}

void MyFrame::BuildMenuBar()
{
    constexpr int ClearHistoryMenuId = 10001;

    auto menuBar = new wxMenuBar;

    auto fileMenu = new wxMenu;
    fileMenu->Append(wxID_NEW);
    fileMenu->Append(wxID_OPEN);

    auto recentFilesMenu = new wxMenu;
    recentFilesMenu->Append(ClearHistoryMenuId, "&Clear");

    fileMenu->AppendSubMenu(recentFilesMenu, "Open Recent");
    fileMenu->AppendSeparator();

    fileMenu->Append(wxID_SAVE);
    fileMenu->Append(wxID_SAVEAS);
    fileMenu->Append(wxID_CLOSE);
    fileMenu->Append(wxID_EXIT);

    menuBar->Append(fileMenu, "&File");

    GetDocumentManager()->FileHistoryUseMenu(recentFilesMenu);
    GetDocumentManager()->FileHistoryLoad(*wxConfig::Get());

    this->Bind(
        wxEVT_MENU, [this](wxCommandEvent &event)
        {
            while (GetDocumentManager()->GetFileHistory()->GetCount() > 0)
            {
                GetDocumentManager()->GetFileHistory()->RemoveFileFromHistory(0);
            } },
        ClearHistoryMenuId);

    auto editMenu = new wxMenu;
    editMenu->Append(wxID_UNDO);
    editMenu->Append(wxID_REDO);
    editMenu->AppendSeparator();
    editMenu->Append(wxID_CUT);
    editMenu->Append(wxID_COPY);
    editMenu->Append(wxID_PASTE);
    editMenu->Append(wxID_DELETE);
    editMenu->AppendSeparator();
    editMenu->Append(wxID_SELECTALL);

    menuBar->Append(editMenu, "&Edit");

    SetMenuBar(menuBar);
}