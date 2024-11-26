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
    //      ���s���� (���ߎ��s���ɌĂ΂�܂�)
    //
    code_next();                            // ���̃R�[�h���擾(�ŏ��ɕK���K�v�ł�)

    switch (cmd) {                         // �T�u�R�}���h���Ƃ̕���

    case 0x00:                              // newcmd

        p1 = code_getdi(123);     // �����l���擾(�f�t�H���g123)
        hspstat = p1;                  // �V�X�e���ϐ�stat�ɑ��
        break;
    default:
        puterror(HSPERR_UNSUPPORTED_FUNCTION);
    }
    return RUNMODE_RUN;
}


/*------------------------------------------------------------*/

static int ref_ival;                        // �Ԓl�̂��߂̕ϐ�

static void* reffunc(int* type_res, int cmd)
{
    //      �֐��E�V�X�e���ϐ��̎��s���� (�l�̎Q�Ǝ��ɌĂ΂�܂�)
    //
    //          '('�Ŏn�܂邩�𒲂ׂ�
    //
    if (*type != TYPE_MARK) puterror(HSPERR_INVALID_FUNCPARAM);
    if (*val != '(') puterror(HSPERR_INVALID_FUNCPARAM);
    code_next();


    switch (cmd) {                         // �T�u�R�}���h���Ƃ̕���

    case 0x00:                              // newcmd

        p1 = code_geti();               // �����l���擾(�f�t�H���g�Ȃ�)
        ref_ival = p1 * 2;              // �Ԓl��ival�ɐݒ�
        break;

    default:
        puterror(HSPERR_UNSUPPORTED_FUNCTION);
    }

    //          '('�ŏI��邩�𒲂ׂ�
    //
    if (*type != TYPE_MARK) puterror(HSPERR_INVALID_FUNCPARAM);
    if (*val != ')') puterror(HSPERR_INVALID_FUNCPARAM);
    code_next();

    *type_res = HSPVAR_FLAG_INT;            // �Ԓl�̃^�C�v�𐮐��Ɏw�肷��
    return (void*)&ref_ival;
}


/*------------------------------------------------------------*/

static int termfunc(int option)
{
    //      �I������ (�A�v���P�[�V�����I�����ɌĂ΂�܂�)
    //
    return 0;
}

/*------------------------------------------------------------*/

static int eventfunc(int event, int prm1, int prm2, void* prm3)
{
    //      �C�x���g���� (HSP�C�x���g�������ɌĂ΂�܂�)
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
    //      DLL�G���g���[ (��������K�v�͂���܂���)
    //
    return TRUE;
}


EXPORT void WINAPI hsp3cmdinit(HSP3TYPEINFO* info)
{
    //      �v���O�C�������� (���s�E�I��������o�^���܂�)
    //
    hsp3sdk_init(info);           // SDK�̏�����(�ŏ��ɍs�Ȃ��ĉ�����)
    info->cmdfunc = cmdfunc;        // ���s�֐�(cmdfunc)�̓o�^
    info->reffunc = reffunc;        // �Q�Ɗ֐�(reffunc)�̓o�^
    info->termfunc = termfunc;      // �I���֐�(termfunc)�̓o�^

    /*
    //  �C�x���g�R�[���o�b�N�𔭐�������C�x���g��ʂ�ݒ肷��
    info->option = HSPEVENT_ENABLE_GETKEY;
    info->eventfunc = eventfunc;    // �C�x���g�֐�(eventfunc)�̓o�^
    */
}
/*------------------------------------------------------------*/
/*
        �ȉ����C��
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