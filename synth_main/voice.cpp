#include "voice.h"
#include "frequencies.h"


Voice::Voice() {

  this->waveform1 = WAVEFORM_SINE;
  this->waveform2 = WAVEFORM_SINE;

  this->pw = 1;
  this->detune = 1;

  this->osc1 = new AudioSynthWaveform();
  this->osc2 = new AudioSynthWaveform();
  this->noise = new AudioSynthNoiseWhite();

  this->noteMixer = new AudioMixer4();
  this->noteMixer->gain(0, 0.5);
  this->noteMixer->gain(1, 0.5);
  this->noteMixer->gain(1, 1);

  this->env = new AudioEffectEnvelope();
  this->env->attack(2.0);
  this->env->hold(2.1);
  this->env->decay(31.4);
  this->env->sustain(1.0);
  this->env->release(284.5);

  this->output = new AudioMixer4();

  this->patchCords[0] = new AudioConnection(*this->osc1, 0, *this->noteMixer, 0);
  this->patchCords[1] = new AudioConnection(*this->osc2, 0, *this->noteMixer, 1);
  this->patchCords[2] = new AudioConnection(*this->noise, 0, *this->noteMixer, 2);
  this->patchCords[3] = new AudioConnection(*this->noteMixer, 0, *this->env, 0);
  this->patchCords[4] = new AudioConnection(*this->env, 0, *this->output, 0);
}

Voice::~Voice() {

}

void Voice::noteOn(byte midiNote) {
  this->currentNote = midiNote;

  float f1, f2;
  if (this->waveform1 == WAVEFORM_PULSE) {
    f1 = tune_frequencies2_PGM[this->currentNote + 12];
  }
  else {
    f1 = tune_frequencies2_PGM[this->currentNote];
  }

  f2 = tune_frequencies2_PGM[midiNote - 12] * this->detune;

  AudioNoInterrupts();

  this->osc1->begin(0.2, f1, this->waveform1);

  if (this->waveform2 == WAVEFORM_NOISE) {
    this->osc2->amplitude(0);
    this->noise->amplitude(0.2);
  }
  else {
    this->osc2->begin(0.2, f2, this->waveform2);
    this->noise->amplitude(0);
  }

  this->env->noteOn();

  AudioInterrupts();
}

void Voice::noteOff() {
  this->env->noteOff();
}

void Voice::setWaveForm1(byte waveform) {

  if (this->waveform1 == WAVEFORM_PULSE || waveform == WAVEFORM_PULSE) {
    float f;
    if (waveform == WAVEFORM_PULSE) {
      f = tune_frequencies2_PGM[this->currentNote + 12];
    }
    else {
      f = tune_frequencies2_PGM[this->currentNote];
    }
    this->osc1->begin(0.2, f, waveform);
  }
  else {
    // Simply change the waveform without begin
    this->osc1->begin(waveform);
  }

  this->waveform1 = waveform;
}

void Voice::setWaveForm2(byte waveform) {
  this->waveform2 = waveform;

  if (this->waveform2 == WAVEFORM_NOISE) {
    this->osc2->amplitude(0);
    this->noise->amplitude(0.2);
  }
  else {
    this->osc2->begin(waveform);
    this->osc2->amplitude(0.2);
    this->noise->amplitude(0);
  }
}

void Voice::setPulseWidth(float pw) {
  this->pw = pw;

  this->osc1->pulseWidth(pw);
}

void Voice::setDetune(float detune) {
  this->detune = detune;
  this->osc2->frequency(tune_frequencies2_PGM[this->currentNote - 12] * this->detune);
}

