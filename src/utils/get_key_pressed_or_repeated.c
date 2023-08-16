#include <time.h>
#include <raylib.h>
#include "get_key_pressed_or_repeated.h"

#define MAX_PRESSED_KEYS 10
static struct timespec pressed_time[MAX_PRESSED_KEYS];
static int             pressed_keys[MAX_PRESSED_KEYS];
static int             repeat_count[MAX_PRESSED_KEYS];
static int         num_pressed_keys = 0;

#define NSEC_PER_SEC 1000000000

static int time_diff_as_ns(struct timespec t1, struct timespec t2)
{
    return (t1.tv_sec - t2.tv_sec) * NSEC_PER_SEC + (t1.tv_nsec - t2.tv_nsec);
}

int GetKeyPressedOrRepeated(int *repeat, int repeat_freq_us, int first_repeat_freq_us)
{
    struct timespec curr_time;
    int have_time = !clock_gettime(CLOCK_REALTIME, &curr_time);
    
    int key = GetKeyPressed();
    if (key > 0 && have_time) {
        if (num_pressed_keys < MAX_PRESSED_KEYS) {
            repeat_count[num_pressed_keys] = 0;
            pressed_time[num_pressed_keys] = curr_time;
            pressed_keys[num_pressed_keys] = key;
            num_pressed_keys++;
        }
        if (repeat)
            *repeat = false;
    } else {
        if (have_time) {
            for (int i = 0; i < num_pressed_keys; i++) {
                if (IsKeyDown(pressed_keys[i])) {

                    int delay = (repeat_count[i] > 0) ? repeat_freq_us : first_repeat_freq_us;

                    if (time_diff_as_ns(curr_time, pressed_time[i]) > delay * 1000) {
                        key = pressed_keys[i];
                        pressed_time[i] = curr_time;
                        repeat_count[i]++;
                        if (repeat)
                            *repeat = true;
                        break;
                    }
                } else {
                    num_pressed_keys--;
                    pressed_time[i] = pressed_time[num_pressed_keys];
                    repeat_count[i] = repeat_count[num_pressed_keys];
                    pressed_keys[i] = pressed_keys[num_pressed_keys];
                    i--;
                }
            }
        }
    }
    return key;
}