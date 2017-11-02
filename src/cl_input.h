#ifndef SRC_CL_INPUT_H_
#define SRC_CL_INPUT_H_

#include "types.h"

#define CLIENT_ALL (-1)

/**
 * @brief Привязка клавиши к действию
 */
typedef struct
{
    /* id игрока */
    int playerId;
    /* код клавиши */
    int key;
    char * action_press;
    char * action_release;
} client_keybind_t;

extern void client_input_init();
extern void client_input_done();

extern int client_key_bind(int playerId, int key, const char * action);
extern int client_key_binded_get(int playerId, const char * action);
extern void client_key_unbind(int key);
extern void client_key_unbind_all(void);
extern void client_key_unbind_action(int playerId, const char * action);

extern void client_key_press(int key);
extern void client_key_release(int key);

extern int client_foreachkey(int (*callback)(const client_keybind_t * keybind, void * userdata), void * userdata );


#endif /* SRC_CL_INPUT_H_ */
