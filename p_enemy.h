#define TEX_ANIMATE 64

void p_enemy_startframe(void);
void p_enemy_think(void);

char allocMobj(char o);
void p_enemy_add_thinker(char o);
void p_enemy_wasseenthisframe(char o);
void p_enemy_damage(char o, char damage);
char p_enemy_get_texture(char o);