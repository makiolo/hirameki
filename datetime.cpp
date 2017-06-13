#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <tuple>

namespace cal {

// add clamp_time
bool check_time(std::time_t time_to_check, std::time_t on_time, std::time_t off_time)
{
	// TODO: use std::difftime
	if (on_time >= off_time)
		if (time_to_check >= on_time || time_to_check < off_time)
			return true;
	else if (on_time <= off_time)
		if (time_to_check >= on_time && time_to_check < off_time)
			return true;
	return false;
}

auto to_localtime(std::time_t t)
{
	return std::localtime(&t);
}

auto to_gmtime(std::time_t t)
{
	return std::gmtime(&t);
}

auto to_time(std::tm* tm)
{
	return std::mktime(tm);
}

std::time_t time()
{
	return std::time(nullptr);
}

auto get_tuple_time()
{
	auto t = time();
	auto tm = to_localtime(t);
	return std::make_tuple(tm->tm_hour, tm->tm_min);
}

auto _to_string(const std::tm* ts)
{
	char buf[80];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ts);
	return std::string(buf);
}

auto to_gmt_string(std::time_t t)
{
	const auto tm = to_gmtime(t);
	return _to_string(tm);
}

auto to_local_string(std::time_t t)
{
	const auto tm = to_localtime(t);
	return _to_string(tm);
}

auto add_sec(std::time_t t, int n)
{
	auto tm = to_localtime(t);
	tm->tm_sec += n;
	return to_time(tm);
}

auto add_min(std::time_t t, int n)
{
	auto tm = to_localtime(t);
	tm->tm_min += n;
	return to_time(tm);
}

auto add_hour(std::time_t t, int n)
{
	auto tm = to_localtime(t);
	tm->tm_hour += n;
	return to_time(tm);
}

int get_sec(std::time_t t)
{
	auto tm = to_localtime(t);
	return tm->tm_sec;
}

int get_min(std::time_t t)
{
	auto tm = to_localtime(t);
	return tm->tm_min;
}

int get_hour(std::time_t t)
{
	auto tm = to_localtime(t);
	return tm->tm_hour;
}

int get_mday(std::time_t t)
{
	auto tm = to_localtime(t);
	return tm->tm_mday;
}

int get_mon(std::time_t t)
{
	auto tm = to_localtime(t);
	return tm->tm_mon;
}

int get_year(std::time_t t)
{
	auto tm = to_localtime(t);
	return tm->tm_year;
}

auto get_tuple(std::time_t t)
{
	return std::make_tuple( get_hour(t), get_min(t) );
}

double time_high_precision()
{
	// TODO: use to_time_t and convert in time_t
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000000.0;
}

auto get_timestamp(int hour, int min)
{
	auto tm = to_localtime( time() );
    struct tm ts;
    ts.tm_year = tm->tm_year - 1900;
    ts.tm_mon = tm->tm_mon - 1;
    ts.tm_mday = tm->tm_mday;
    ts.tm_hour = hour;
    ts.tm_min = min;
    ts.tm_sec = 0;
    ts.tm_isdst = -1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
    return to_time(&ts);
}

}

int main()
{
	using namespace cal;

    auto t = get_timestamp(13, 45);
	std::cout << "h = " << get_hour(t) << std::endl;
	std::cout << "m = " << get_min(t) << std::endl;
	std::cout << "------" << std::endl;
	auto pas = add_min(t, -45);
	auto fut = add_min(t, 30);
	std::cout << to_local_string(pas) << std::endl;
	std::cout << to_local_string(t) << std::endl;
	std::cout << to_local_string(fut) << std::endl;
	if(check_time(t, pas, fut))
	{
		std::cout << "its ok" << std::endl;
	}
	else
	{
		std::cout << "no ok" << std::endl;
	}
}
