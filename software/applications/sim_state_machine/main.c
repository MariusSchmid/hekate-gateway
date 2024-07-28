
typedef enum
{
    STATE_INITIAL,
    STATE_GPRS_REGISTERED,
    STATE_BAR,
    NUM_STATES
} state_t;

typedef struct instance_data_s
{

} instance_data_t;
typedef state_t state_func_t(instance_data_t *data);

state_t do_state_initial(instance_data_t *data)
{
    return STATE_GPRS_REGISTERED;
}
state_t do_state_foo(instance_data_t *data)
{
    return STATE_BAR;
}
state_t do_state_bar(instance_data_t *data)
{
    return STATE_INITIAL;
}

state_func_t *const state_table[NUM_STATES] = {
    do_state_initial, do_state_foo, do_state_bar};

state_t run_state(state_t cur_state, instance_data_t *data)
{
    return state_table[cur_state](data);
};

int main(int argc, char const *argv[])
{
    state_t cur_state = STATE_INITIAL;
    instance_data_t data;

    while (1)
    {
        cur_state = run_state(cur_state, &data);
    }
    return 0;
}
