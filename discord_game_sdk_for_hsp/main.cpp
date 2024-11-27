#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdexcept>
#include "hsp3plugin.h"
#include "discord.h"
#include "iconv.h"

/*------------------------------------------------------------*/

static int cmdfunc(int cmd)
{
    //      実行処理 (命令実行時に呼ばれます)
    //
    code_next();                            // 次のコードを取得(最初に必ず必要です)

    switch (cmd) {                         // サブコマンドごとの分岐
    default:
        puterror(HSPERR_UNSUPPORTED_FUNCTION);
    }
    return RUNMODE_RUN;
}


/*------------------------------------------------------------*/

static int ref_ival;                        // 返値のための変数

static void* reffunc(int* type_res, int cmd)
{
    //      関数・システム変数の実行処理 (値の参照時に呼ばれます)
    //
    //          '('で始まるかを調べる
    //
    if (*type != TYPE_MARK) puterror(HSPERR_INVALID_FUNCPARAM);
    if (*val != '(') puterror(HSPERR_INVALID_FUNCPARAM);
    code_next();


    switch (cmd) {                         // サブコマンドごとの分岐

    case 0x00:                              // newcmd

        p1 = code_geti();               // 整数値を取得(デフォルトなし)
        ref_ival = p1 * 2;              // 返値をivalに設定
        break;

    default:
        puterror(HSPERR_UNSUPPORTED_FUNCTION);
    }

    //          '('で終わるかを調べる
    //
    if (*type != TYPE_MARK) puterror(HSPERR_INVALID_FUNCPARAM);
    if (*val != ')') puterror(HSPERR_INVALID_FUNCPARAM);
    code_next();

    *type_res = HSPVAR_FLAG_INT;            // 返値のタイプを整数に指定する
    return (void*)&ref_ival;
}


/*------------------------------------------------------------*/

static int termfunc(int option)
{
    //      終了処理 (アプリケーション終了時に呼ばれます)
    //
    return 0;
}

/*------------------------------------------------------------*/

static int eventfunc(int event, int prm1, int prm2, void* prm3)
{
    //      イベント処理 (HSPイベント発生時に呼ばれます)
    //
    switch (event) {
    case HSPEVENT_GETKEY:
    {
        int* ival;
        ival = (int*)prm3;
        *ival = 123;
        return 1;
    }
    }
    return 0;
}

/*------------------------------------------------------------*/
/*
        interface
*/
/*------------------------------------------------------------*/

int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
    //      DLLエントリー (何もする必要はありません)
    //
    return TRUE;
}


EXPORT void WINAPI hsp3cmdinit(HSP3TYPEINFO* info)
{
    //      プラグイン初期化 (実行・終了処理を登録します)
    //
    hsp3sdk_init(info);           // SDKの初期化(最初に行なって下さい)
    info->cmdfunc = cmdfunc;        // 実行関数(cmdfunc)の登録
    info->reffunc = reffunc;        // 参照関数(reffunc)の登録
    info->termfunc = termfunc;      // 終了関数(termfunc)の登録

    /*
    //  イベントコールバックを発生させるイベント種別を設定する
    info->option = HSPEVENT_ENABLE_GETKEY;
    info->eventfunc = eventfunc;    // イベント関数(eventfunc)の登録
    */
}
/*------------------------------------------------------------*/
/*
        以下メイン
*/
/*------------------------------------------------------------*/

#define EXPORT extern "C" __declspec (dllexport)

std::string convertSJIStoUTF8(const std::string& shift_jis_str) {
    iconv_t cd = iconv_open("UTF-8", "SHIFT-JIS");
    if (cd == (iconv_t)-1) {
        throw std::runtime_error("iconv_open failed");
    }

    size_t in_bytes_left = shift_jis_str.size();
    size_t out_bytes_left = in_bytes_left * 2;
    char* out_buf = new char[out_bytes_left];
    const char* in_buf = shift_jis_str.c_str();

    char* out_ptr = out_buf;
    if (iconv(cd, const_cast<char**>(&in_buf), &in_bytes_left, &out_ptr, &out_bytes_left) == (size_t)-1) {
        delete[] out_buf;
        iconv_close(cd);
        throw std::runtime_error("iconv failed");
    }

    std::string utf8_str(out_buf, out_ptr - out_buf);

    delete[] out_buf;
    iconv_close(cd);

    return utf8_str;
}

