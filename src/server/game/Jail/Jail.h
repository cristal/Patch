// Copyright (C) 2008-2011 by WarHead - United Worlds of MaNGOS - http://www.uwom.de

#ifndef _JAIL_H
#define _JAIL_H

#include <Chat.h>

#define MAX_FMT_STRING  32000

struct JailConfStructure
{
    uint32 MaxJails;    // When reaching this value, the character deleted / banned the account.
    uint32 MaxDuration; // The maximum duration for a Jail from contains.
    uint32 MinReason;   // Minimum length for JailReason.
    uint32 MapAlly;     // Jail Map for the Allies
    uint32 MapHorde;    // Jail Map for the Horde
    uint32 BanDuration; // After that time (hours) a spellbound by the Jail account will be reactivated.
    uint32 Radius;      // May move in the radius of the Jail inmate.
    uint32 GMAcc;       // Account which is used for banning
    uint32 Amnesty;     // Months (after the last incident), after which the jail will be deleted entries.

    Position AllyPos;   // Coordinates for the AllyJail
    Position HordePos;  // Coordinates for the HordeJail

    bool WarnUser;      // Warn players if he is just a jail on the erasure character / account banishment?
    bool DelChar;       // Delete character when m_MaxJails achieved?
    bool BanAcc;        // Account ban when m_MaxJails achieved?
    bool Enabled;       // Jail is activated?

    std::string GMChar; // Charname which is used for banning
};

struct JailEntryStructure
{
    uint32 Release;         // Release Time
    uint32 Times;           // Number of Imprisonment
    uint32 BTimes;          // Number of accounts of bans on Reason by this Char
    uint32 Duration;        // Duration of Imprisonment
    uint32 GMAcc;           // Account of the GM
    uint32 Time;            // Imprisonments time

    std::string CharName;   // Character name of the inmate
    std::string GMChar;     // Character name of the GM
    std::string Reason;     // Reason of Imprisonment

    uint32 account;         // Not available in Jailtable - is only for internal purposes only!
};

typedef UNORDERED_MAP<uint32, JailEntryStructure> JailMap;

class Jail
{
    friend class ACE_Singleton<Jail, ACE_Null_Mutex>;
    Jail();
    ~Jail();

private:
    JailConfStructure    m_JailConf; // configuration of Jails
    JailMap             m_JailMap;   // Jail unordered_map all entries

    void Amnesty(); // Check if someone has to get Amnesty.
    bool SendInactive(ChatHandler * handler);
    bool Imprisonment(ChatHandler * handler, Player * chr, std::string cname, uint32 jailtime, std::string jailreason, uint32 acc_id, std::string announce);
    bool Imprisonment(ChatHandler * handler, uint32 guid, std::string cname, uint32 jailtime, std::string jailreason, uint32 acc_id, std::string announce);
    void BannAccount(uint32 acc_id, uint32 guid, Player * chr = NULL);
    char const * fmtstring(char const * format, ...);

public:
    // load configuration
    bool LoadConfiguration(bool reload = false);
    // Imprisonment invite
    bool Init(bool reload = false);
    // Jail to non-existent characters examined.
    void JailCleanup();
    // Every minute to see if someone has to be dismissed.
    void Update();

    // Editing commands from jail_commandscript
    bool InfoCommand(ChatHandler * handler);
    bool GotoCommand(ChatHandler * handler, const char * args);
    bool PInfoCommand(ChatHandler * handler, const char * args);
    bool ArrestCommand(ChatHandler * handler, const char * args);
    bool ReleaseCommand(ChatHandler * handler, const char * args, bool reset = false);
    bool ResetCommand(ChatHandler * handler, const char * args, bool force = false);
    bool ReloadCommand(ChatHandler * handler);
    bool EnableCommand(ChatHandler * handler);
    bool DisableCommand(ChatHandler * handler);

    Position HoleAllyJailPos() const { return m_JailConf.AllyPos; }
    Position HoleHordeJailPos() const { return m_JailConf.HordePos; }

    uint32 HoleAllyJailMap() const { return m_JailConf.MapAlly; }
    uint32 HoleHordeJailMap() const { return m_JailConf.MapHorde; }

    void Control(Player * pPlayer, bool update = false);
    void SendWarning(Player * pPlayer);

    JailMap const & HoleJailMap() const { return m_JailMap; }
    void UpdateJailMap(uint32 guid, JailEntryStructure & JES) { m_JailMap[guid] = JES; };
};

#define sJail ACE_Singleton<Jail, ACE_Null_Mutex>::instance()

#endif
