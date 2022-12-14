// This project was started 28.12.2020 with name "Device money consumption".
// It uses dialog box as its main window. So all message processing is in dialogMainProcedure()

// DEPENDENCIES:
// Help v1.0.3
// Version History v1.0.3
// av.dll v1.3.5

#include <windows.h>
#include <CommCtrl.h>
#include <string>
#include <dwmapi.h>
#include <HtmlHelp.h>
#include <Shlwapi.h> // for stripping paths
#include <regex>

#include <Tests.h>
#include <av.h>
#include "../res/rc files/resource.h"
#include "V:/0010/activeProjects/Visual Studio/_Avlibs/Currency names/Currency Name Strings.h"

#include <EditThings.h>
#include <LoadSave.h>

struct OldProcedure
{
    HWND controlWND;
    WNDPROC procedure;

    static std::vector <OldProcedure> procedures;
    static const OldProcedure *find(HWND wnd)
    {
        for(int i=0; i < procedures.size(); i++)
            if(wnd == procedures.at(i).controlWND)
                return &procedures.at(i);
    }
};
std::vector <OldProcedure> OldProcedure::procedures;

// ### global variables
AvTitleBtn onTopBtn;
AvTitleBtn toTrayBtn;
AvTrayIcon trayIcon; 
HINSTANCE hInst = GetModuleHandle(0);
HWND mainWnd;
HWND spinWattUsage;
HWND spinHours;
HWND spinMinutes;
HWND spinPrice;
HWND spinDaysInUse;
HWND editPrice;
HWND comboCurrencies;
HWND statCurrSymbol;
HWND statCurrSymbol2nd;
HFONT fontCurrency; // font for controls where currency symbol is used
WINDOWPLACEMENT windowPlacement; 
std::wstring companyName;
std::wstring appName;
std::wstring productVersion;
std::wstring releaseDate = L"11.09.2021";
HICON appIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPICON));
bool programmaticCall = false;

// ### function declarations / definitions
BOOL CALLBACK dialogMainProcedure (HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
BOOL CALLBACK dialogAboutProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK newEditProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
void fillComboCurrencies();
void fillAboutDlgInfo(HWND dlg);
void onOperandChange(int32_t editID);
void recalculateTotal();  
void setChildHWNDs(HWND hwnd);
void setControlsFont(); 
void setGlobalVariables(HWND hwnd);
void setAppAppearance();
void setDefaultCurrency();
void setMinMaxParams();
void onHelp();
void onComboCurrency(WPARAM wparam, LPARAM lparam);
void setCurrency(const wchar_t *countryISO);
int getFloatPrecision(cwstr numberStr);
// # message processors
void onInitDialogMsg(HWND);
void onUpDown(uint updownID, bool incrementing);
void onExit();
void onMenuAbout();
void onEndSession(WPARAM, LPARAM);
void onCommand(WPARAM, LPARAM);
void onNotify(WPARAM, LPARAM);
INT_PTR onCtlColorStatic(WPARAM, LPARAM);
void toggleTopmostStyle();
void hideToTray();
void onWindowPosChanged();
void parseEditText(int editID);

// ### application main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, char* args, int nCmdShow)
{
    mainWnd = CreateDialogW(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), 0,
                         dialogMainProcedure);
    
    HACCEL accelerators = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
    MSG message = {0};
    while(GetMessage(&message, 0, 0, 0))
    {
        if(!TranslateAcceleratorW(mainWnd, accelerators, &message)
        && !IsDialogMessageW(mainWnd, &message)) 
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    return 0;
}

HMODULE loadAvDll()
{
    std::wstring resourcePath;
#if _DEBUG
    resourcePath = L"V:\\0010\\Archive\\Exes, libs\\AvtemLibs\\av\\Version 1.3.5\\avD.dll";
#else
    resourcePath = L"V:\\0010\\Archive\\Exes, libs\\AvtemLibs\\av\\Version 1.3.5\\av.dll";
#endif
    
    return LoadLibrary(resourcePath.c_str());
}

BOOL CALLBACK dialogMainProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch(message)
    {
        case WM_INITDIALOG:             onInitDialogMsg(hwnd);          return true;
        case WM_NOTIFY:                 onNotify(wparam, lparam);       return true;
        case WM_ENDSESSION:             onEndSession(wparam, lparam);   
        case WM_CLOSE:                  onExit();                       return 0; 
        case WM_COMMAND:                onCommand(wparam, lparam);      return 0;
        case WM_WINDOWPOSCHANGED:       onWindowPosChanged();           return 0;
        case WM_CTLCOLORSTATIC:         return onCtlColorStatic(wparam, lparam);
        case WM_CTLCOLORDLG:            return (INT_PTR) GetStockObject(LTGRAY_BRUSH);
    }
    return 0;   // if we don't process a message, we have to return "false"
}

