// Copyright (C) 2008-2011 by WarHead - United Worlds of MaNGOS - http://www.uwom.de

#include <Jail.h>

Jail::Jail()
{
}

Jail::~Jail()
{
}

bool Jail::LoadConfiguration(bool reload)
{
    // Standard Configuration Initialize...
    m_JailConf.MaxJails = 3;
    m_JailConf.MaxDuration = 720;  // 30 Days
    m_JailConf.MinReason = 40;
    m_JailConf.Radius = 15;     // Yards

    m_JailConf.MapAlly = 0;
    m_JailConf.MapHorde = 1;

    m_JailConf.BanDuration = 168;  // 1 Weeks

    m_JailConf.Amnesty = 3;    // 3 Months

    m_JailConf.GMAcc = 0;

    m_JailConf.AllyPos.m_positionX = -8673.43f;
    m_JailConf.AllyPos.m_positionY = 631.795f;
    m_JailConf.AllyPos.m_positionZ = 96.9406f;
    m_JailConf.AllyPos.m_orientation = 2.1785f;

    m_JailConf.HordePos.m_positionX = 2179.85f;
    m_JailConf.HordePos.m_positionY = -4763.96f;
    m_JailConf.HordePos.m_positionZ = 54.911f;
    m_JailConf.HordePos.m_orientation = 4.44216f;

    m_JailConf.DelChar = false;
    m_JailConf.BanAcc = true;
    m_JailConf.WarnUser = true;
    m_JailConf.Enabled = false;

    QueryResult result = CharacterDatabase.Query("SELECT * FROM `jail_conf`");
    if (!result)
        return false;

    Field * fields = result->Fetch();

    m_JailConf.MaxJails = fields[0].GetUInt32();
    m_JailConf.MaxDuration = fields[1].GetUInt32();
    m_JailConf.MinReason = fields[2].GetUInt32();
    m_JailConf.WarnUser = fields[3].GetBool();

    m_JailConf.AllyPos.m_positionX = fields[4].GetFloat();
    m_JailConf.AllyPos.m_positionY = fields[5].GetFloat();
    m_JailConf.AllyPos.m_positionZ = fields[6].GetFloat();
    m_JailConf.AllyPos.m_orientation = fields[7].GetFloat();
    m_JailConf.MapAlly = fields[8].GetUInt32();

    m_JailConf.HordePos.m_positionX = fields[9].GetFloat();
    m_JailConf.HordePos.m_positionY = fields[10].GetFloat();
    m_JailConf.HordePos.m_positionZ = fields[11].GetFloat();
    m_JailConf.HordePos.m_orientation = fields[12].GetFloat();
    m_JailConf.MapHorde = fields[13].GetUInt32();

    m_JailConf.DelChar = fields[14].GetBool();
    m_JailConf.BanAcc = fields[15].GetBool();
    m_JailConf.BanDuration = fields[16].GetUInt32();
    m_JailConf.Radius = fields[17].GetUInt32();
    m_JailConf.Enabled = fields[18].GetBool();

    m_JailConf.GMAcc = fields[19].GetUInt32();
    m_JailConf.GMChar = fields[20].GetString();
    m_JailConf.Amnesty = fields[21].GetUInt32();

    if (!reload)
        sLog->outInfo(LOG_FILTER_GENERAL, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CONF_LOADED));
    else
        sLog->outInfo(LOG_FILTER_GENERAL, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_RELOAD));

    return true;
}

void Jail::JailCleanup()
{
    sLog->outInfo(LOG_FILTER_SERVER_LOADING, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_STARTUP_CLEANUP));
    CharacterDatabase.PExecute("DELETE FROM `jail` WHERE `guid` NOT IN (SELECT `guid` FROM `characters`)");
}

