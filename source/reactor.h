#ifndef REACTOR_H__
#define REACTOR_H__

#define MOVE_INCREASE_AMOUNT 1

void reactor_init(void);
void reactor_update(void);
void reactor_deinit(void);

void reactor_increase_temp(int amount);
void reactor_decrease_temp(int amount);

#endif
