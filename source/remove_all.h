#ifndef REMOVE_ALL_H
#define REMOVE_ALL_H

/* C imitation of `std::filesystem::remove_all()` from C++17.
 * Unlike standard C remove(3), it can remove recursively.
 */
int remove_all(const char * const p);

#endif