bool Jail::InfoCommand(ChatHandler * handler)
{
    if (!m_JailConf.Enabled)
        return SendInactive(handler);

    Player * chr = handler->GetSession()->GetPlayer();
    if (!chr)
        return false;

    JailMap::iterator itr = m_JailMap.find(chr->GetGUIDLow());
    if (itr == m_JailMap.end())
    {
        handler->PSendSysMessage(LANG_JAIL_NOTJAILED_INFO, chr->GetName());
        return true;
    }
    // Only registered banishings
    if (!chr->m_NumberJailed && chr->m_JailBans)
    {
        handler->PSendSysMessage(LANG_JAIL_GM_INFO_BANS, chr->GetName(), chr->m_JailBans);
        return true;
    }
    // Active or expired Jail registered
    time_t localtime = time(NULL);
    uint32 min_left = uint32(floor(float(chr->m_JailRelease - localtime) / MINUTE));

    if (min_left && min_left > (m_JailConf.MaxDuration*MINUTE)) // This could lead to "strange" issues. ;)
        min_left = 0;

    if (min_left >= MINUTE)
    {
        uint32 hours = uint32(floor(float(chr->m_JailRelease - localtime) / HOUR));
        handler->PSendSysMessage(LANG_JAIL_JAILED_H_INFO, hours, min_left-(hours*MINUTE));
        handler->PSendSysMessage(LANG_JAIL_REASON, chr->m_JailGMChar.c_str(), chr->m_JailReason.c_str());
    }
    else if (min_left)
    {
        handler->PSendSysMessage(LANG_JAIL_JAILED_M_INFO, min_left);
        handler->PSendSysMessage(LANG_JAIL_REASON, chr->m_JailGMChar.c_str(), chr->m_JailReason.c_str());
    }
    else
        handler->SendSysMessage(LANG_JAIL_PRESERVATION);

    return true;
}

bool Jail::GotoCommand(ChatHandler * handler, const char * args)
{
    if (!m_JailConf.Enabled)
        return SendInactive(handler);

    Player * chr = handler->GetSession()->GetPlayer();
    if (!chr)
        return false;

    std::string tmp;
    char * chartmp = strtok((char*)args, " ");
    if (chartmp == NULL)
    {
        handler->SendSysMessage(LANG_JAIL_NO_SITE);
        handler->SetSentErrorMessage(true);
        return false;
    }
    else
    {
        tmp = chartmp;
        normalizePlayerName(tmp);
    }

    if (tmp == "horde")
        chr->TeleportTo(m_JailConf.MapHorde, m_JailConf.HordePos.m_positionX, m_JailConf.HordePos.m_positionY, m_JailConf.HordePos.m_positionZ, m_JailConf.HordePos.m_orientation);
    else
        chr->TeleportTo(m_JailConf.MapAlly, m_JailConf.AllyPos.m_positionX, m_JailConf.AllyPos.m_positionY, m_JailConf.AllyPos.m_positionZ, m_JailConf.AllyPos.m_orientation);

    return true;
}

