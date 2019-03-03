/*
 *  spresense_beatbox.ino  Copyright 2018-2019 Yoshino Taro
 *
 *  This is the source code of Spresense BeatBox.  
 *  You need Spresense Main Board and Extenstion board.
 *  The schematic is "spresense_beatbox.png" in "resources" folder.
 *  Don't forget to store R1-R7.mp3 and L1-L7.mp3 on your SD card
 *  when you run this program.  
 *  
 *  This example code is under LGPL v2.1 
 *  (because Arduino library of Spresense is under LGPL v2.1)
 *  And the mp3 files using this program are under CC-BY-SA
 */

#include <Audio.h>

SDClass theSD;
AudioClass *theAudio;

File file, rFile, lFile;

#define WAIT 0
#define START 1
#define PLAY 2
#define STOP 3
int state_main = WAIT; 
int state_sub = WAIT;
int master_vol = -1020;
int main_vol = -1020;
int sub_vol = -1020;

char* r_filenames[] = {
  "R1.mp3"
 ,"R2.mp3"
 ,"R3.mp3"
 ,"R4.mp3"
 ,"R5.mp3"
 ,"R6.mp3"
 ,"R7.mp3"
};

char* l_filenames[] = {
  "L1.mp3"
 ,"L2.mp3"
 ,"L3.mp3"
 ,"L4.mp3"
 ,"L5.mp3"
 ,"L6.mp3"
 ,"L7.mp3"
};

File& soundOpen(char* name) 
{
  file = theSD.open(name);
  if (!file) {
    printf("%s open fail\n", name);
    exit(1); 
  } 
  return file;
}

int start_r_buttons = 0;
int start_l_buttons = 0;
int start_play(int buttons, bool right)
{
  AudioClass::PlayerId id;
  if (buttons == 0) {
    printf("start_play state error: buttons is zero\n");
    return -1;
  }
  if (right) {
    printf("%s open\n", r_filenames[buttons-1]);
    file = rFile = soundOpen(r_filenames[buttons-1]);
    id = AudioClass::Player0;
    start_r_buttons = buttons;
  } else {
    printf("%s open\n", l_filenames[buttons-1]);
    file = lFile = soundOpen(l_filenames[buttons-1]);
    id = AudioClass::Player1;
    start_l_buttons = buttons;
  }
  err_t err0 = theAudio->writeFrames(id, file); 
  if (err0){
    printf("File Read Error! =%d\n",err0);
    file.seek(0);
    return -1;
  }
  theAudio->startPlayer(id);  
  return 0;
}

err_t continue_play(int buttons, bool right)
{
  AudioClass::PlayerId id;
  if (right) {
    file = rFile;
    id = AudioClass::Player0;
    if (buttons != start_r_buttons) {
      return -1;
    }
  } else {
    file = lFile;
    id = AudioClass::Player1;
    if (buttons != start_l_buttons) {
      return -1;
    }
  }
  err_t err = theAudio->writeFrames(id, file); 
  if (err == AUDIOLIB_ECODE_FILEEND){
    printf("File End\n");
    file.close();
    if (right) file = soundOpen(r_filenames[buttons-1]);
    else       file = soundOpen(l_filenames[buttons-1]);
    return 0;
  } else if (err) {
    printf("File Read Error! =%d\n",err);
    file.close();
    return -1;
  }
  return 0;
}

err_t stop_play(bool right)
{
  AudioClass::PlayerId id;
  if (right) {
    file = rFile;
    id = AudioClass::Player0;
  } else {
    file = lFile;
    id = AudioClass::Player1;    
  }
  err_t err0 = theAudio->stopPlayer(id, AS_STOPPLAYER_NORMAL);
  file.close();  
  if (err0){
    printf("File Read Error! =%d\n",err0);
    return -1;
  }
  return 0;
}

