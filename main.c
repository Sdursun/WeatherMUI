#include <libraries/mui.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <stdio.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
// #include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <weather.h>
#include <httpgetlib.h>
#include <funcs.h>

BOOL Open_Libs(void)
{
    if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 39)))
    {
        return (0);
    }

    if (!(GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0)))
    {
        CloseLibrary((struct Library *)IntuitionBase);
        return (0);
    }

    if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME, 19)))
    {
        CloseLibrary((struct Library *)GfxBase);
        CloseLibrary((struct Library *)IntuitionBase);
        return (0);
    }

    return (1);
}

void Close_Libs(void)
{
    if (IntuitionBase)
    {
        CloseLibrary((struct Library *)IntuitionBase);
    }

    if (GfxBase)
    {
        CloseLibrary((struct Library *)GfxBase);
    }
    if (MUIMasterBase)
    {
        CloseLibrary(MUIMasterBase);
    }
}

LONG xget(Object *obj, ULONG attribute)
{
    LONG x;
    get(obj, attribute, &x);
    return (x);
}

char *getstr(Object *obj)
{
    return ((char *)xget(obj, MUIA_String_Contents));
}

//**********************************************************************************************************************

#define START 44
// #define DUR        45
#define BUFFER 0x0400
char **wdata;

char weatherURL[BUFFER / 2] = "";
char wf[50] = "RAM:T/weathermui.json";
char weatherText[BUFFER] = "";
int indx;

char location[BUFFER / 8] = "Antalya,tr";
char unit[10] = "metric";
char lang[16] = "tr, Turkish";
char slang[16];
char tmplocation[BUFFER / 8]; //??????????????
char apikey[BUFFER / 16] = "e45eeea40d4a9754ef176588dd068f18";

char cMessage[100];


#define IPTR ULONG

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *MUIMasterBase;

enum
{
    MENU_PROJECT = 1,
    MENU_ABOUT,
    MENU_QUIT,
};

static struct NewMenu MenuStr[] =
    {
        {NM_TITLE, "Project", 0, 0, 0, (APTR)MENU_PROJECT},
        {NM_ITEM, "About", "?", 0, 0, (APTR)MENU_ABOUT},
        {NM_ITEM, NM_BARLABEL, 0, 0, 0, (APTR)0},
        {NM_ITEM, "Quit", "Q", 0, 0, (APTR)MENU_QUIT},
        {NM_END, NULL, 0, 0, 0, (APTR)0},
};

char about_text[] =
"\33cWeatherMUI!\n © 2024 Serkan DURSUN \n version 1.0 (09/01/2024)\n This program is Open Source.\n Mail: blasterreal@gmail.com\n Github: https://github.com/";


IPTR selectedCyc1ItemIndex = 0;

static const char *LocaleList[] =
    {
        "en, English",
        "tr, Turkish",
        "de, German",
        NULL};