bool Jail::PInfoCommand(ChatHandler * handler, const char * args)
{
    if (!m_JailConf.Enabled)
        return SendInactive(handler);

    Player * target = NULL;
    uint64 target_guid = 0;
    std::string target_name;

    if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
        return false;

    JailMap::iterator itr = m_JailMap.find(GUID_LOPART(target_guid));
    if (itr == m_JailMap.end())
    {
        handler->PSendSysMessage(LANG_JAIL_GM_NOINFO, target_name.c_str());
        return true;
    }

    if (target)
    {
        // No jails filled
        if (!target->m_NumberJailed)
        {
            // No bans registered
            if (!target->m_JailBans)
            {
                if (handler->GetSession()->GetPlayer()->GetGUID() == target->GetGUID())
                {
                    handler->SendSysMessage(LANG_JAIL_NOTJAILED_INFO);
                    return true;
                }
                else
                {
                    handler->SendSysMessage(LANG_JAIL_GM_NOINFO);
                    return true;
                }
            }
            // Ban is not set to 0
            else
            {
                handler->PSendSysMessage(LANG_JAIL_GM_INFO_BANS, target_name.c_str(), target->m_JailBans);
                return true;
            }
        }
        // Active or expired Jail registered
        time_t localtime = time(NULL);
        uint32 min_left = uint32(floor(float(target->m_JailRelease - localtime) / MINUTE));

        if (min_left && min_left > (m_JailConf.MaxDuration*MINUTE)) // This could lead to "strange" issues. ;)
            min_left = 0;

        if (min_left >= MINUTE)
        {
            uint32 hours = uint32(floor(float(target->m_JailRelease - localtime) / HOUR));
            handler->PSendSysMessage(LANG_JAIL_GM_INFO_H, target_name.c_str(), target->m_NumberJailed, hours, min_left-(hours*MINUTE),
                target->m_JailGMChar.c_str(), TimeToTimestampStr(target->m_JailTime, GERMAN).c_str(), target->m_JailReason.c_str());
        }
        else
            handler->PSendSysMessage(LANG_JAIL_GM_INFO, target_name.c_str(), target->m_NumberJailed, min_left,
            target->m_JailGMChar.c_str(), TimeToTimestampStr(target->m_JailTime, GERMAN).c_str(), target->m_JailReason.c_str());

        if (target->m_JailBans)
            handler->PSendSysMessage(LANG_JAIL_GM_INFO_ONLY_BANS, target->m_JailBans);

        return true;
    }
    else
    {
        uint64 GUID = sObjectMgr->GetPlayerGUIDByName(target_name);
        uint32 guid = GUID_LOPART(GUID);

        if (!guid)
        {
            handler->SendSysMessage(LANG_JAIL_WRONG_NAME);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // No active / expired jail, but a medium
        if (!itr->second.Times && itr->second.BTimes)
        {
            handler->PSendSysMessage(LANG_JAIL_GM_INFO_BANS, target_name.c_str(), itr->second.BTimes);
            return true;
        }

        time_t localtime = time(NULL);
        uint32 min_left = uint32(floor(float(itr->second.Release - localtime) / MINUTE));

        if (min_left && min_left > (m_JailConf.MaxDuration*MINUTE)) // This could lead to "strange" issues. ;)
            min_left = 0;

        if (min_left >= MINUTE)
        {
            uint32 hours = uint32(floor(float(itr->second.Release - localtime) / HOUR));
            handler->PSendSysMessage(LANG_JAIL_GM_INFO_H, target_name.c_str(), itr->second.Times, hours, min_left-(hours*MINUTE),
                itr->second.GMChar.c_str(), TimeToTimestampStr(itr->second.Time, GERMAN).c_str(), itr->second.Reason.c_str());
        }
        else
            handler->PSendSysMessage(LANG_JAIL_GM_INFO, target_name.c_str(), itr->second.Times, min_left, itr->second.GMChar.c_str(), TimeToTimestampStr(itr->second.Time, GERMAN).c_str(), itr->second.Reason.c_str());

        if (itr->second.BTimes)
            handler->PSendSysMessage(LANG_JAIL_GM_INFO_ONLY_BANS, itr->second.BTimes);

        return true;
    }
    return false;
}

bool Jail::ArrestCommand(ChatHandler * handler, const char * args)
{
    if (!m_JailConf.Enabled)
        return SendInactive(handler);

    std::string cname;
    char * charname = strtok((char*)args, " ");
    if (charname == NULL)
    {
        handler->SendSysMessage(LANG_JAIL_NONAME);
        handler->SetSentErrorMessage(true);
        return false;
    }
    else
    {
        cname = charname;
        normalizePlayerName(cname);
    }

    char * timetojail = strtok(NULL, " ");
    if (timetojail == NULL)
    {
        handler->SendSysMessage(LANG_JAIL_NOTIME);
        handler->SetSentErrorMessage(true);
        return false;
    }

    uint32 jailtime = uint32(atoi(timetojail));
    if (jailtime < 1 || jailtime > m_JailConf.MaxDuration)
    {
        handler->PSendSysMessage(LANG_JAIL_VALUE, m_JailConf.MaxDuration);
        handler->SetSentErrorMessage(true);
        return false;
    }

    char * reason = strtok(NULL, "\0");
    std::string jailreason;
    if (reason == NULL || strlen((const char*)reason) < m_JailConf.MinReason)
    {
        handler->PSendSysMessage(LANG_JAIL_NOREASON, m_JailConf.MinReason);
        handler->SetSentErrorMessage(true);
        return false;
    }
    else
        jailreason = reason;

    uint64 GUID = sObjectMgr->GetPlayerGUIDByName(cname);
    uint32 guid = GUID_LOPART(GUID);

    if (!guid)
    {
        handler->SendSysMessage(LANG_JAIL_WRONG_NAME);
        handler->SetSentErrorMessage(true);
        return false;
    }

    JailMap::iterator itr = m_JailMap.find(guid);
    if (itr != m_JailMap.end())
    {
        // There is already an active jail! Can not lock up again!
        if (itr->second.Release)
        {
            handler->PSendSysMessage(LANG_JAIL_GM_ALREADY_JAILED, cname.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }
    }

    if (guid == GUID_LOPART(handler->GetSession()->GetPlayer()->GetGUID()))
    {
        handler->SendSysMessage(LANG_JAIL_NO_JAIL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    uint32 acc_id = sObjectMgr->GetPlayerAccountIdByPlayerName(cname);
    std::string announce = fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_ANNOUNCE), cname.c_str(), jailtime, handler->GetSession()->GetPlayerName(), jailreason.c_str());

    if (Player * chr = sObjectMgr->GetPlayerByLowGUID(guid))
    {   // Without parentheses, it jumps to the end, if chr == NULL!
        if (Imprisonment(handler, chr, cname, jailtime, jailreason, acc_id, announce))
            return true;
    }
    else
    {
        if (Imprisonment(handler, guid, cname, jailtime, jailreason, acc_id, announce))
            return true;
    }
    return false;
}

bool Jail::ReleaseCommand(ChatHandler * handler, const char * args, bool reset)
{
    if (!m_JailConf.Enabled)
        return SendInactive(handler);

    Player * target = NULL;;
    uint64 target_guid = 0;
    std::string target_name;

    if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
        return false;

    JailMap::iterator itr = m_JailMap.find(GUID_LOPART(target_guid));
    if (itr == m_JailMap.end())
    {
        handler->PSendSysMessage(LANG_JAIL_GM_NOINFO, target_name.c_str());
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (target_guid == handler->GetSession()->GetPlayer()->GetGUID())
    {
        handler->SendSysMessage(LANG_JAIL_NO_JAIL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (target)
    {
        if (target->m_Jailed || reset)
        {
            target->m_Jailed = false;
            target->m_JailRelease = 0;

            if (target->m_NumberJailed)
                --target->m_NumberJailed;

            if (target->m_NumberJailed == 0 || reset)
            {
                CharacterDatabase.PExecute("DELETE FROM `jail` WHERE `guid`=%u LIMIT 1", target->GetGUIDLow());
                // There are no DB entry more -> delete any!
                m_JailMap.erase(itr);
            }
            else
                target->JailDataSave(); // Players gejailt was more than 1x, so save new data!

            handler->PSendSysMessage(LANG_JAIL_WAS_UNJAILED, target_name.c_str());
            ChatHandler(target).PSendSysMessage(LANG_JAIL_YOURE_UNJAILED, handler->GetSession()->GetPlayerName());

            sLog->outDebug(LOG_FILTER_GENERAL, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_FREE), target_name.c_str(), target->GetGUIDLow()));
            sWorld->SendServerMessage(SERVER_MSG_STRING, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_FREE), target_name.c_str(), target->GetGUIDLow()));

            target->TeleportTo(target->GetStartPosition());
        }
        return true;
    }
    else
    {
        if (!itr->second.Release && !reset)
        {
            // There is one entry jail, but the release time is set to 0 ergo -> not yet released can once!
            // To delete the entries of Jail 'reset' command is here! ;)
            handler->PSendSysMessage(LANG_JAIL_CHAR_NOTJAILED, target_name.c_str());
            handler->SetSentErrorMessage(true);
            return false;
        }
        else
        {
            if (itr->second.Times)
                --itr->second.Times;

            if (itr->second.Times == 0 || reset)
            {
                CharacterDatabase.PExecute("DELETE FROM `jail` WHERE `guid`=%u LIMIT 1", GUID_LOPART(target_guid));
                // There are no DB entry more -> delete any!
                m_JailMap.erase(itr);
            }
            else
            {
                CharacterDatabase.PExecute("UPDATE `jail` SET `release`=0,`times`=%u WHERE `guid`=%u LIMIT 1", itr->second.Times, GUID_LOPART(target_guid));
                itr->second.Release = 0;
            }

            // Since the character is not online, we set it back by hand at home. ;)
            CharacterDatabase.PExecute("UPDATE characters c,character_homebind b SET c.position_x=b.posX,c.position_y=b.posY,c.position_z= b.posZ,c.map=b.mapId WHERE c.guid=%u", GUID_LOPART(target_guid));

            sLog->outDebug(LOG_FILTER_GENERAL, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_FREE), target_name.c_str(), GUID_LOPART(target_guid)));
            sWorld->SendServerMessage(SERVER_MSG_STRING, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_FREE), target_name.c_str(), GUID_LOPART(target_guid)));

            handler->PSendSysMessage(LANG_JAIL_WAS_UNJAILED, target_name.c_str());
        }
        return true;
    }
    return false;
}

