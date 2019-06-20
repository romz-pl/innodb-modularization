#pragma once

/** Explicitly call the destructor, this is to get around Clang bug#12350.
@param[in,out]	p		Instance on which to call the destructor */
template <typename T>
void call_destructor(T *p) {
  p->~T();
}
