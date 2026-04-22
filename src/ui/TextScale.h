#pragma once

constexpr int kTextScaleMinPercent = 75;
constexpr int kTextScaleMaxPercent = 175;
constexpr int kTextScaleDefaultPercent = 100;
constexpr int kTextScaleStepPercent = 10;

int ClampTextScalePercent(int percent);
int GetConfiguredTextScalePercent();
float GetConfiguredTextScaleFactor();
void SetRuntimeTextScalePercent(int percent);
void SaveConfiguredTextScalePercent(int percent);