BOOL CALLBACK dialogAboutProcedure(HWND dlgAbout, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch(message)
    {
        case WM_INITDIALOG:
        {
            fillAboutDlgInfo(dlgAbout);
            return true;
        }
        case WM_COMMAND:
        {
            switch(LOWORD(wparam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(dlgAbout, 0);
                    return true;
            }
            return true;
        }
    }
    return false;
}

LRESULT CALLBACK newEditProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
        case WM_KILLFOCUS:
            if(av::getEditText(hwnd).empty() || av::getEditText(hwnd) == L".")
                SetDlgItemText(mainWnd, GetDlgCtrlID(hwnd), L"0");
            break;
    }

    return CallWindowProc(OldProcedure::find(hwnd)->procedure, 
                          hwnd, message, wparam, lparam);
}

void setMinMaxParams()
{
    SendMessage(spinWattUsage, UDM_SETRANGE32, 0, 999999);
    SendMessage(spinHours    , UDM_SETRANGE32, 0, 24);
    SendMessage(spinMinutes  , UDM_SETRANGE32, 0, 59);
    SendMessage(spinPrice    , UDM_SETRANGE32, 0, 999999);
    SendMessage(spinDaysInUse, UDM_SETRANGE32, 0, 999999);
}

void onHelp()
{
    // try "avApp Archive" first!
    std::wstring helpPath = L"V:\\0010\\Archive\\Resource files\\Device money consumption\\Help\\Version 1.0.2\\Device money consumption - Help.chm";
    
    if(!PathFileExists(helpPath.c_str()))  
       helpPath = av::getExeDir() + L"\\Device money consumption - Help.chm";

    av::tryToOpenCHM(helpPath.c_str(), mainWnd);
}

void onComboCurrency(WPARAM wparam, LPARAM lparam)
{
    uint selectedIndex = SendMessage((HWND) lparam, CB_GETCURSEL, 0, 0);
    currencies.at(selectedIndex);

    if (HIWORD(wparam) == CBN_SELCHANGE
        && selectedIndex >= 0
        && selectedIndex  < currencies.size())
    {
        SetWindowTextW(statCurrSymbol, currencies.at(selectedIndex).symbol);
        SetWindowTextW(statCurrSymbol2nd, currencies.at(selectedIndex).symbol);
    }
}


void onVersionHistory()
{
    // try "avApp Archive" first!
    std::wstring helpPath = L"V:\\0010\\Archive\\Resource files\\Device money consumption\\Version history\\Version 1.0.3\\Version history - Device money consumption.chm";

    if(!PathFileExists(helpPath.c_str()))  
       helpPath = av::getExeDir() + L"\\Version history - Device money consumption.chm";

    av::tryToOpenCHM(helpPath.c_str(), mainWnd);
}

