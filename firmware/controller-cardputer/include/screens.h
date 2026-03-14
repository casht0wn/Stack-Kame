#pragma once

#include <app_state.h>
#include <communication.h>

void handleInput(const KeyInput &input, AppState &state, CommandSender &sender);
void refreshDiagnostics(AppState &state);
void drawActiveScreen(const AppState &state);
