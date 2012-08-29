// Copyright (C) 2008-2011 by WarHead - United Worlds of MaNGOS - http://www.uwom.de

#include "Jail.h"
#include "ScriptMgr.h"
#include "Chat.h"

class jail_commandscript : public CommandScript
{
public:
    jail_commandscript() : CommandScript("jail_commandscript") { }

    ChatCommand * GetCommands() const
    {
        static ChatCommand JailCommandTable[] =
        {
            { "info",       SEC_PLAYER,         true,   &HandleJailInfoCmd,     "", NULL },
            { "goto",       SEC_PLAYER,         true,   &HandleJailGotoCmd,     "", NULL },
            { "pinfo",      SEC_MODERATOR,      true,   &HandleJailPInfoCmd,    "", NULL },
            { "arrest",     SEC_GAMEMASTER,     true,   &HandleJailArrestCmd,   "", NULL },
            { "release",    SEC_GAMEMASTER,     true,   &HandleJailReleaseCmd,  "", NULL },
            { "reset",      SEC_GAMEMASTER,     true,   &HandleJailResetCmd,    "", NULL },
            { "reload",     SEC_GAMEMASTER,     true,   &HandleJailReloadCmd,   "", NULL },
            { "enable",     SEC_GAMEMASTER,     true,   &HandleJailEnableCmd,   "", NULL },
            { "disable",    SEC_GAMEMASTER,     true,   &HandleJailDisableCmd,  "", NULL },
            { "delete",     SEC_ADMINISTRATOR,  true,   &HandleJailDeleteCmd,   "", NULL },
            { NULL,         0,                  false,  NULL,                   "", NULL }
        };
        static ChatCommand commandTable[] =
        {
            { "jail",   SEC_PLAYER, true,   NULL,   "", JailCommandTable },
            { NULL,     0,          false,  NULL,   "", NULL }
        };
        return commandTable;
    }
    static bool HandleJailInfoCmd(ChatHandler       * handler, const char * /*args*/)   { return sJail->InfoCommand(handler);              }
    static bool HandleJailGotoCmd(ChatHandler       * handler, const char * args)       { return sJail->GotoCommand(handler, args);        }
    static bool HandleJailPInfoCmd(ChatHandler      * handler, const char * args)       { return sJail->PInfoCommand(handler, args);       }
    static bool HandleJailArrestCmd(ChatHandler     * handler, const char * args)       { return sJail->ArrestCommand(handler, args);      }
    static bool HandleJailReleaseCmd(ChatHandler    * handler, const char * args)       { return sJail->ReleaseCommand(handler, args);     }
    static bool HandleJailResetCmd(ChatHandler      * handler, const char * args)       { return sJail->ResetCommand(handler, args);       }
    static bool HandleJailReloadCmd(ChatHandler     * handler, const char * /*args*/)   { return sJail->ReloadCommand(handler);            }
    static bool HandleJailEnableCmd(ChatHandler     * handler, const char * /*args*/)   { return sJail->EnableCommand(handler);            }
    static bool HandleJailDisableCmd(ChatHandler    * handler, const char * /*args*/)   { return sJail->DisableCommand(handler);           }
    static bool HandleJailDeleteCmd(ChatHandler     * handler, const char * args)       { return sJail->ResetCommand(handler, args, true); }
};

void AddSC_jail_commandscript()
{
    new jail_commandscript();
}