void setCurrency(const wchar_t *countryISO)
{
    // get corresponding currency symbol
    int indexInVec;
    CurrencyEntry *currency = CurrencyEntry::find(&currencies, countryISO);
    if(currency)
        indexInVec = currency->getIndex(&currencies);
    else // we haven't found it? Set US currency symbol then
        indexInVec = CurrencyEntry::find(&currencies, L"US")->getIndex(&currencies);
    
    // set values for controls in dialog box
    SendMessage(comboCurrencies, CB_SETCURSEL, indexInVec, 0);

    SetDlgItemText(mainWnd, IDC_STATIC_CURRENCY,    currencies.at(indexInVec).symbol);
    SetDlgItemText(mainWnd, IDC_STATIC_CURRENCY2ND, currencies.at(indexInVec).symbol);
}

void setChildHWNDs(HWND hwnd)
{
    spinWattUsage     = GetDlgItem(hwnd, IDC_SPIN_WATT_USAGE);
    spinHours         = GetDlgItem(hwnd, IDC_SPIN_HOURS);
    spinMinutes       = GetDlgItem(hwnd, IDC_SPIN_MINUTES);
    spinPrice         = GetDlgItem(hwnd, IDC_SPIN_PRICE);
    spinDaysInUse     = GetDlgItem(hwnd, IDC_SPIN_DAYS_IN_USE);
    comboCurrencies   = GetDlgItem(hwnd, IDC_COMBO_CURRENCY);
    statCurrSymbol    = GetDlgItem(hwnd, IDC_STATIC_CURRENCY);
    statCurrSymbol2nd = GetDlgItem(hwnd, IDC_STATIC_CURRENCY2ND);
    editPrice         = GetDlgItem(hwnd, IDC_EDIT_PRICE);
}

void setControlsFont()
{
    // avTodo: find font that supports all currency symbols
    return;

    fontCurrency = CreateFont(0,0,0,0,0,0,0,0,0,0,0,0,0,L"Arial");

    SendMessage(comboCurrencies  , WM_SETFONT, (WPARAM) fontCurrency, 0);
    SendMessage(statCurrSymbol   , WM_SETFONT, (WPARAM) fontCurrency, 0);
    SendMessage(statCurrSymbol2nd, WM_SETFONT, (WPARAM) fontCurrency, 0);
}

void setGlobalVariables(HWND hwnd)
{
    mainWnd = hwnd;
    
    AvVersionInfo versionInfo;
    versionInfo.getInfo();
    
    appName = versionInfo.productName;
    companyName = versionInfo.companyName;
    productVersion = versionInfo.productVersion;

    // subclass all 5 edits
    for(int i=0; i < 5; i++)    
    {
        HWND wnd = 0;
        switch (i)
        {
            case 0:   wnd = GetDlgItem(hwnd, IDC_EDIT_HOURS);           break;
            case 1:   wnd = GetDlgItem(hwnd, IDC_EDIT_MINUTES);         break;
            case 2:   wnd = GetDlgItem(hwnd, IDC_EDIT_WATT_USAGE);      break;
            case 3:   wnd = GetDlgItem(hwnd, IDC_EDIT_DAYS_IN_USE);     break;
            case 4:   wnd = GetDlgItem(hwnd, IDC_EDIT_PRICE);           break;
        }

        OldProcedure::procedures.push_back
            ({wnd, (WNDPROC)SetWindowLongW(wnd, GWL_WNDPROC, (LONG)newEditProcedure)});
    }
}

void setAppAppearance()
{
    SetWindowTextW(mainWnd, appName.c_str());
    SetClassLong(mainWnd, GCL_HBRBACKGROUND, (LPARAM) GetStockObject(LTGRAY_BRUSH));
    SendMessage(mainWnd, WM_SETICON, ICON_BIG  , (LPARAM) appIcon);

    setChildHWNDs(mainWnd);
    av::setWndHeight(comboCurrencies, 150); // RESIZE TO FIT DROPDOWN LIST
    setControlsFont();
}

void fillAboutDlgInfo(HWND dlg)
{
    SetDlgItemTextW(dlg, ID_APPNAME, appName.c_str());
    SetDlgItemTextW(dlg, ID_BY,      std::wstring(L"Created by: " + companyName).c_str());
    SetDlgItemTextW(dlg, ID_VERSION, std::wstring(L"Version: "    +productVersion).c_str());
    SetDlgItemTextW(dlg, ID_RELEASED, releaseDate.c_str());

    SetWindowTextW(dlg, (L"About " + appName).c_str());
}

