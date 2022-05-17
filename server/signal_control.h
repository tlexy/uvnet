#ifndef SIGNAL_CONTROL_H
#define SIGNAL_CONTROL_H

#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <signal.h>

using SignalContrlFunc = std::function<void(int)>;

class SignalControl
{
public:
	static void add_signal(int sig, SignalContrlFunc func);

	static void do_signal(int);

private:
	static std::unordered_map<int, SignalContrlFunc> _maps;
};

#endif