int resultToInt(discord::Result result) {
    switch (result) {
    case discord::Result::Ok:
        return 0;
    case discord::Result::ServiceUnavailable:
        return 1;
    case discord::Result::InvalidVersion:
        return 2;
    case discord::Result::LockFailed:
        return 3;
    case discord::Result::InternalError:
        return 4;
    case discord::Result::InvalidPayload:
        return 5;
    case discord::Result::InvalidCommand:
        return 6;
    case discord::Result::InvalidPermissions:
        return 7;
    case discord::Result::NotFetched:
        return 8;
    case discord::Result::NotFound:
        return 9;
    case discord::Result::Conflict:
        return 10;
    case discord::Result::InvalidSecret:
        return 11;
    case discord::Result::InvalidJoinSecret:
        return 12;
    case discord::Result::NoEligibleActivity:
        return 13;
    case discord::Result::InvalidInvite:
        return 14;
    case discord::Result::NotAuthenticated:
        return 15;
    case discord::Result::InvalidAccessToken:
        return 16;
    case discord::Result::ApplicationMismatch:
        return 17;
    case discord::Result::InvalidDataUrl:
        return 18;
    case discord::Result::InvalidBase64:
        return 19;
    case discord::Result::NotFiltered:
        return 20;
    case discord::Result::LobbyFull:
        return 21;
    case discord::Result::InvalidLobbySecret:
        return 22;
    case discord::Result::InvalidFilename:
        return 23;
    case discord::Result::InvalidFileSize:
        return 24;
    case discord::Result::InvalidEntitlement:
        return 25;
    case discord::Result::NotInstalled:
        return 26;
    case discord::Result::NotRunning:
        return 27;
    case discord::Result::InsufficientBuffer:
        return 28;
    case discord::Result::PurchaseCanceled:
        return 29;
    case discord::Result::InvalidGuild:
        return 30;
    case discord::Result::InvalidEvent:
        return 31;
    case discord::Result::InvalidChannel:
        return 32;
    case discord::Result::InvalidOrigin:
        return 33;
    case discord::Result::RateLimited:
        return 34;
    case discord::Result::OAuth2Error:
        return 35;
    case discord::Result::SelectChannelTimeout:
        return 36;
    case discord::Result::GetGuildTimeout:
        return 37;
    case discord::Result::SelectVoiceForceRequired:
        return 38;
    case discord::Result::CaptureShortcutAlreadyListening:
        return 39;
    case discord::Result::UnauthorizedForAchievement:
        return 40;
    case discord::Result::InvalidGiftCode:
        return 41;
    case discord::Result::PurchaseError:
        return 42;
    case discord::Result::TransactionAborted:
        return 43;
    case discord::Result::DrawingInitFailed:
        return 44;
    default:
        return -1;
    }
}

// Variables
discord::Core* core{};
discord::Activity activity{};
unsigned short* onActivityJoinLabel;

// Initialize
bool WINAPI DiscordInitialize(HSPEXINFO* hei)
{
    char* __clientId = hei->HspFunc_prm_gets();
    std::string _clientId = __clientId;
    int64_t clientId = std::stoll(_clientId);

    discord::Result result = discord::Core::Create(clientId, DiscordCreateFlags_Default, &core);
    if (!core || result != discord::Result::Ok) {
        hei->hspctx->stat = resultToInt(result);
        return 0;
    }

    hei->hspctx->stat = resultToInt(result);
    return 0;
}

// Run callbacks
bool WINAPI DiscordUpdate(HSPEXINFO* hei)
{
    if (!core) {
        hei->hspctx->stat = -1;
        return 0;
    }

    discord::Result result = core->RunCallbacks();
    hei->hspctx->stat = resultToInt(result);
    return 0;
}

//Activity
bool WINAPI DiscordActivitySetState(HSPEXINFO* hei)
{
    char* _state = hei->HspFunc_prm_getns();
    std::string state = _state;
    if (hei->hspctx->hspstat & HSPSTAT_UTF8) {
        activity.SetState(state.c_str());
    }
    else {
        std::string utf8State = convertSJIStoUTF8(state);
        activity.SetState(utf8State.c_str());
    }
    return 0;
}

bool WINAPI DiscordActivitySetDetails(HSPEXINFO* hei)
{
    char* _details = hei->HspFunc_prm_getns();
    std::string details = _details;
    if (hei->hspctx->hspstat & HSPSTAT_UTF8) {
        activity.SetDetails(details.c_str());
    }
    else {
        std::string utf8Details = convertSJIStoUTF8(details);
        activity.SetDetails(utf8Details.c_str());
    }

    return 0;
}

bool WINAPI DiscordActivitySetName(HSPEXINFO* hei)
{
    char* _name = hei->HspFunc_prm_getns();
    std::string name = _name;
    if (hei->hspctx->hspstat & HSPSTAT_UTF8) {
        activity.SetName(name.c_str());
    }
    else {
        std::string utf8Name = convertSJIStoUTF8(name);
        activity.SetName(utf8Name.c_str());
    }
    return 0;
}

