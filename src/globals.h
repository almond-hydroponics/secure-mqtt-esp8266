#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef DEBUG
#define DEBUG
#endif


#ifdef DEBUG
#  define DEBUG_LOG(msg)     Serial.print(msg)
#  define DEBUG_LOGF(fmt,p1) Serial.printf((fmt),(p1))
#  define DEBUG_LOGLN(msg)   Serial.println(msg)
#else
#  define DEBUG_LOG(msg)
#  define DEBUG_LOGF(fmt,p1)
#  define DEBUG_LOGLN(msg)
#endif

#endif //GLOBALS_H
