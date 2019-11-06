# VoiceHook
For plugin SpeakingList (L4D/L4D2) http://core-ss.org/forum/threads/207/#post-10721

## Natives

```javascript

/**
 * Called when a client is speaking.
 *
 * @param client        Client index.
 */
forward void OnClientSpeaking(int client);

/**
 * Called once a client speaking end.
 *
 * @param client        Client index.
 */
forward void OnClientSpeakingEnd(int client);

/**
 * Called once a client speaking start.
 *
 * @param client        Client index.
 */
forward void OnClientSpeakingStart(int client);

/**
 * Returns if a certain player is speaking.
 *
 * @param client        Player index (index does not have to be connected).
 * @return              True if player is speaking, false otherwise.
 * @error               Invalid client index.
 */
native bool IsClientSpeaking(int client);