/*
  ClusterConfig.h - Library for managing configuration for Cluster devices.
  Copyright (c) 2020 Devin Henkel-Legare.  All right reserved.
*/

// ensure this library description is only included once
#ifndef ClusterConfig_h
#define ClusterConfig_h

// include types & constants of Wiring core API
#include "Arduino.h"

// library interface description
class ClusterConfig
{
  // user-accessible "public" interface
  public:
    ClusterConfig();
    char *getConfig();
    void setConfig(char *configString); 
    char *getUUID();

  // library-accessible "private" interface
  private:
    _configString;
};

#endif

