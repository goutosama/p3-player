// библиотека для эмуляции Serial порта
#include <DFMiniMp3.h>
#include <SoftwareSerial.h>
#include "structs.h"
// Declaration for SoftwareSerial (for communication with player) and DFPLayerMini chip
SoftwareSerial SoftSerial(10, 11);

// Init values
/*
  int buttonNext = 2;   // кнопка следующий трек
  int buttonPause = 3;  // кнопка пауза/ пуск
  int buttonPrevious = 4; // кнопка предыдущий трек
  int buttonVolumeUp = 5; // кнопка увеличение громкости
  int buttonVolumeDown = 6; // кнопка уменьшение громкости
  boolean isPlaying = false; // статус воспроизведения/пауза
*/

// forward declare the notify class, just the name
//
class Mp3Callbacks;

// define a handy type using serial and our notify class
//
typedef DFMiniMp3<SoftwareSerial, Mp3Callbacks, Mp3ChipMH2024K16SS> DfMp3;

// instance a DfMp3 object,
//
DfMp3 dfmp3(SoftSerial);

class Mp3Callbacks
{
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char *action)
  {
    if (source & DfMp3_PlaySources_Sd)
    {
      Serial.print("SD Card, ");
    }
    if (source & DfMp3_PlaySources_Usb)
    {
      Serial.print("USB Disk, ");
    }
    if (source & DfMp3_PlaySources_Flash)
    {
      Serial.print("Flash, ");
    }
    Serial.println(action);
  }
  static void OnError([[maybe_unused]] DfMp3 &mp3, uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished([[maybe_unused]] DfMp3 &mp3, [[maybe_unused]] DfMp3_PlaySources source, uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);

    // start next track
    track += 1;
    // this example will just start back over with 1 after track 3
    if (track > 3)
    {
      track = 1;
    }
    dfmp3.playMp3FolderTrack(track); // sd:/mp3/0001.mp3, sd:/mp3/0002.mp3, sd:/mp3/0003.mp3
  }
  static void OnPlaySourceOnline([[maybe_unused]] DfMp3 &mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "online");
  }
  static void OnPlaySourceInserted([[maybe_unused]] DfMp3 &mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "inserted");
  }
  static void OnPlaySourceRemoved([[maybe_unused]] DfMp3 &mp3, DfMp3_PlaySources source)
  {
    PrintlnSourceAction(source, "removed");
  }
};

void initPlayer(){
  dfmp3.begin(); //default 9600 baud
  
  // debug                                      wrap in if with defines.h number
  Serial.println(dfmp3.getSoftwareVersion());

}

void chooseFolder(uint8_t number)
{
  stat->folder = number;
};

// This function changes modes for playing current track. Pass the PlayMode enum as a parameter.
// It has three modes: Loop one track, Loop full folder, Shuffle folder
void switchModes(PlayMode mode)
{

  if (mode == Loop1) // 0 - loop one track
  {
    stat->playMode = mode;
    dfmp3.setRepeatPlayCurrentTrack(true);
  }
  if (mode == LoopFolder) // 1 - loop folder
  {
    stat->playMode = mode;
    dfmp3.loopFolder(stat->folder);
  }
  if (mode == Shuffle) // 2 - shuffle
  {
    // Guys i have a theory (a game theory of course): if setRepeatPlayCurrentTrack(false) plays track and then stops
    // then I can have notifs wait for track finish and then open any random track in folder, then it works and not plays next track
    dfmp3.setRepeatPlayCurrentTrack(false);
  }
}

void nextTrack()
{
  stat->track = dfmp3.getCurrentTrack();
  dfmp3.nextTrack();
};

void prevTrack()
{
  stat->track = dfmp3.getCurrentTrack();
  dfmp3.prevTrack();
};

void playTrack()
{
  dfmp3.playFolderTrack16(stat->folder, stat->track);
  // MAY be required on some chips: TESTING NEEDED
  dfmp3.start();
}

void pause()
{
  if (stat->PlayState == 3)
  {
    dfmp3.pause();
    stat->PlayState = 1;
  }
  else
  { // IDK how this .start() will affect system if it is stopped              WARNING: BUGS
    // and also, how do you even handle Stop state? Should it play same mode again?
    dfmp3.start();
    stat->PlayState = 3;
  }
}

/*!
  @brief This function only adds volume.
  @note Note that it gets uint8 number for compatibility. Use subVolume() to decrease instead
*/
void addVolume(uint8_t add)
{
  dfmp3.setVolume(stat->volume + add);
}

void subVolume(uint8_t sub)
{
  dfmp3.setVolume(stat->volume - sub);
}