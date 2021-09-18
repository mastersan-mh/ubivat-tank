
#include "input.h"

#include "eng_actions.h"
#include "eng_conf.h"

#include "game.h"

#include <string.h>

/*
 *
 */
void input_key_setState(SDL_Scancode key, bool state)
{
    char action_neg[ENG_ACTION_NAME_SIZE];
    const char * action_name = eng_conf_key_action_get(key);
    if(action_name == NULL)
    {
        return;
    }

    const char * act_name;
    if(state)
    {
        act_name = action_name;
    }
    else
    {
        if(action_name[0] == '+')
        {
            strncpy(action_neg, action_name, ENG_ACTION_NAME_SIZE);
            action_neg[0] = '-';
        }
        else if(action_name[0] == '-')
        {
            strncpy(action_neg, action_name, ENG_ACTION_NAME_SIZE);
            action_neg[0] = '+';
        }
        else
        {
            return;
        }
        act_name = action_neg;
    }

    eng_action_exec(act_name);
}
