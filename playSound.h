enum ESound
{
	SOUND_CLAW,
	SOUND_DMPAIN,
	SOUND_DOROPN,
	SOUND_DORCLS,
	SOUND_ITEMUP,
	SOUND_OOF,
	SOUND_GURGLE,
	SOUND_PISTOL,
	SOUND_PLPAIN,
	SOUND_POPAIN,
	SOUND_SGCOCK,
	SOUND_SGTDTH,
	SOUND_SHOTGN,
	SOUND_STNMOV
};

void __fastcall__ playSoundInitialize(void);
void __fastcall__ playSound(char soundIndex);

void __fastcall__ setEffectsVolume(char vol);
char __fastcall__ getEffectsVolume(void);

void __fastcall__ setMusicVolume(char vol);
char __fastcall__ getMusicVolume(void);

char __fastcall__ getTickCount(void);
void __fastcall__ setTickCount(void);