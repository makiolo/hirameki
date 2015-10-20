#include <iostream>
#include <memory>
#include <unordered_set>
#include <set>
#include <unordered_map>
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
	using nodes_unordered = node::nodes_unordered;
	using nodes_ordered = node::nodes_ordered;
	using plans_set = std::set<nodes_ordered>;

	node_internal make_node(const std::string& key)
	{
		auto newnode = std::make_shared<node>(key);
		_nodes.insert(_nodes.begin(), newnode);
		return newnode;
	}

	const nodes_ordered& get_container() const { return _nodes; }

	void calculate(bool merge_roots = true)
	{
		// 1/4: Generate solutions in each node
		plans_set sols;
		nodes_unordered classified;
		for(auto& node : _nodes)
		{
			if(classified.count(node) == 0)
			{
				auto solution = node->resolve();
				sols.emplace( solution );
				for(auto& n : solution)
				{
					classified.emplace(n);
				}
			}
		}

		// 2/4: remove solutions are subset of other solution
		plans_set sols2 = sols;
		for(auto& solution1 : sols)
		{
			for(auto& solution2 : sols)
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
						sols2.erase(solution1);
					}
				}
			}
		}

		if(merge_roots)
		{
			// 3/4: merge solutions with same root
			std::unordered_map<node_internal, nodes_ordered> sols3;
			for(auto& solution : sols2)
			{
				auto& first = solution.front();
				auto& chunk = sols3[first];
				for(auto& node : solution)
				{
					if(node != first)
					{
						if(std::find(chunk.begin(), chunk.end(), node) == chunk.end())
						{
							chunk.emplace_back(node);
						}
					}
				}
			}

			// 4/4: write final plan
			for(auto& pair : sols3)
			{
				nodes_ordered nodes;
				nodes.emplace_back(pair.first);
				for(auto& node : pair.second)
				{
					nodes.emplace_back(node);
				}
				_solutions.emplace(nodes);
			}
		}
		else
		{
			_solutions = sols2;
		}
	}

	void show_plan()
	{
		for(auto& solution : _solutions)
		{
			std::cout << "---------------- plan " << std::endl;
			for(auto& node : solution)
			{
				std::cout << node->get_name() << std::endl;
			}
		}
	}

protected:
	plans_set _solutions;
	nodes_ordered _nodes;
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
	auto j = g.make_node("j");

	asyncply->needs(fes);
	fes->needs(docker);
	docker->needs(apt_get);
	docker->needs(linux);
	apt_get->needs(linux);
	e->needs(d);
	j->needs(d);

	g.calculate();
	g.show_plan();

	return 0;
}

