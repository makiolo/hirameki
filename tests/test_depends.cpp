#include <iostream>
#include <memory>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <functional>
#include <algorithm>
#include <exception>
#include <asyncply/parallel.h>

class node : public std::enable_shared_from_this<node>
{
public:
	using node_internal = std::shared_ptr<node>;
	using nodes_unordered = std::unordered_set<node_internal>;
	using nodes_ordered = std::vector<node_internal>;

	node(std::string name, int priority)
		: _name(std::move(name))
		, _priority(priority)
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
	int get_priority() { return _priority; }

protected:
	node_internal shared()
	{
		return shared_from_this();
	}

	void _resolve(nodes_ordered& resolved, nodes_unordered& seen)
	{
		seen.emplace(shared());
		for(auto& node : _depends)
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
	int _priority;
};

class graph
{
public:
	using node_internal = node::node_internal;
	using nodes_unordered = node::nodes_unordered;
	using nodes_ordered = node::nodes_ordered;
	using plans_vector = std::vector<nodes_ordered>;
	using plans_priorized_vector = std::vector<std::pair<int, nodes_ordered> >;

	node_internal make_node(const std::string& key, int priority)
	{
		auto newnode = std::make_shared<node>(key, priority);
		_nodes.insert(_nodes.begin(), newnode);
		return newnode;
	}

	const nodes_ordered& get_container() const { return _nodes; }

	void calculate(bool merge_roots = true)
	{
		// 1/4: Generate solutions in each node
		plans_vector sols;
		nodes_unordered classified;
		for(auto& node : _nodes)
		{
			if(classified.count(node) == 0)
			{
				auto solution = node->resolve();
				sols.emplace_back( solution );
				for(auto& n : solution)
				{
					classified.emplace(n);
				}
			}
		}

		// 2/4: remove solutions are subset of other solution
		plans_vector sols2 = sols;
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
						auto it = std::remove(sols2.begin(), sols2.end(), solution1);
						sols2.erase(it, sols2.end());
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
				auto priority_total = pair.first->get_priority();
				nodes.emplace_back(pair.first);
				for(auto& node : pair.second)
				{
					nodes.emplace_back(node);
					priority_total += node->get_priority();
				}
				priority_total /= static_cast<int>(nodes.size());
				_solutions.emplace_back(std::make_pair(priority_total, nodes));
			}

			// validation groups
			for(auto& pair : _solutions)
			{
				auto valid_priority = pair.first;
				auto& solution = pair.second;
				for(auto& node : solution)
				{
					if(node->get_priority() != valid_priority)
					{
						std::stringstream ss;
						ss << "Invalid group, node " << node->get_name() << " has priority " << node->get_priority() << " and expected " << valid_priority << std::endl;
						ss << "Maybe: " << node->get_name() << " this in invalid group";
						throw std::runtime_error(ss.str());
					}
				}
			}

			// 5/4: sort using priority
			std::sort(_solutions.begin(), _solutions.end(),
					[]( const plans_priorized_vector::value_type& one,
						const plans_priorized_vector::value_type& other) {
						return one.first < other.first;
					});
		}
		else
		{
			for(auto& solution : sols2)
			{
				_solutions.emplace_back(std::make_pair(0, solution));
			}
		}
	}

	void show_plan()
	{
		for(auto& pair : _solutions)
		{
			auto& solution = pair.second;
			std::cout << "---------------- plan " << std::endl;
			for(auto& node : solution)
			{
				//std::cout << node->get_name() << " (priority: " << node->get_priority() << ")" << std::endl;
				std::cout << node->get_name() << std::endl;
			}
		}
	}

protected:
	plans_priorized_vector _solutions;
	nodes_ordered _nodes;
};

int main(int, const char**)
{
	graph g;

	auto cloog = g.make_node("cloog", 10);
	auto gcc = g.make_node("gcc", 10);
	auto minimal = g.make_node("minimal", 10);
	auto lapack = g.make_node("lapack", 20);
	auto cmake = g.make_node("cmake", 20);
	auto python = g.make_node("python", 20);
	auto tools = g.make_node("tools", 20);
	auto xerces = g.make_node("xerces", 30);
	auto xalan = g.make_node("xalan", 30);
	auto lua = g.make_node("lua", 30);

	gcc->needs(cloog);
	minimal->needs(cloog);
	minimal->needs(gcc);
	lapack->needs(cmake);
	tools->needs(cmake);
	tools->needs(python);
	tools->needs(lapack);
	xalan->needs(xerces);

	g.calculate();
	g.show_plan();

	return 0;
}

