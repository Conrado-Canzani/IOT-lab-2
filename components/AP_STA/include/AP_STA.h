#ifndef AP_STA_H
#define AP_STA_H

void wifi_init_ap(const char *ssid,
                  const char *password,
                  int channel,
                  int max_conn);

void wifi_init_sta(const char *ssid,
                   const char *password);

#endif