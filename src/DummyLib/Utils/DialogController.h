#pragma once

#include <cstdint>

#include <Sys/Signal_.h>

class ScriptedDialog;
class ScriptedSequence;

class DialogController {
  public:
    enum class eState { Paused, Sequence, ChoicePlaying, ChoicePaused };

    DialogController();

    eState state() const { return state_; }

    void SetDialog(ScriptedDialog *dialog);

    void Play(double cur_time_s);
    void Pause();
    void Update(double cur_time_s);

    double GetPlayTime();
    void SetPlayTime(double cur_time_s, double play_time_s);

    ScriptedSequence *GetCurSequence() { return cur_seq_; }
    void SetCurSequence(int id);

    void MakeChoice(const char *key);

    Sys::Signal<void(const char *text, const uint8_t color[4])> push_caption_signal;
    Sys::Signal<void(const char *key, const char *text)> push_choice_signal;

    Sys::Signal<void(int id)> switch_sequence_signal;

  private:
    ScriptedDialog *dialog_ = nullptr;
    ScriptedSequence *cur_seq_ = nullptr;
    int next_seq_id_ = -1;

    eState state_ = eState::Paused;

    double play_started_time_s_ = 0.0, play_time_s_ = 0.0;

    void OnPushCaption(const char *text, const uint8_t color[4]);
};