bool Jail::ResetCommand(ChatHandler * handler, const char * args, bool force)
{
    if (!m_JailConf.Enabled)
        return SendInactive(handler);

    Player * target = NULL;
    uint64 target_guid = 0;
    uint32 add_id = 0;
    std::string target_name;

    if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
        return false;

    JailMap::iterator itr = m_JailMap.find(GUID_LOPART(target_guid));
    if (itr == m_JailMap.end())
    {
        handler->PSendSysMessage(LANG_JAIL_GM_NOINFO, target_name.c_str());
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (target_guid == handler->GetSession()->GetPlayer()->GetGUID())
    {
        handler->SendSysMessage(LANG_JAIL_NO_JAIL);
        handler->SetSentErrorMessage(true);
        return false;
    }

    if (itr != m_JailMap.end())
    {
        // Jails, whose account was banned, delete, can not, unless force (delete command) is set!
        if (!force && ((target && target->m_JailBans) || itr->second.BTimes))
        {
            handler->SendSysMessage(LANG_JAIL_GM_NO_DELETE);
            handler->SetSentErrorMessage(true);
            return false;
        }
    }

    add_id = itr->second.account;

    // Only get out of the jail, the jail before entry will be deleted!
    ReleaseCommand(handler, args, true);

    CharacterDatabase.PExecute("DELETE FROM `jail` WHERE `guid`=%u LIMIT 1", GUID_LOPART(target_guid));

    if (force)
        LoginDatabase.PExecute("DELETE FROM `account_banned` WHERE `id`=%u LIMIT 1", add_id);

    // There are no DB entry more -> delete any! But it may be that itr in Release Command () has already been deleted!
    itr = m_JailMap.find(GUID_LOPART(target_guid));
    if (itr != m_JailMap.end())
        m_JailMap.erase(itr);

    return true;
}

bool Jail::EnableCommand(ChatHandler * handler)
{
    m_JailConf.Enabled = true;
    CharacterDatabase.PExecute("UPDATE `jail_conf` SET `enabled`=1");
    handler->SendSysMessage(LANG_JAIL_ENABLED);
    return Init(true);
}

bool Jail::DisableCommand(ChatHandler * handler)
{
    m_JailConf.Enabled = false;
    CharacterDatabase.PExecute("UPDATE `jail_conf` SET `enabled`=0");
    handler->SendSysMessage(LANG_JAIL_DISABLED);
    return true;
}

bool Jail::ReloadCommand(ChatHandler * handler)
{
    if (LoadConfiguration(true))
    {
        handler->SendSysMessage(LANG_JAIL_RELOAD);

        if (Init(true))
            handler->SendSysMessage(LANG_JAIL_RELOAD_JAIL);

        return true;
    }
    return false;
}

void Jail::Control(Player * pPlayer, bool update)
{
    if (!pPlayer || !m_JailConf.Enabled)
        return;

    if (pPlayer->m_Jailed)
    {
        Position pos;
        uint32 map;

        if (pPlayer->GetTeam() == ALLIANCE)
        {
            map = HoleAllyJailMap();
            pos = HoleAllyJailPos();
        }
        else
        {
            map = HoleHordeJailMap();
            pos = HoleHordeJailPos();
        }

        if (pPlayer->GetMapId() != map || m_JailConf.Radius < uint32(pPlayer->GetDistance(pos)))
            pPlayer->TeleportTo(map, pos.m_positionX, pos.m_positionY, pos.m_positionZ, pos.m_orientation);

        // Player has just logged
        if (!update)
        {
            sLog->outDebug(LOG_FILTER_GENERAL, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_TELE), pPlayer->GetName(), pPlayer->GetGUIDLow()));
            sWorld->SendServerMessage(SERVER_MSG_STRING, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_TELE), pPlayer->GetName(), pPlayer->GetGUIDLow()));
        }
    }
}

