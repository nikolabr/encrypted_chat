// encrypted_chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "crypto.h"
#include <wx/wx.h>

class User : public CryptographicUser {
public: 
    std::string name; 

    User(std::string str) : name(str) {}
};

class EncryptedChatApp : public wxApp {
public: 
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
    AppFrame();

private:
    void OnExit(wxCommandEvent& event);
};


AppFrame::AppFrame() : wxFrame(nullptr, wxID_ANY, L"Encrypted Chat") {
    sodium_init();

    User alice("Alice");
    User bob("Bob");

    alice.keypair.print_keypair();
    bob.keypair.print_keypair();

    bool status = key_exchange(alice, bob);
    CreateStatusBar();
    if (status == true)
        SetStatusText("Successful key exchange!");
    else
        SetStatusText("Failed to exchange keys!");
}

void AppFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

bool EncryptedChatApp::OnInit()
{
    AppFrame* mainFrame = new AppFrame();
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
