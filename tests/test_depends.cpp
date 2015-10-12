#include <iostream>
#include <memory>
#include <unordered_set>
#include <set>
#include <vector>
#include <sstream>
#include <functional>
#include <algorithm>
#include <asyncply/parallel.h>

class node : public std::enable_shared_from_this<node>
{
public:
	using node_internal = std::shared_ptr<node>;
	using nodes_unordered = std::unordered_set<node_internal>;
	using nodes_ordered = std::vector<node_internal>;

	node(std::string name)
		: _name(std::move(name))
	{
		;
	}

	virtual ~node() { ; }

	node(const node& other) = delete;
	node(node&& other) = delete;
	node& operator=(const node& other) = delete;
	node& operator=(node&& other) = delete;

	void needs(const node_internal& other)
	{
		_depends.emplace_back(other);
	}

	nodes_ordered resolve()
	{
		nodes_ordered resolved;
		nodes_unordered seen;
		_resolve(resolved, seen);
		return resolved;
	}

	const std::string& get_name() const { return _name; }

protected:
	node_internal shared()
	{
		return shared_from_this();
	}

	void _resolve(nodes_ordered& resolved, nodes_unordered& seen)
	{
		seen.emplace(shared());
		for(auto node : _depends)
		{
			if(std::find(resolved.begin(), resolved.end(), node) == resolved.end())
			{
				if(seen.count(node) > 0)
				{
					std::stringstream ss;
					ss << "Circular reference detected: ";
					ss << get_name() <<  " and " << node->get_name();
					throw std::runtime_error(ss.str());
				}
				node->_resolve(resolved, seen);
			}
		}
		resolved.emplace_back(shared());
		seen.erase(shared());
	}

protected:
	std::string _name;
	nodes_ordered _depends;
};

class graph
{
public:
	using node_internal = node::node_internal;
	using nodes_ordered = node::nodes_ordered;

	node_internal make_node(const std::string& key)
	{
		_nodes.emplace_back( std::make_shared<node>(key) );
		return _nodes.back();
	}

	const std::vector<node_internal>& get_container() const { return _nodes; }

	void calculate()
	{
		std::set<nodes_ordered> _solutions;
		for(auto& node : _nodes)
		{
			auto solution = node->resolve();
			_solutions.emplace( solution );
		}

		// copy
		_solutions_filtered = _solutions;

		for(auto& solution1 : _solutions)
		{
			for(auto& solution2 : _solutions)
			{
				if(solution1 != solution2)
				{
					// remove solution1 if is subset of solution2
					bool match = true;
					for(auto& node : solution1)
					{
						if(std::find(solution2.begin(), solution2.end(), node) == solution2.end())
						{
							match = false;
							break;
						}
					}
					if(match)
					{
						_solutions_filtered.erase(solution1);
					}
				}
			}
		}
	}

	void show_plan()
	{
		for(auto& solution : _solutions_filtered)
		{
			std::cout << "---------------- plan " << std::endl;
			for(auto& node : solution)
			{
				std::cout << node->get_name() << std::endl;
			}
		}
	}

protected:
	std::set<nodes_ordered> _solutions_filtered;
	std::vector<node_internal> _nodes;
};

int main(int, const char**)
{
	graph g;

	auto linux = g.make_node("linux");
	auto docker = g.make_node("docker");
	auto apt_get = g.make_node("apt-get");
	auto fes = g.make_node("fes");
	auto asyncply = g.make_node("asyncply");
	auto d = g.make_node("d");
	auto e = g.make_node("e");
	auto f = g.make_node("f");
	auto h = g.make_node("h");

	asyncply->needs(fes);
	fes->needs(docker);
	docker->needs(apt_get);
	docker->needs(linux);
	apt_get->needs(linux);
	e->needs(d);

	g.calculate();
	g.show_plan();

	// std::cout << "solution docker" << std::endl;
	// auto solution = docker->resolve();
	// for(auto& node : solution)
	// {
	// 	std::cout << node->get_name() << std::endl;
	// }

	return 0;
}

