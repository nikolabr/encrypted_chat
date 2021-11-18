// encrypted_chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "crypto.h"
#include <wx/wx.h>

class User : public CryptographicUser {
public: 
    std::string name; 
};

class EncryptedChatApp : public wxApp {
public: 
    User user;
    EncryptedChatApp();
    virtual ~EncryptedChatApp();
    virtual bool OnInit() override;
};

EncryptedChatApp::EncryptedChatApp()
{
}

EncryptedChatApp::~EncryptedChatApp()
{
}

class AppFrame : public wxFrame
{
public:
    AppFrame(int status);

private:
    void OnExit(wxCommandEvent& event);
    void UsernamePrompt(wxCommandEvent& event);
};

AppFrame::AppFrame(int status) : wxFrame(nullptr, wxID_ANY, L"Encrypted Chat") {
    sodium_init();

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    wxPanel* panel = new wxPanel(this, wxID_ANY);

    wxTextCtrl* text_output = new wxTextCtrl(panel, wxID_ANY, wxT(""),
        wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);
    vbox->Add(text_output, 4, wxEXPAND | wxTOP);

    wxTextCtrl *text_input = new wxTextCtrl(panel, wxID_ANY, wxT(""),
        wxDefaultPosition, wxDefaultSize, 0);
    vbox->Add(text_input, 1, wxEXPAND | wxBOTTOM);

    panel->SetSizer(vbox);

    CreateStatusBar();
    if (status == 0)
        SetStatusText("Successfully generated keypair!");
    else
        SetStatusText("Failed to generate keypair!");

    Center();
}

void AppFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

bool EncryptedChatApp::OnInit()
{
    user.name = "me";
    AppFrame* mainFrame = new AppFrame(user.keypair.success);
    mainFrame->Show(true);
    return true;
}

wxIMPLEMENT_APP(EncryptedChatApp);
wxIMPLEMENT_WXWIN_MAIN_CONSOLE;

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