void Jail::Amnesty()
{
    time_t localtime = time(NULL);

    for (JailMap::iterator itr = m_JailMap.begin(); itr != m_JailMap.end(); ++itr)
        if (itr->second.Time+(m_JailConf.Amnesty*MONTH) <= localtime)
        {
            CharacterDatabase.PExecute("DELETE FROM `jail` WHERE `guid`=%u LIMIT 1", itr->first);
            m_JailMap.erase(itr);
            break;
        }
}

void Jail::Update()
{
    sLog->outDebug(LOG_FILTER_GENERAL, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_DEBUG_UPDATE_1));

    if (m_JailMap.empty())
    {
        sLog->outDebug(LOG_FILTER_GENERAL, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_DEBUG_UPDATE_2));
        return;
    }
    // Check for Amnesty if m_JailConf.Amnesty> 0 ...
    if (m_JailConf.Amnesty)
        Amnesty();

    uint32 cnt = 0;
    uint32 oldMSTime = getMSTime();

    for (JailMap::iterator itr = m_JailMap.begin(); itr != m_JailMap.end(); ++itr)
    {
        if (!itr->second.Release)
            continue;

        time_t localtime = time(NULL);

        if (Player * pPlayer = sObjectMgr->GetPlayerByLowGUID(itr->first))
        {
            // Online Char gefunden!
            ++cnt;

            if (itr->second.Release > localtime)
                continue;

            pPlayer->m_Jailed = false;
            pPlayer->m_JailRelease = 0;
            pPlayer->JailDataSave();
            pPlayer->TeleportTo(pPlayer->GetStartPosition());

            sLog->outDebug(LOG_FILTER_GENERAL, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_FREE), pPlayer->GetName(), pPlayer->GetGUIDLow()));
            sWorld->SendServerMessage(SERVER_MSG_STRING, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_FREE), pPlayer->GetName(), pPlayer->GetGUIDLow()));

            continue;
        }

        if (itr->second.Release <= localtime)
        {
            ++cnt;

            itr->second.Release = 0;
            CharacterDatabase.PExecute("UPDATE `jail` SET `release`=%u WHERE `guid`=%u LIMIT 1", itr->second.Release, itr->first);

            // Since the character is not online, we set it back by hand at home. ;)
            CharacterDatabase.PExecute("UPDATE characters c,character_homebind b SET c.position_x=b.posX,c.position_y=b.posY,c.position_z= b.posZ,c.map=b.mapId WHERE c.guid=%u", itr->first);

            sLog->outDebug(LOG_FILTER_GENERAL, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_FREE), itr->second.CharName.c_str(), itr->first));
            sWorld->SendServerMessage(SERVER_MSG_STRING, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_CHAR_FREE), itr->second.CharName.c_str(), itr->first));
        }
    }
    sLog->outDebug(LOG_FILTER_GENERAL, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_DEBUG_UPDATE_3), cnt, GetMSTimeDiffToNow(oldMSTime)));
}

