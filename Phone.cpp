//---------------------------------------------------------------------------

#define _EXPORTING
#include "..\tSIP\tSIP\phone\Phone.h"
#include "..\tSIP\tSIP\phone\PhoneSettings.h"
#include "..\tSIP\tSIP\phone\PhoneCapabilities.h"
#include "Log.h"
#include <assert.h>
#include <algorithm>	// needed by Utils::in_group
#include "Utils.h"
#include <string>


//---------------------------------------------------------------------------

static const struct S_PHONE_DLL_INTERFACE dll_interface =
{DLL_INTERFACE_MAJOR_VERSION, DLL_INTERFACE_MINOR_VERSION};

// callback ptrs
CALLBACK_LOG lpLogFn = NULL;
CALLBACK_CONNECT lpConnectFn = NULL;
CALLBACK_KEY lpKeyFn = NULL;


void *callbackCookie;	///< used by upper class to distinguish library instances when receiving callbacks

namespace {

}


extern "C" __declspec(dllexport) void GetPhoneInterfaceDescription(struct S_PHONE_DLL_INTERFACE* interf) {
    interf->majorVersion = dll_interface.majorVersion;
    interf->minorVersion = dll_interface.minorVersion;
}

void Log(char* txt) {
    if (lpLogFn)
        lpLogFn(callbackCookie, txt);
}

void Connect(int state, char *szMsgText) {
    if (lpConnectFn)
        lpConnectFn(callbackCookie, state, szMsgText);
}

void Key(int keyCode, int state) {
    if (lpKeyFn)
        lpKeyFn(callbackCookie, keyCode, state);
}

void SetCallbacks(void *cookie, CALLBACK_LOG lpLog, CALLBACK_CONNECT lpConnect, CALLBACK_KEY lpKey) {
    assert(cookie && lpLog && lpConnect && lpKey);
    lpLogFn = lpLog;
    lpConnectFn = lpConnect;
    lpKeyFn = lpKey;
    callbackCookie = cookie;
    lpLogFn(callbackCookie, "Phone DLL for EX03 loaded\n");

    //armScope.callbackLog = Log;
    //armScope.callbackConnect = Connect;
}

void GetPhoneCapabilities(struct S_PHONE_CAPABILITIES **caps) {
    static struct S_PHONE_CAPABILITIES capabilities = {
        0
    };
    *caps = &capabilities;
}

void ShowSettings(HANDLE parent) {
    MessageBox((HWND)parent, "No additional settings.", "Device DLL", MB_ICONINFORMATION);
}

int Connect(void) {
    return 0;
}

int Disconnect(void) {
    return 0;
}

static bool bSettingsReaded = false;

static int GetDefaultSettings(struct S_PHONE_SETTINGS* settings) {
    settings->ring = 1;

    bSettingsReaded = true;
    return 0;
}

int GetPhoneSettings(struct S_PHONE_SETTINGS* settings) {
    //settings->iTriggerSrcChannel = 0;

    std::string path = Utils::GetDllPath();
    path = Utils::ReplaceFileExtension(path, ".cfg");
    if (path == "")
        return GetDefaultSettings(settings);


    GetDefaultSettings(settings);


    //int mode = root.get("TriggerMode", TRIGGER_MODE_AUTO).asInt();
    settings->ring = true;//root.get("ring", settings->ring).asInt();


    bSettingsReaded = true;
    return 0;
}

int SavePhoneSettings(struct S_PHONE_SETTINGS* settings) {

    return 0;
}

int SetRegistrationState(int state) {
    return 0;
}

int SetCallState(int state, const char* display) {
    return 0;
}

bool connected = false;
bool exited = true;

DWORD WINAPI RingThreadProc(LPVOID data) {
    unsigned int loopCnt = 0;

    while (connected) {
        if (loopCnt == 0 || loopCnt == 50) {
            Beep(300, 300);
        }
        loopCnt++;
        if (loopCnt > 300) {
            loopCnt = 0;
        }
        Sleep(10);
    }


    exited = true;
    return 0;
}


int RingThreadStart(void) {
    DWORD dwtid;
    exited = false;
    connected = true;
    HANDLE RingThread = CreateThread(NULL, 0, RingThreadProc, /*this*/NULL, 0, &dwtid);
    if (RingThread == NULL) {
        connected = false;
        exited = true;
    }

    return 0;
}

int RingThreadStop(void) {
    connected = false;
    while (!exited) {
        Sleep(50);
    }
    return 0;
}

int Ring(int state) {
    //MessageBox(NULL, "Ring", "Device DLL", MB_ICONINFORMATION);
    if (state) {
        if (connected == false) {
            RingThreadStart();
        }
    } else {
        RingThreadStop();
    }
    return 0;
}
