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

    case 0x00:                              // newcmd

        p1 = code_getdi(123);     // 整数値を取得(デフォルト123)
        hspstat = p1;                  // システム変数statに代入
        break;
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

discord::Core* core{};
discord::Activity activity{};

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

bool WINAPI DiscordInitialize(HSPEXINFO* hei)
{
    char* __clientId = hei->HspFunc_prm_gets();
    std::string _clientId = __clientId;
    int64_t clientId = std::stoll(_clientId);

    discord::Result result = discord::Core::Create(clientId, DiscordCreateFlags_Default, &core);
    if (!core || result != discord::Result::Ok) {
        return 1;
    }

    return 0;
}

bool WINAPI DiscordUpdate(HSPEXINFO* hei)
{
    if (!core) {
        return 1;
    }

    discord::Result result = core->RunCallbacks();
    if (result != discord::Result::Ok) {
        return 1;
    }
    return 0;
}

discord::Activity* WINAPI DiscordActivitySetState(HSPEXINFO* hei)
{
    char* _state = hei->HspFunc_prm_gets();
    std::string state = _state;
    activity.SetState(convertSJIStoUTF8(state).c_str());
    return 0;
}

discord::Activity* WINAPI DiscordActivitySetDetails(HSPEXINFO* hei)
{
    char* _details = hei->HspFunc_prm_gets();
    std::string details = _details;
    activity.SetDetails(convertSJIStoUTF8(details).c_str());
    return 0;
}

discord::Activity* WINAPI DiscordActivitySetName(HSPEXINFO* hei)
{
    char* _name = hei->HspFunc_prm_gets();
    std::string name = _name;
    activity.SetName(convertSJIStoUTF8(name).c_str());
    return 0;
}

discord::Activity* WINAPI DiscordActivitySetType(HSPEXINFO* hei)
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
    return 0;
}

bool WINAPI DiscordUpdateActivity()
{
    static bool fresult = false;
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        if (result == discord::Result::Ok) {
            fresult = true;
        }
        else {
            fresult = false;
        }
    });
    if (fresult == true) {
        return 0;
    }
    else {
        return 1;
    }
}