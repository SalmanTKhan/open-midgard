#pragma once

#include "GameActor.h"

void DrawQueuedMsgEffects(HDC hdc);
bool QueueQueuedMsgEffectsQuads();
bool GetQueuedMsgEffectsBounds(RECT* outRect);
void ClearQueuedMsgEffects();
bool HasQueuedMsgEffects();
bool HasActiveMsgEffects();
