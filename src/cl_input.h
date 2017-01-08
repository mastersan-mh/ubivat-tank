#ifndef SRC_CL_INPUT_H_
#define SRC_CL_INPUT_H_

#include "types.h"
#include "actions.h"

#define CLIENT_ALL (-1)

typedef enum
	{
		INPUT_ACTIONSRC_STR,
		INPUT_ACTIONSRC_FUNC
	} input_actionsrc_t;

typedef struct
{
	/* id игрока-клиента */
	int clientId;
	/* код клавиши */
	int key;
	input_actionsrc_t actionsrc;
	union
	{
		/* функции обработки нажатий/отпусканий */
		struct
		{
			char * press;
			char * release;
		} str;
		struct
		{
			actionf_t press;
			actionf_t release;
		} func;
	} action;
} input_key_t;

extern void input_init();
extern void input_done();

extern int input_foreachkey(int (*callback)(const input_key_t * keybind, void * userdata), void * userdata );
extern void input_key_setState(int key, bool state);
extern int input_key_bind_act(int clientId, int key, const char * action);
extern int input_key_bind(int key, actionf_t press, actionf_t release);
extern int input_key_bindAction(int key, action_t action);
extern int input_key_get(int clientId, const char * action);
extern void input_key_unbind(int key);
extern void input_key_unbind_all();
extern void input_action_unbind(int clientId, const char * action);


#endif /* SRC_CL_INPUT_H_ */
