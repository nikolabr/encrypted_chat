// encrypted_chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "crypto.hpp"
#include "net.hpp"
#include <wx/wx.h>
#include <wx/socket.h>

class User : public CryptographicUser {
public: 
    std::string name; 
};


User user1;
User user2;

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
    AppFrame(bool status, wxString hostname);
    wxTextCtrl* text_output;
    wxTextCtrl* text_input;
    wxSocketServer* sock;
    wxSocketBase* peer_sock;
    wxDECLARE_EVENT_TABLE();

private:
    void OnExit(wxCommandEvent& event);
    void UsernamePrompt(wxCommandEvent& event);
    void OnSend(wxCommandEvent& event);
    void OnServerEvent(wxSocketEvent& event);
    void OnSocketEvent(wxSocketEvent& event);

};

enum {
    ID_Output = 1,
    ID_Input = 2,
    ID_Socket = 3,
    ID_Server = 4,
};

void AppFrame::OnSend(wxCommandEvent& event) {
    std::string input = text_input->GetValue().utf8_string();
    text_output->WriteText(wxNow() << ' ' << "User 1" << ": " << input << '\n');
    /*TransportCipher cipher = user1.encrypt_message(input);
    user2.decrypt_message(cipher);
    text_output->WriteText(wxNow() << ' ' << "User 2" << ": " << cipher.data << '\n');*/
    if (peer_sock) {
        if (peer_sock->IsConnected()) {
            peer_sock->Write((input + '\n').c_str(), input.length());
        }
    }
    text_input->Clear();
}

void AppFrame::OnServerEvent(wxSocketEvent& event) {
    switch (event.GetSocketEvent()) {
    case wxSOCKET_CONNECTION:
        peer_sock = sock->Accept(false);
        if (peer_sock) {
            wxIPV4address addr;
            if (peer_sock->GetPeer(addr)) {
                text_output->WriteText(wxNow() << " New connection from " << addr.IPAddress() << '\n');
            }
        }
        peer_sock->SetEventHandler(*this, ID_Socket);
        peer_sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
        peer_sock->Notify(true);
        break;
    }
}

void AppFrame::OnSocketEvent(wxSocketEvent& event) {
    wxSocketBase* sockBase = event.GetSocket();
    wxUint32 lenRd;

    switch (event.GetSocketEvent()) {
    case wxSOCKET_INPUT:
        sockBase->SetNotify(wxSOCKET_LOST_FLAG);

        sockBase->SetFlags(wxSOCKET_WAITALL);

        char buf[16];
        lenRd = sockBase->Read(buf, 5).LastCount();

        if (lenRd) {
            text_output->WriteText(wxNow() << ' ' << "User 2: " << buf);
        }

        sockBase->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
        break;
    case wxSOCKET_LOST:
        sockBase->Destroy();
        break;
    }
}

AppFrame::AppFrame(bool status, wxString hostname) : wxFrame(nullptr, wxID_ANY, L"Encrypted Chat") {

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
    if (status)
        SetStatusText("Successfully generated keypair!");
    else
        SetStatusText("Failed to generate keypair!");

    wxIPV4address addr;

    addr.AnyAddress();
    if (hostname != "localhost")
        addr.Hostname(hostname);

    addr.Service(28015);
    
    sock = new wxSocketServer(addr);

    sock->SetEventHandler(*this, ID_Server);
    sock->SetNotify(wxSOCKET_CONNECTION_FLAG);
    sock->Notify(true);

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
    user1 = User();
    user2 = User();
    user1.name = "me";
    user2.name = "you";

    wxString addr;
    if (wxApp::argc > 1) {
        addr = wxApp::argv[0];
    }
    else {
        addr = "localhost";
    }
    AppFrame* mainFrame = new AppFrame(key_exchange(user1, user2), addr);

    mainFrame->Show(true);

    return true;
}

wxBEGIN_EVENT_TABLE(AppFrame, wxFrame)
EVT_SOCKET(ID_Server, AppFrame::OnServerEvent)
EVT_SOCKET(ID_Socket, AppFrame::OnSocketEvent)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(EncryptedChatApp);

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
