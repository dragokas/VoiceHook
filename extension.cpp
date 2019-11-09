#include "extension.h"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

VoiceHook g_VoiceHook;		/**< Global singleton for extension's main interface */
SMEXT_LINK(&g_VoiceHook);

#if SOURCE_ENGINE == SE_CSGO
SH_DECL_HOOK1_void(IServerGameClients, ClientVoice, SH_NOATTRIB, 0, edict_t *);
#else
SH_DECL_HOOK1(IClientMessageHandler, ProcessVoiceData, SH_NOATTRIB, 0, bool, CLC_VoiceData *);
#endif
CGlobalVars *gpGlobals;
IForward *g_OnClientSpeaking = NULL;
IForward *g_OnClientSpeakingEnd = NULL;
IForward *g_OnClientSpeakingStart = NULL;
ITimer *g_pTimerSpeaking[65];
float g_fGameTime[65];
#if SOURCE_ENGINE == SE_CSGO
IServerGameClients *gameclients = NULL;
#else
ISDKTools *g_pSDKTools = NULL;
IServer *g_pServer = NULL;
#endif

const sp_nativeinfo_t g_ExtensionNatives[] =
{
	{ "IsClientSpeaking",		IsClientSpeaking },
	{ NULL,						NULL }
};

bool VoiceHook::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
#if SOURCE_ENGINE == SE_CSGO
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientVoice, gameclients, this, &VoiceHook::ClientVoice, true);
#endif
	gpGlobals = ismm->GetCGlobals();
	return true;
}

bool VoiceHook::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
#if SOURCE_ENGINE == SE_CSGO
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientVoice, gameclients, this, &VoiceHook::ClientVoice, true);
#endif
	return true;
}

bool VoiceHook::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
#if SOURCE_ENGINE != SE_CSGO
	playerhelpers->AddClientListener(&g_VoiceHook);
#endif
	sharesys->AddNatives(myself, g_ExtensionNatives);

	return true;
}

void VoiceHook::SDK_OnAllLoaded()
{
#if SOURCE_ENGINE != SE_CSGO
	SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);
 
 	if (!g_pSDKTools)
 	{
 		return;
	}
	g_pServer = g_pSDKTools->GetIServer();
#endif
	g_OnClientSpeaking = forwards->CreateForward("OnClientSpeaking", ET_Event, 1, NULL, Param_Cell);
	g_OnClientSpeakingEnd = forwards->CreateForward("OnClientSpeakingEnd", ET_Event, 1, NULL, Param_Cell);
	g_OnClientSpeakingStart = forwards->CreateForward("OnClientSpeakingStart", ET_Event, 1, NULL, Param_Cell);
}

void VoiceHook::SDK_OnUnload()
{
	forwards->ReleaseForward(g_OnClientSpeaking);
	forwards->ReleaseForward(g_OnClientSpeakingEnd);
	forwards->ReleaseForward(g_OnClientSpeakingStart);
}

class SpeakingEndTimer : public ITimedEvent
{
public:
	ResultType OnTimer(ITimer *pTimer, void *pData)
	{
		int client = (int)(intptr_t)pData;
		if ((gpGlobals->curtime - g_fGameTime[client]) > 0.1)
		{
			g_OnClientSpeakingEnd->PushCell(client);
			g_OnClientSpeakingEnd->Execute();

			return Pl_Stop;
		}
		return Pl_Continue;
	}
	void OnTimerEnd(ITimer *pTimer, void *pData)
	{
		g_pTimerSpeaking[(int)(intptr_t)pData] = NULL;
	}
} s_SpeakingEndTimer;

#if SOURCE_ENGINE == SE_CSGO
void VoiceHook::ClientVoice(edict_t *pEntity)
{
	if (pEntity)
	{
		int client = gamehelpers->IndexOfEdict(pEntity);
		if (client)
		{
			g_fGameTime[client] = gpGlobals->curtime;

			if (g_pTimerSpeaking[client] == NULL)
			{
				g_pTimerSpeaking[client] = timersys->CreateTimer(&s_SpeakingEndTimer, 0.3f, (void *)(intptr_t)client, 1);

				g_OnClientSpeakingStart->PushCell(client);
				g_OnClientSpeakingStart->Execute();
			}

			g_OnClientSpeaking->PushCell(client);
			g_OnClientSpeaking->Execute();
		}
	}
}
#else
void VoiceHook::OnClientPutInServer(int client)
{
	IClient *pClient = g_pServer->GetClient(client-1);
	if (pClient != NULL)
	{
		SH_ADD_HOOK(IClientMessageHandler, ProcessVoiceData, (IClientMessageHandler *)((intptr_t)(pClient) + 4), SH_MEMBER(this, &VoiceHook::ProcessVoiceData), true);
	}
}

void VoiceHook::OnClientDisconnecting(int client)
{
	IClient *pClient = g_pServer->GetClient(client-1);
	if (pClient != NULL)
	{
		SH_REMOVE_HOOK(IClientMessageHandler, ProcessVoiceData, (IClientMessageHandler *)((intptr_t)(pClient) + 4), SH_MEMBER(this, &VoiceHook::ProcessVoiceData), true);
	}
}

bool VoiceHook::ProcessVoiceData(CLC_VoiceData *msg)
{
	IClient *pClient = (IClient *)((intptr_t)(META_IFACEPTR(IClient)) - 4);
	if (pClient != NULL)
	{
		int client = pClient->GetPlayerSlot() + 1;

		g_fGameTime[client] = gpGlobals->curtime;

		if (g_pTimerSpeaking[client] == NULL)
		{
			g_pTimerSpeaking[client] = timersys->CreateTimer(&s_SpeakingEndTimer, 0.3f, (void *)(intptr_t)client, 1);

			g_OnClientSpeakingStart->PushCell(client);
			g_OnClientSpeakingStart->Execute();
		}

		g_OnClientSpeaking->PushCell(client);
		g_OnClientSpeaking->Execute();
	}
	return true;
}
#endif

static cell_t IsClientSpeaking(IPluginContext *pContext, const cell_t *params)
{
	IGamePlayer *pPlayer = playerhelpers->GetGamePlayer(params[1]);
	if (pPlayer == NULL)
	{
		return pContext->ThrowNativeError("Client index %d is invalid", params[1]);
	}
	else if (!pPlayer->IsConnected())
	{
		return pContext->ThrowNativeError("Client %d is not connected", params[1]);
	}
	else if (!pPlayer->IsInGame())
	{
		return pContext->ThrowNativeError("Client %d is not in-game", params[1]);
	}
	else if (pPlayer->IsFakeClient())
	{
		return pContext->ThrowNativeError("Client %d is a bot", params[1]);
	}
	
	return g_pTimerSpeaking[params[1]] != NULL;
}
