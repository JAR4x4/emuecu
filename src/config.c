#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "injection.h"

// defaults
#define PWM_MIN       (910)
#define PWM_MAX       (2230)

#define DWELL_TIME_MS (2000)
#define IDLE_RPM      (1200)

#define CONFIG_EE_ADDR (0)

emuconfig_t config;

const int16_t atab0[A_TAB_SIZE] PROGMEM = {
  -1008,   358,  1532,  2541,
  3411,  4168, 4840, 5452,
  6031,  6604,  7197,  7837,
  8550, 9364, 10304, 11397, 12670
};

const int16_t atab1[A_TAB_SIZE] PROGMEM = {
  -2775, -1848, -1058,  -385,
  192,   698,  1154, 1580,
  1999,  2434,  2905,  3434,
  4045,  4758,  5595,  6578,  7730
};

void config_defaults()
{
  config.version = CONFIG_VERSION;
  config.dwell_time_ms = DWELL_TIME_MS;
  config.idle_rpm = IDLE_RPM;
  config.pwm_max = PWM_MAX;
  config.pwm_min = PWM_MIN;
  config.rpm_limit = RPM_LIMIT; // set to table boundary
  config.capacity = 35;
  config.inj_open = 900;
  config.inj_close = 650;
  config.inj_flow = 38;

  memcpy_P(config.a0cal, atab0, sizeof(config.a0cal));
  memcpy_P(config.a1cal, atab1, sizeof(config.a1cal));
  inj_map_default();
  memset(config.ign_adv, 0, sizeof(config.ign_adv));
  config.checksum = 0;
}

uint16_t calc_chksum(const void* const data, size_t n)
{
  uint16_t chksum = 0;
  const uint8_t* end_p = (const uint8_t*)data + n;
  for (const uint8_t* p = (const uint8_t*)data; p < end_p ; ++p)
  {
    chksum += *p;
  }
  return chksum;
}

void config_load()
{
  eeprom_read_block(&config, CONFIG_EE_ADDR, sizeof(emuconfig_t));
  uint16_t checksum = calc_chksum(&config, sizeof(config) - sizeof(config.checksum));
  if (checksum != config.checksum) {
    printf("invalid config: using defaults\n");
    config_defaults();
  }
}

void config_save()
{
  config.checksum = calc_chksum(&config, sizeof(config) - sizeof(config.checksum));
  eeprom_update_block(&config, CONFIG_EE_ADDR, sizeof(emuconfig_t));
}

void config_dump()
{
  printf("dwell_time_ms : %u\n",config.dwell_time_ms);
  printf("idle_rpm : %u\n", config.idle_rpm);
  printf("pwm_max : %u\n", config.pwm_max);
  printf("pwm_min : %u\n", config.pwm_min);
  printf("rpm_limit : %u\n", config.rpm_limit);
  printf("version : %u\n", config.version);
  printf("checksum : 0x%04x\n", config.checksum);
  inj_map_dump();
}
