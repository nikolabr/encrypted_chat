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
    wxTextCtrl* text_output;
    wxTextCtrl* text_input;

private:
    void OnExit(wxCommandEvent& event);
    void UsernamePrompt(wxCommandEvent& event);
    void OnSend(wxCommandEvent& event);
};

enum {
    ID_Output = 1,
    ID_Input = 2
};

void AppFrame::OnSend(wxCommandEvent& event) {
    std::string input = text_input->GetValue().utf8_string();
    User user = 
    text_output->WriteText(wxNow() << ' ' << "Encrypted" << ": " << input << '\n');
    text_input->Clear();
}

AppFrame::AppFrame(int status) : wxFrame(nullptr, wxID_ANY, L"Encrypted Chat") {

    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);
    wxPanel* panel = new wxPanel(this, wxID_ANY);

    text_output = new wxTextCtrl(panel, ID_Output, wxT(""),
        wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_WORDWRAP);
    vbox->Add(text_output, 4, wxEXPAND | wxTOP);

    text_input = new wxTextCtrl(panel, ID_Input, wxT(""),
        wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    vbox->Add(text_input, 1, wxEXPAND | wxBOTTOM);

    panel->SetSizer(vbox);

    CreateStatusBar();
    if (status == 0)
        SetStatusText("Successfully generated keypair!");
    else
        SetStatusText("Failed to generate keypair!");

    Bind(wxEVT_TEXT_ENTER, &AppFrame::OnSend, this, ID_Input);
    Center();
}

void AppFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

bool EncryptedChatApp::OnInit()
{
    sodium_init();
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
