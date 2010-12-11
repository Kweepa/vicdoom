#define SOUND_CLAW 0
#define SOUND_DMPAIN 1
#define SOUND_DOROPN 2
#define SOUND_ITEMUP 3
#define SOUND_OOF 4
#define SOUND_PISTOL 5
#define SOUND_PLPAIN 6
#define SOUND_POPAIN 7
#define SOUND_SGCOCK 8
#define SOUND_SGTDTH 9
#define SOUND_SHOTGN 10

void __fastcall__ playSoundInitialize(void);
void __fastcall__ playSound(char soundIndex);

void __fastcall__ setEffectsVolume(char vol);
char __fastcall__ getEffectsVolume(void);

void __fastcall__ setMusicVolume(char vol);
char __fastcall__ getMusicVolume(void);