void onExit()
{
    HWND focusedWnd = GetFocus();
    if(av::getClassName(focusedWnd) == L"Edit"
    && (av::getEditText(focusedWnd).empty() || av::getEditText(focusedWnd) == L"."))
        SetDlgItemText(mainWnd, GetDlgCtrlID(focusedWnd), L"0");

    saveAllSettings(mainWnd, trayIcon);
    DeleteObject(fontCurrency);

    DestroyWindow(mainWnd);
    PostQuitMessage(0);
}

void onMenuAbout()
{
    DialogBoxW(hInst, MAKEINTRESOURCE(IDD_DIALOG_ABOUT), 
               mainWnd, dialogAboutProcedure);
}

void onEndSession(WPARAM wparam, LPARAM lparam)
{
    if(wparam   // wparam is "true" when user ends session Forcibly
       && (lparam & ENDSESSION_CRITICAL) == false)
    {
        onExit();
    }
}

void onCommand(WPARAM wparam, LPARAM lparam)
{
    const int32_t controlID = LOWORD(wparam);

    switch (controlID)
    {
        case IDC_EDIT_HOURS:
        case IDC_EDIT_MINUTES:
        case IDC_EDIT_PRICE:
        case IDC_EDIT_WATT_USAGE:
        case IDC_EDIT_DAYS_IN_USE:
        {
            if (!programmaticCall && HIWORD(wparam) == EN_UPDATE)
                onOperandChange(controlID);
        }
            break;

        case IDC_COMBO_CURRENCY:    onComboCurrency(wparam, lparam);    break;
        case ID_HELP_ABOUT:                     onMenuAbout();          break;
        case ID_HELP_HOWTOUSETHISPROGRAM:       onHelp();               break;
        case ID_VERSION:                        onVersionHistory();     break;
        case ID_FILE_EXIT:                      onExit();               break;
    }
}

void onNotify(WPARAM, LPARAM lparam)
{
     LPNMUPDOWN info = (LPNMUPDOWN) lparam;
     onUpDown(info->hdr.idFrom, info->iDelta > 0);
}

INT_PTR onCtlColorStatic(WPARAM wparam, LPARAM lparam)
{
    // default behaviour!
    if (av::getClassName((HWND) lparam) == L"Edit")
        return (INT_PTR)DefWindowProc(HWND(lparam), WM_CTLCOLORSTATIC, wparam, lparam);

    // a bit of nice looking background
    SetBkMode((HDC) wparam, TRANSPARENT);
    return (INT_PTR) GetStockObject(LTGRAY_BRUSH);
}

void toggleTopmostStyle()
{
    av::toggleTopMostStyle(mainWnd);
}

void hideToTray()
{
    ShowWindow(mainWnd, SW_HIDE);  // HIDE MAIN WINDOW
    trayIcon.show(); 
}

void onWindowPosChanged()
{
    onTopBtn.setIcon(av::isTopMost(mainWnd) ? (HICON) AvTitleBtn::PinRed
                                            : (HICON) AvTitleBtn::PinGray);
}

void parseEditText(int editID)
{
    static std::wstring replaceWith = L"";  // delete EVERY MATCH
    static std::wregex needle;
    static std::wstring haystack;

    // delete any "text" characters
    haystack = getEditText(editID);
    needle = L"[^\\d.,]";
    haystack = std::regex_replace(haystack, needle, replaceWith);

    switch (editID)
    {
        case IDC_EDIT_WATT_USAGE:
        case IDC_EDIT_DAYS_IN_USE:
        case IDC_EDIT_HOURS:
        case IDC_EDIT_MINUTES:
        {
            needle = L"[.,]";
            haystack = std::regex_replace(haystack, needle, replaceWith);

            if (!editHasValidValue(editID))
                haystack = av::numToStr((int)getCorrectEditValue(editID));

            haystack = std::regex_replace(haystack, needle, replaceWith);
        }
            break;
        case IDC_EDIT_PRICE:
        {          
            std::wcmatch matches;
            needle = L"[.,]";
            
            // leave only first dot
            int dotPos = -1;
            std::regex_search(haystack.c_str(), matches, needle);
            haystack = std::regex_replace(haystack, needle, replaceWith);
            if(matches.size())
            {
                dotPos = matches.position(0);
                haystack.insert(dotPos, L".");
            }
        }
            break;
    }
    
    setEditText(editID, haystack.c_str());
}

