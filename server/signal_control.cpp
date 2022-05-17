#include "signal_control.h"

std::unordered_map<int, SignalContrlFunc> SignalControl::_maps;

void SignalControl::add_signal(int sig, SignalContrlFunc func)
{
	_maps[sig] = func;
	signal(sig, &SignalControl::do_signal);
}

void SignalControl::do_signal(int sig)
{
	if (_maps.find(sig) != _maps.end())
	{
		_maps[sig](sig);
	}
}


