#ifndef VERSION_CONFIG_H
#define VERSION_CONFIG_H

// Version is pulled from Makefile via -DAPP_VERSION
// APP_VERSION is defined by the compiler from Makefile's APP_VERSION variable
#define VERSION_APP APP_VERSION
#define VERSION_MAJOR_APP 1
#define VERSION_MINOR_APP 5
#define VERSION_MICRO_APP 0
#define VERSION_TAG_APP ""

// Version getter functions
inline unsigned int get_version_major()
{
  return VERSION_MAJOR_APP;
}

inline unsigned int get_version_minor()
{
  return VERSION_MINOR_APP;
}

inline unsigned int get_version_micro()
{
  return VERSION_MICRO_APP;
}

inline const char* get_version_tag(){
  return VERSION_TAG_APP;
}

#endif // VERSION_CONFIG_H