void fillComboCurrencies()
{
    for (uint i = 0; i < currencies.size(); i++) {
        std::wstring currencyStr = currencies.at(i).country
            + std::wstring(L" - ")
            + std::wstring(currencies.at(i).currency)
            + std::wstring(L": ")
            + std::wstring(currencies.at(i).symbol);

        SendMessage(comboCurrencies, CB_ADDSTRING, 0, (LPARAM) currencyStr.c_str());
    }
}

void setDefaultCurrency()
{
    // get current user locale
    GEOID geoID = GetUserGeoID(GEOCLASS_NATION);
    if(geoID == GEOID_NOT_AVAILABLE)
        geoID = 244; // United States
    wchar_t isoCode[3];
    GetGeoInfoW(geoID, GEO_ISO2, isoCode, 3, 0);

    setCurrency(isoCode);
}

void recalculateTotal()
{
  // get values from edits
    float hours     = getEditFloat(IDC_EDIT_HOURS);
    float minutes   = (hours != 24) ? getEditFloat(IDC_EDIT_MINUTES) : 0;
    float watts     = getEditFloat(IDC_EDIT_WATT_USAGE);
    float price     = getEditFloat(IDC_EDIT_PRICE) / 1000.0f;
    float daysInUse = getEditFloat(IDC_EDIT_DAYS_IN_USE);

    float hoursTotal = (daysInUse * (hours*60.0f +minutes)) /60.0f;
    float totalCost = hoursTotal * watts *price; // so, that's the main result!

    setEditFloat(IDC_EDIT_TOTAL, totalCost, 2);
}

void onInitDialogMsg(HWND hwnd)
{
    toTrayBtn.create(hwnd, hInst, 1, hideToTray, (HICON) AvTitleBtn::ToTray);
    onTopBtn.create(hwnd, hInst, 2, toggleTopmostStyle, 0);
    trayIcon.create(hwnd, appName.c_str(), appIcon, AvTrayIcon::DefaultMenu, 0);

    setGlobalVariables(hwnd);
    
    setAppAppearance();
    setMinMaxParams();
    fillComboCurrencies();

    loadAllSettings(hwnd, trayIcon);

#ifdef _DEBUG
    // avDis: tesss
    //UnitTest unitTest;
    //unitTest.runAllTests();
#endif
}

void onUpDown(uint updownID, bool incrementing)
{
    if(updownID != IDC_SPIN_PRICE)
        return;

    updownID -= 1; // EDIT ids are less by one than SPINs

    int floatPrecision = getFloatPrecision(getEditText(updownID).c_str());
    float incrementValue = powf(10, -floatPrecision) *(incrementing ? 1 : -1);

    float newValue = getEditFloat(updownID) +incrementValue;
    if(newValue < 0)
        return;

    setEditFloat(updownID, newValue, floatPrecision);    
}

int getFloatPrecision(cwstr numberStr)
{
    int digitsAfter = 0; // digit count after comma
    // count
    for(int i = lstrlenW(numberStr) -1; i >= 0; i--)  // lstrlenA does not include NULL character
    {
        if(numberStr[i] == '.')
            return digitsAfter;
        digitsAfter ++;
    }

    return 0; // no '.' was found!
}

void onOperandChange(int32_t editID)
{    
    programmaticCall = true;
    
    parseEditText(editID);
    recalculateTotal();
    
    programmaticCall = false;
}