bool Jail::Init(bool reload)
{
    m_JailMap.clear(); // For the Reload

    sLog->outInfo(LOG_FILTER_GENERAL, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_INIT_1));

    if (!m_JailConf.Enabled)
    {
         sLog->outInfo(LOG_FILTER_GENERAL, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_INIT_2));
         return false;
    }

    QueryResult result = CharacterDatabase.Query("SELECT * FROM `jail`");
    if (!result)
    {
        sLog->outInfo(LOG_FILTER_GENERAL, (sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_INIT_3)));
        return false;
    }

    uint32 cnt = 0;
    uint32 cntaktiv = 0;
    uint32 oldMSTime = getMSTime();

    do
    {
        ++cnt;

        Field * fields = result->Fetch();

        JailEntryStructure JES;

        uint32 guid     = fields[0].GetUInt32();
        JES.CharName    = fields[1].GetString();
        JES.Release     = fields[2].GetUInt32();
        JES.Reason      = fields[3].GetString();
        JES.Times       = fields[4].GetUInt32();
        JES.GMAcc       = fields[5].GetUInt32();
        JES.GMChar      = fields[6].GetString();
        JES.Time        = fields[7].GetUInt32();
        JES.Duration    = fields[8].GetUInt32();
        JES.BTimes      = fields[9].GetUInt32();
        JES.account     = sObjectMgr->GetPlayerAccountIdByPlayerName(JES.CharName);

        if (JES.Release)
            ++cntaktiv;

         m_JailMap[guid] = JES;

    } while (result->NextRow());

    sLog->outInfo(LOG_FILTER_GENERAL, fmtstring(sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_INIT_4), cnt, cntaktiv, GetMSTimeDiffToNow(oldMSTime)));

    if (reload)
        sLog->outInfo(LOG_FILTER_GENERAL, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_RELOAD_JAIL));
    else
        sLog->outInfo(LOG_FILTER_GENERAL, sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_DATA_LOADED));

    return true;
}