void setup()
{
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);

  /* start audio system */
  theAudio = AudioClass::getInstance();

  puts("initialization Audio Library");
  theAudio->begin();

  /* Set output device to speaker */
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP);

  err_t err = theAudio->initPlayer(AudioClass::Player0, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);
  if (err != AUDIOLIB_ECODE_OK) {
    printf("Player0 initialize error\n");
    exit(1);
  }

  err = theAudio->initPlayer(AudioClass::Player1, AS_CODECTYPE_MP3, "/mnt/sd0/BIN", AS_SAMPLINGRATE_AUTO, AS_CHANNEL_STEREO);
  if (err != AUDIOLIB_ECODE_OK) {
    printf("Player1 initialize error\n");
    exit(1);
  }

  main_vol = 1.1875*analogRead(A0)-1031.88;
  sub_vol = 1.1875*analogRead(A1)-1031.88;
  master_vol = 1.1875*analogRead(A2)-1031.88;
  theAudio->setVolume(master_vol,main_vol,sub_vol);
}

int state_machine(int state, int push)
{
  if (state == WAIT) {
    return push > 0 ? START : WAIT;
  } else if (state == START) {
    return push > 0 ? PLAY : STOP;
  } else if (state == PLAY) {
    return push > 0 ? PLAY : STOP;
  } else if (state == STOP) {
    return push > 0 ? START : WAIT;
  }
  return STOP;
}

void heart_beat(int count)
{
  int a = count % 8;
  switch(a) {
  case 0:
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    break;
  case 1:
    digitalWrite(LED0, HIGH);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    break;
  case 2:
    digitalWrite(LED0, HIGH);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    break;
  case 3:
    digitalWrite(LED0, HIGH);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    break;
  case 4:
    digitalWrite(LED0, HIGH);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    break;
  case 5:
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    break;
  case 6:
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    break;
  case 7:
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    break;
  }
}

unsigned int cnt = 0;
void loop()
{

  heart_beat(cnt);
  main_vol = 1.1875*analogRead(A0)-1031.88;
  sub_vol = 1.1875*analogRead(A1)-1031.88;
  master_vol = 1.1875*analogRead(A2)-1031.88;
  theAudio->setVolume(master_vol,main_vol,sub_vol);

  int r_buttons = !digitalRead(4) | !digitalRead(3) << 1 | !digitalRead(2) << 2;
  int l_buttons = !digitalRead(5) | !digitalRead(6) << 1 | !digitalRead(7) << 2;
  printf("R buttons: %d\n", r_buttons);
  printf("L buttons: %d\n", l_buttons);

  state_main = state_machine(state_main, r_buttons);
  state_sub = state_machine(state_sub, l_buttons);

  int err0 = 0;
  switch(state_main) {
  case WAIT:
    break;
  case START:
    err0 = start_play(r_buttons, true);
    if (err0 < 0) {
      stop_play(true);
      state_main = STOP;
      return;
    }
    break;
  case PLAY:
    err0 = continue_play(r_buttons, true);
    if (err0 < 0) {
      stop_play(true);
      state_main = STOP;
      return;
    }
    break;
  case STOP:
    stop_play(true);
    break;
  }
 
  int err1 = 0;
  switch(state_sub) {
  case WAIT:
    break;
  case START:
    err1 = start_play(l_buttons, false);
    if (err1 < 0) {
      stop_play(false);
      state_sub = STOP;
      return;
    }
    break;
  case PLAY:
    err1 = continue_play(l_buttons, false);
    if (err1 < 0) {
      stop_play(false);
      state_sub = STOP;
      return;
    }
    break;
  case STOP:
    stop_play(false);
    break;
  }

  if (err0 || err1) {
    printf("Main player error code: %d\n", err0);
    printf("Sub player error code: %d\n", err1);
    goto stop_player;
  }
   
  usleep(30000);
  cnt++;
  return;

stop_player:
  sleep(1);
  theAudio->stopPlayer(AudioClass::Player0);
  theAudio->stopPlayer(AudioClass::Player1);
  if (state_main == START || state_main == PLAY)
    rFile.close();
  if (state_sub == START  || state_sub  == PLAY)
    lFile.close();
    
  exit(1);
}
