#ifndef _PATCHMGR_H
#define _PATCHMGR_H

#include "Common.h"
#include "BigNumber.h"
#include "RealmSocket.h"
#include <openssl/md5.h>

class AuthSocket;

typedef struct PATCH_INFO
{
    int build;
    int locale;
    uint64 filesize;
    uint8 md5[MD5_DIGEST_LENGTH];
} PATCH_INFO;

class Patcher
{
    typedef std::vector<PATCH_INFO> Patches;

public:
    void Initialize();

    void LoadPatchMD5(const char*, char*);

    bool InitPatching(int _build, std::string _locale, AuthSocket* _authsocket);
    bool PossiblePatching(int _build, std::string _locale);

private:
    PATCH_INFO* getPatchInfo(int _build, std::string _locale, bool* fallback);

    void LoadPatchesInfo();
    Patches _patches;
};

// Launch a thread to transfer a patch to the client
class PatcherRunnable: public ACE_Based::Runnable
{
public:
    PatcherRunnable(class AuthSocket *, uint64 start, uint64 size);
    void run();
    void stop();

private:
    AuthSocket * mySocket;
    uint64 pos;
    uint64 size;
    bool stopped;
};

#endif