bool WINAPI DiscordActivitySetType(HSPEXINFO* hei)
{
    int _type = hei->HspFunc_prm_geti();
    if (_type == 0) {
        activity.SetType(discord::ActivityType::Playing);
    }
    else if (_type == 1) {
        activity.SetType(discord::ActivityType::Streaming);
    }
    else if (_type == 2) {
        activity.SetType(discord::ActivityType::Listening);
    }
    else if (_type == 3) {
        activity.SetType(discord::ActivityType::Watching);
    }
    else {
        puterror(HSPERROR::HSPERR_INVALID_PARAMETER);
    }
    return 0;
}

bool WINAPI DiscordActivitySetInstance(HSPEXINFO* hei)
{
    int instance = hei->HspFunc_prm_geti();
    activity.SetInstance(instance);
    return 0;
}

bool WINAPI DiscordActivitySetSupportedPlatforms(HSPEXINFO* hei)
{
    int platforms = hei->HspFunc_prm_geti();
    activity.SetSupportedPlatforms(platforms);
    return 0;
}

bool WINAPI DiscordActivitySetApplicationId(HSPEXINFO* hei)
{
    int applicationId = hei->HspFunc_prm_geti();
    activity.SetApplicationId(applicationId);
    return 0;
}

// Assets
bool WINAPI DiscordActivitySetSmallImage(HSPEXINFO* hei)
{
    char* _smallImage = hei->HspFunc_prm_getns();
    std::string smallImage = _smallImage;
    if (hei->hspctx->hspstat & HSPSTAT_UTF8) {
        activity.GetAssets().SetSmallImage(smallImage.c_str());
    }
    else {
        std::string utf8SmallImage = convertSJIStoUTF8(smallImage);
        activity.GetAssets().SetSmallImage(utf8SmallImage.c_str());
    }
    return 0;
}

bool WINAPI DiscordActivitySetSmallText(HSPEXINFO* hei)
{
    char* _smallText = hei->HspFunc_prm_getns();
    std::string smallText = _smallText;
    if (hei->hspctx->hspstat & HSPSTAT_UTF8) {
        activity.GetAssets().SetSmallText(smallText.c_str());
    }
    else {
        std::string utf8SmallImage = convertSJIStoUTF8(smallText);
        activity.GetAssets().SetSmallText(utf8SmallImage.c_str());
    }
    return 0;
}

bool WINAPI DiscordActivitySetLargeImage(HSPEXINFO* hei)
{
    char* _largeImage = hei->HspFunc_prm_getns();
    std::string largeImage = _largeImage;
    if (hei->hspctx->hspstat & HSPSTAT_UTF8) {
        activity.GetAssets().SetLargeImage(largeImage.c_str());
    }
    else {
        std::string utf8LargeImage = convertSJIStoUTF8(largeImage);
        activity.GetAssets().SetLargeImage(utf8LargeImage.c_str());
    }
    return 0;
}

bool WINAPI DiscordActivitySetLargeText(HSPEXINFO* hei)
{
    char* _largeText = hei->HspFunc_prm_getns();
    std::string largeText = _largeText;
    if (hei->hspctx->hspstat & HSPSTAT_UTF8) {
        activity.GetAssets().SetLargeText(largeText.c_str());
    }
    else {
        std::string utf8LargeText = convertSJIStoUTF8(largeText);
        activity.GetAssets().SetLargeText(utf8LargeText.c_str());
    }
    return 0;
}

// Timestamps
bool WINAPI DiscordActivitySetStart(HSPEXINFO* hei)
{
    activity.GetTimestamps().SetStart(hei->HspFunc_prm_geti());
    return 0;
}

bool WINAPI DiscordActivitySetEnd(HSPEXINFO* hei)
{
    activity.GetTimestamps().SetEnd(hei->HspFunc_prm_geti());
    return 0;
}

// Party
bool WINAPI DiscordActivitySetPartyId(HSPEXINFO* hei)
{
    char* _id = hei->HspFunc_prm_getns();
    std::string id = _id;
    if (hei->hspctx->hspstat & HSPSTAT_UTF8) {
        activity.GetParty().SetId(id.c_str());
    }
    else {
        std::string utf8Id = convertSJIStoUTF8(id);
        activity.GetParty().SetId(utf8Id.c_str());
    }
    return 0;
}

bool WINAPI DiscordActivitySetPartyPrivacy(HSPEXINFO* hei)
{
    int privacy = hei->HspFunc_prm_geti();
    if (privacy == 0) {
        activity.GetParty().SetPrivacy(discord::ActivityPartyPrivacy::Private);
    }
    else if (privacy == 1) {
        activity.GetParty().SetPrivacy(discord::ActivityPartyPrivacy::Public);
    }
    else {
        puterror(HSPERROR::HSPERR_INVALID_PARAMETER);
    }
    return 0;
}

bool WINAPI DiscordUpdateActivity(HSPEXINFO* hei)
{
    static discord::Result fresult = discord::Result::Ok;
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        fresult = result;
    });
    hei->hspctx->stat = resultToInt(fresult);
    return 0;
}
