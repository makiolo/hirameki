#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <tuple>

namespace cal {

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

auto add_mday(std::time_t t, int n)
{
	auto tm = to_localtime(t);
	tm->tm_mday += n;
	return to_time(tm);
}

auto add_mon(std::time_t t, int n)
{
	auto tm = to_localtime(t);
	tm->tm_mon += n;
	return to_time(tm);
}

auto add_year(std::time_t t, int n)
{
	auto tm = to_localtime(t);
	tm->tm_year += n;
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

auto get_time(std::time_t t)
{
	return std::make_tuple( get_hour(t), get_min(t), get_sec(t) );
}

auto get_datetime(std::time_t t)
{
	return std::make_tuple( get_year(t), get_mon(t), get_mday(t), get_hour(t), get_min(t), get_sec(t) );
}

double timestamp_high_precision()
{
	// TODO: use to_time_t and convert in time_t
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000000.0;
}

auto make_time(int hour, int min, int sec = 0)
{
	auto tm = to_localtime( time() );
	struct tm ts;
	ts.tm_year = tm->tm_year - 1900;
	ts.tm_mon = tm->tm_mon;
	ts.tm_mday = tm->tm_mday;
	ts.tm_hour = hour;
	ts.tm_min = min;
	ts.tm_sec = sec;
	ts.tm_isdst = -1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
	return to_time(&ts);
}
	
auto make_datetime(int year, int mon, int mday, int hour, int min, int sec = 0)
{
	struct tm ts;
	ts.tm_year = year - 1900;
	ts.tm_mon = mon;
	ts.tm_mday = mday;
	ts.tm_hour = hour;
	ts.tm_min = min;
	ts.tm_sec = sec;
	ts.tm_isdst = -1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
	return to_time(&ts);
}

// if on_time <= time_to_check < off_time -> true
// dont work (need use std::difftime)
bool inside_interval(std::time_t self, std::time_t from, std::time_t to)
{
	if (from > to)
		if (from < self || self < to)
			return true;
	else if (from <= to)
		if (from <= self && self < to)
			return true;
	return false;
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