bool Jail::SendInactive(ChatHandler * handler)
{
    handler->SendSysMessage(LANG_JAIL_NOT_ACTIVE);
    handler->SetSentErrorMessage(true);
    return false;
}

void Jail::SendWarning(Player * pPlayer)
{
    if (!pPlayer || !m_JailConf.Enabled)
        return;

    // Warn only if m_MaxJails set too! ;)
    if (m_JailConf.WarnUser && m_JailConf.MaxJails && (m_JailConf.MaxJails - pPlayer->m_NumberJailed) <= 1)
    {
        if (m_JailConf.DelChar)
            pPlayer->GetSession()->SendNotification(LANG_JAIL_WARNING);

        if (m_JailConf.BanAcc)
            pPlayer->GetSession()->SendNotification(LANG_JAIL_WARNING_BAN);
    }
}

bool Jail::Imprisonment(ChatHandler * handler, Player * chr, std::string cname, uint32 jailtime, std::string jailreason, uint32 acc_id, std::string announce)
{
    time_t localtime = time(NULL);
    uint32 release = uint32(localtime + (jailtime * HOUR));

    if (!chr || !chr->IsInWorld())
        return false;

    chr->SaveToDB();

    chr->m_Jailed = true;
    chr->m_JailRelease = release;
    chr->m_JailReason = jailreason;
    chr->m_JailGMAcc = handler->GetSession()->GetAccountId();
    chr->m_JailGMChar = handler->GetSession()->GetPlayerName();
    chr->m_JailDuration = jailtime;
    chr->m_JailTime = uint32(localtime);

    ++chr->m_NumberJailed;

    chr->JailDataSave();

    handler->PSendSysMessage(LANG_JAIL_WAS_JAILED, cname.c_str(), jailtime);
    ChatHandler(chr).PSendSysMessage(LANG_JAIL_YOURE_JAILED, handler->GetSession()->GetPlayerName(), jailtime);
    ChatHandler(chr).PSendSysMessage(LANG_JAIL_REASON, handler->GetSession()->GetPlayerName(), jailreason.c_str());

    sWorld->SendServerMessage(SERVER_MSG_STRING, announce.c_str());

    // Only if m_MaxJails set, go into it here!
    if (m_JailConf.MaxJails && m_JailConf.MaxJails == chr->m_NumberJailed)
    {
        uint64 GUID = chr->GetGUID();

        if (m_JailConf.DelChar)
        {
            chr->GetSession()->KickPlayer();
            chr = NULL; // Kick player by chr () is invalid!
            Player::DeleteFromDB(GUID, acc_id);
        }

        if (m_JailConf.BanAcc)
        {
            if (chr)
                BannAccount(acc_id, GUID_LOPART(GUID), chr);
            else
                BannAccount(acc_id, GUID_LOPART(GUID));
        }
    }
    return true;
}

