#ifndef GLOBAL_INCLUDE_H
#define GLOBAL_INCLUDE_H

#include <gtkmm.h>

#if GTKMM_MINOR_VERSION < 24

namespace Gtk {

template<class T, class... T_Args>
T* make_managed(T_Args&&... args)
{
  return manage(new T(std::forward<T_Args>(args)...));
}

};
#endif

#endif
