#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

class EnableSharedFromThisBase;

template <typename T>
class EnableSharedFromThis;