bool Jail::Imprisonment(ChatHandler * handler, uint32 guid, std::string cname, uint32 jailtime, std::string jailreason, uint32 acc_id, std::string announce)
{
    time_t localtime = time(NULL);
    uint32 release = uint32(localtime + (jailtime * HOUR));
    uint32 times = 0;
    JailEntryStructure JES;

    JailMap::iterator itr = m_JailMap.find(guid);
    if (itr == m_JailMap.end()) // No entry
    {
        ++times;

        CharacterDatabase.PExecute("INSERT INTO `jail` (`guid`,`char`,`release`,`reason`,`times`,`gmacc`,`gmchar`,`lasttime`,`duration`) VALUES (%u,'%s',%u,'%s',%u,%u,'%s',%u,%u)",
            guid, cname.c_str(), release, jailreason.c_str(), times, handler->GetSession()->GetAccountId(), handler->GetSession()->GetPlayerName(), localtime, jailtime);

        JES.BTimes = 0;
        JES.CharName = cname;
        JES.Duration = jailtime;
        JES.GMAcc = handler->GetSession()->GetAccountId();
        JES.GMChar = handler->GetSession()->GetPlayerName();
        JES.Reason = jailreason;
        JES.Time = uint32(localtime);
        JES.Times = times;
        JES.Release = release;

        m_JailMap[guid] = JES;
    }
    else // There is already an entry
    {
        times = ++itr->second.Times;

        CharacterDatabase.PExecute("UPDATE `jail` SET `release`=%u,`reason`='%s',`times`=%u,`gmacc`=%u,`gmchar`='%s',`lasttime`=%u,`duration`=%u WHERE `guid`=%u LIMIT 1",
            release, jailreason.c_str(), times, handler->GetSession()->GetAccountId(), handler->GetSession()->GetPlayerName(), localtime, jailtime, guid);

        itr->second.Release = release;
        itr->second.Reason = jailreason;
        itr->second.GMAcc = handler->GetSession()->GetAccountId();
        itr->second.GMChar = handler->GetSession()->GetPlayerName();
        itr->second.Time = uint32(localtime);
        itr->second.Duration = jailtime;
    }
    handler->PSendSysMessage(LANG_JAIL_WAS_JAILED, cname.c_str(), jailtime);
    sWorld->SendServerMessage(SERVER_MSG_STRING, announce.c_str());

    // Only if m_MaxJails set, go into it here!
    if (m_JailConf.MaxJails && m_JailConf.MaxJails == times)
    {
        if (m_JailConf.DelChar)
            Player::DeleteFromDB(guid, acc_id);

        if (m_JailConf.BanAcc)
            BannAccount(acc_id, guid);
    }
    return true;
}

void Jail::BannAccount(uint32 acc_id, uint32 guid, Player * chr)
{
    uint32 btimes = 0;
    time_t localtime = time(NULL);
    uint32 banrelease = uint32(localtime + (m_JailConf.BanDuration * HOUR));

    JailMap::iterator itr = m_JailMap.find(guid);
    if (itr != m_JailMap.end())
    {
        btimes = ++itr->second.BTimes;

        itr->second.Release = 0;
        itr->second.Reason = sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_BAN_REASON);
        itr->second.Times = 0;
        itr->second.GMAcc = m_JailConf.GMAcc;
        itr->second.GMChar = m_JailConf.GMChar;
        itr->second.Duration = m_JailConf.BanDuration;
    }
    else
        btimes = 1;

    LoginDatabase.PExecute("REPLACE INTO `account_banned` (`id`,`bandate`,`unbandate`,`bannedby`,`banreason`) VALUES (%u,%u,%u,'%s','%s')",
        acc_id, localtime, banrelease, m_JailConf.GMChar.c_str(), sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_BAN_REASON));

    // If the account was banned, the data must be put in jail back!
    // To be able to see why they were put back, we shall ensure that a spell data. ;)
    CharacterDatabase.PExecute("UPDATE `jail` SET `release`=0,`reason`='%s',`times`=0,`gmacc`=%u,`gmchar`='%s',`duration`=%u,`btimes`=%u WHERE `guid`=%u LIMIT 1",
        sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_BAN_REASON), m_JailConf.GMAcc, m_JailConf.GMChar.c_str(), m_JailConf.BanDuration, btimes, guid);

    if (chr)
    {
        chr->JailDataSave();
        chr->GetSession()->KickPlayer();
    }
}

char const * Jail::fmtstring(char const * format, ...)
{
    va_list     argptr;
    static char temp_buffer[MAX_FMT_STRING];
    static char string[MAX_FMT_STRING];
    static int  index = 0;
    char        *buf;
    int         len;

    va_start(argptr, format);
    vsnprintf(temp_buffer,MAX_FMT_STRING, format, argptr);
    va_end(argptr);

    len = strlen(temp_buffer);

    if (len >= MAX_FMT_STRING)
        return sObjectMgr->GetTrinityStringForDBCLocale(LANG_JAIL_FMTSTR);

    if (len + index >= MAX_FMT_STRING-1)
        index = 0;

    buf = &string[index];
    memcpy(buf, temp_buffer, len+1);

    index += len + 1;

    return buf;
}
