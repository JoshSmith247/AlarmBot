#pragma once

/* STATE payload: [state, flags]. ALARM_START payload: [speed 0-100], empty means default. */
enum { ST_IDLE, ST_FLEEING, ST_CAUGHT, ST_FAULT };
enum { FL_EDGE = 1, FL_OBSTACLE = 2, FL_LIFTED = 4 };

#define DEFAULT_SPEED 60
#define HEARTBEAT_MS 1000
#define RING_MAX_MS 300000
#define BEEP_MS 400
#define LINK_TIMEOUT_MS 3000