int main(int argc, char **argv[])
{
    Object *app, *win1, *closeButton, *startButton;
    Object *apiKeyOjb, *apiKeyLabel, *apiKeyGroup;
    Object *apiUnitOjb, *apiUnitLabel, *apiUnitGroup;
    Object *apilocationObj, *apilocationLabel, *apilocationGroup;
    Object *apiLanguageCycle, *apiLanguageLabel, *apiLanguageGroup;
    Object *temperatureObj, *tempLabel, *mytempGroup;
    Object *tempStatusObj, *tempStatusLabel, *tempStatusGroup;
    Object *locationObj, *locationLabel, *locationGroup;

    ULONG signals;
    BOOL running = TRUE;

    APTR GROUP_ROOT_0;

    if (!Open_Libs())
    {
        printf("Cannot open libs\n");
        return (0);
    }

    // Display API key Object******************************************************
    apiKeyOjb = StringObject,
    MUIA_Frame, MUIV_Frame_String,
    MUIA_HelpNode, "STR_label_0",
    End;

    apiKeyLabel = Label2("ApiKey ");

    apiKeyGroup = GroupObject,
    MUIA_Group_Columns, 2,
    Child, apiKeyLabel,
    Child, apiKeyOjb,
    End;

    // Display Unit Object******************************************************
    apiUnitOjb = StringObject,
    MUIA_Frame, MUIV_Frame_String,
    MUIA_HelpNode, "STR_label_0",
    End;

    apiUnitLabel = Label2("Unit ");

    apiUnitGroup = GroupObject,
    MUIA_Group_Columns, 2,
    Child, apiUnitLabel,
    Child, apiUnitOjb,
    End;

    // Api Location Object******************************************************
    apilocationObj = StringObject,
    MUIA_Frame, MUIV_Frame_String,
    MUIA_HelpNode, "STR_label_0",
    End;

    apilocationLabel = Label2("Location");

    apilocationGroup = GroupObject,
    MUIA_Group_Columns, 2,
    Child, apilocationLabel,
    Child, apilocationObj,
    End;

    // Display Api Language Object******************************************************
    apiLanguageCycle = CycleObject,
    MUIA_HelpNode, "apiLanguageCycle",
    MUIA_Frame, MUIV_Frame_Button,
    MUIA_Cycle_Entries, LocaleList,
    End;

    apiLanguageLabel = Label2("Language ");

    apiLanguageGroup = GroupObject,
    MUIA_Group_Columns, 2,
    Child, apiLanguageLabel,
    Child, apiLanguageCycle,
    End;

    // Display Temperature Object******************************************************
    temperatureObj = TextObject,
    MUIA_Frame, MUIV_Frame_String,
    MUIA_HelpNode, "STR_label_0",
    End;

    tempLabel = Label2("Temperature ");

    mytempGroup = GroupObject,
    MUIA_Group_Columns, 2,
    Child, tempLabel,
    Child, temperatureObj,
    End;

    // Display Location Object*********************************************************
    locationObj = TextObject,
    MUIA_Frame, MUIV_Frame_String,
    MUIA_HelpNode, "STR_label_0",
    End;

    locationLabel = Label2("Location");

    locationGroup = GroupObject,
    MUIA_Group_Columns, 2,
    Child, locationLabel,
    Child, locationObj,
    End;

    // Display Temperature status Object***********************************************
    tempStatusObj = TextObject,
    MUIA_Frame, MUIV_Frame_String,
    MUIA_HelpNode, "STR_label_0",
    End;

    tempStatusLabel = Label2("Status ");

    tempStatusGroup = GroupObject,
    MUIA_Group_Columns, 2,
    Child, tempStatusLabel,
    Child, tempStatusObj,
    End;

    // Main object group****************************************************************
    GROUP_ROOT_0 = GroupObject,
    Child, apiKeyGroup,
    Child, apiUnitGroup,
    Child, apilocationGroup,
    Child, apiLanguageGroup,
    Child, startButton = MUI_MakeObject(MUIO_Button, "Get Temperature", NULL),
    Child, MUI_MakeObject(MUIO_HBar, 10),
    Child, locationGroup,
    Child, mytempGroup,
    Child, tempStatusGroup,
    Child, closeButton = MUI_MakeObject(MUIO_Button, "Quit", NULL),
    End;

    // Application
    MUIA_Application_Window, win1 = WindowObject,
                             MUIA_Window_Title, "WeatherMUI",
                             MUIA_Window_ID, MAKE_ID('S', 'E', 'R', 'K'),
                             MUIA_Window_Width, 480,
                             MUIA_Window_Height, 480,
                             WindowContents, GROUP_ROOT_0,
                             End;

    // App info
    app = ApplicationObject,
    MUIA_Application_Title, "WeatherMUI",
    MUIA_Application_Version, "$VER: WeatherMUI 1.0 (04.10.23)",
    MUIA_Application_Copyright, "Open Source",
    MUIA_Application_Author, " Serkan DURSUN",
    MUIA_Application_Description, "Weather Information",
    MUIA_Application_Base, " ",
    MUIA_Application_Menustrip, MUI_MakeObject(MUIO_MenustripNM, MenuStr, 0),
    SubWindow, win1,
    End;

    // If application success and than run!
    if (!app)
    {
        printf("Cannot create application.\n");
        return (0);
    }

    // DoMethot Area

    DoMethod(win1, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit); // Window close

    DoMethod(closeButton, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit); // Button close

    DoMethod(startButton, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, START); // Get temperature

    set(win1, MUIA_Window_Open, TRUE); // open Window
    set(apiKeyOjb, MUIA_String_Contents, apikey);
    set(apiUnitOjb, MUIA_String_Contents, unit);
    set(apilocationObj, MUIA_String_Contents, location);
    // set ( apiLanguageOjb, MUIA_String_Contents, lang );

    // strcpy(slang, lang);

    // get weather URL

    while (running)
    {
        ULONG id = DoMethod(app, MUIM_Application_Input, &signals);

        switch (id)
        {
        case MUIV_Application_ReturnID_Quit:
            running = FALSE;
            break;

        case START:
            get(apiLanguageCycle, MUIA_Cycle_Active, &selectedCyc1ItemIndex);
            strcpy(cMessage, LocaleList[selectedCyc1ItemIndex]);
            strcpy(slang, cMessage);
            slang[2] = '\0';

            strcpy(location, getstr(apilocationObj));

            strcpy(weatherURL, getURL(apikey, convertToUTF8(location), unit, slang));

            httpget(weatherURL, wf);

            strcpy(weatherText, getWeatherData(wf)); // 0 hava durumu, 1 Konum, 2 s�cakl�k, 3 celcius simgesi
            strcpy(weatherText, convertToLatin(weatherText));

            wdata = getArray(weatherText, "|", 4);

            indx = iconIndex(wdata[3]);                            // Celsius simgersi i�in kullan�l�yor
            strcpy(wdata[2], temperatureWithUnit(wdata[2], unit)); // Celcius simgesini ekler

            set(tempStatusObj, MUIA_Text_Contents, wdata[0]);
            set(locationObj, MUIA_Text_Contents, wdata[1]);
            set(temperatureObj, MUIA_Text_Contents, wdata[2]);

            // puts(wdata[2]);

            break;

        case MENU_ABOUT:
            MUI_RequestA(app,win1,0,"About WeatherMUI","*OK",about_text,NULL);
            break;

        case MENU_QUIT:
            running = FALSE;
            break;
        }

        if (running && signals)
        {
            Wait(signals);
        }
    }

    set(win1, MUIA_Window_Open, FALSE);

    if (app)
    {
        MUI_DisposeObject(app);
    }
    Close_Libs();
    exit(TRUE);
}
