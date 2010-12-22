//extern mobj_t mobjs[];

void p_enemy_startframe(void);

char allocMobj(char o);
void p_enemy_think(char o);
void p_enemy_wasseenthisframe(char o);
void p_enemy_damage(char o, char damage);
char p_enemy_get_texture(char o);