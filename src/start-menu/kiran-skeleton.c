#include "src/start-menu/kiran-skeleton.h"


KiranStartMenuS *kiran_start_menu_s_get_default()
{
  static KiranStartMenuS *singleton;

  if (singleton) return singleton;

  singleton = kiran_start_menu_s_skeleton_new();
  return singleton;
}