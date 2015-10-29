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

enum kind_layers
{
	minimal,
	tools,
	third_party,
	default_layer
};

enum kind_relations
{
	needs,
	use,
	improve,
	satisfy,
	esta,
	hay,
};

class node;

class relation
{
public:
	using node_internal = std::shared_ptr<node>;
	using nodes_unordered = std::unordered_set<node_internal>;
	using nodes_ordered = std::vector<node_internal>;

	relation(node_internal from, node_internal to, int kind)
		: _from(std::move(from))
		, _to(std::move(to))
		, _kind(kind)
	{
		;
	}

	auto from() const {return _from;}
	auto to() const {return _to;}
	auto kind() const {return _kind;}

protected:
	node_internal _from;
	node_internal _to;
	int _kind;
};

class node : public std::enable_shared_from_this<node>
{
public:
	using node_internal = relation::node_internal;
	using nodes_unordered = relation::nodes_unordered;
	using nodes_ordered = relation::nodes_ordered;

	node(std::string name, int priority)
		: _name(std::move(name))
		, _priority(priority)
	{
		;
	}

	~node() { ; }

	node(const node& other) = delete;
	node(node&& other) = delete;
	node& operator=(const node& other) = delete;
	node& operator=(node&& other) = delete;

	void relation_of(int kind, const node_internal& other)
	{
		_depends.emplace_back(shared(), other, kind);
		other->relation_inverse_of(shared(), kind);
	}

	void relation_inverse_of(const node_internal& other, int kind)
	{
		_inverses.emplace_back(shared(), other, kind);
	}

	nodes_ordered what(int kind)
	{
		nodes_ordered resolved;
		nodes_unordered seen;
		_resolve(kind, resolved, seen, false);

		// remove me
		auto it = std::remove(resolved.begin(), resolved.end(), shared());
		resolved.erase(it, resolved.end());

		return resolved;
	}

	nodes_ordered where(int kind)
	{
		return what(kind);
	}

	nodes_ordered how(int kind)
	{
		nodes_ordered resolved;
		nodes_unordered seen;
		_resolve(kind, resolved, seen, true);

		// remove me
		auto it = std::remove(resolved.begin(), resolved.end(), shared());
		resolved.erase(it, resolved.end());

		return resolved;
	}

	nodes_ordered who(int kind)
	{
		return how(kind);
	}

	const std::string& get_name() const { return _name; }
	int get_priority() { return _priority; }

protected:
	node_internal shared()
	{
		return shared_from_this();
	}

	void _resolve(int kind, nodes_ordered& resolved, nodes_unordered& seen, bool inverse)
	{
		seen.emplace(shared());
		std::vector<relation> relations;
		if(!inverse)
			relations = _depends;
		else
			relations = _inverses;

		for(auto& relation : relations)
		{
			if(kind == relation.kind())
			{
				node_internal node = relation.to();
				if(std::find(resolved.begin(), resolved.end(), node) == resolved.end())
				{
					if(seen.count(node) > 0)
					{
						std::stringstream ss;
						ss << "Circular reference detected: ";
						ss << get_name() <<  " and " << node->get_name();
						throw std::runtime_error(ss.str());
					}
					node->_resolve(kind, resolved, seen, inverse);
				}
			}
		}
		resolved.emplace_back(shared());
		seen.erase(shared());
	}

protected:
	std::string _name;
	std::vector<relation> _depends;
	std::vector<relation> _inverses;
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

	node_internal make_node(const std::string& key, int priority = default_layer)
	{
		auto newnode = std::make_shared<node>(key, priority);
		_nodes.insert(_nodes.begin(), newnode);
		return newnode;
	}

	const nodes_ordered& get_container() const { return _nodes; }

	void resolve_all(int kind = needs, bool merge_roots = true)
	{
		// 1/4: Generate solutions in each node
		plans_vector sols;
		nodes_unordered classified;
		for(auto& node : _nodes)
		{
			if(classified.count(node) == 0)
			{
				auto solution = node->what(kind);
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

	auto cloog = g.make_node("cloog", minimal);
	auto gcc = g.make_node("gcc", minimal);
	auto minimals = g.make_node("minimal", minimal);
	auto lapack = g.make_node("lapack", tools);
	auto cmake = g.make_node("cmake", tools);
	auto python = g.make_node("python", tools);
	auto tool = g.make_node("tools", tools);
	auto xerces = g.make_node("xerces", third_party);
	auto xalan = g.make_node("xalan", third_party);
	auto lua = g.make_node("lua", third_party);
	auto frigo = g.make_node("frigo");
	auto hambre = g.make_node("hambre");
	auto sed = g.make_node("sed");
	auto tonica = g.make_node("tonica");

	gcc->relation_of(needs, cloog);
	minimals->relation_of(needs, cloog);
	minimals->relation_of(needs, gcc);
	lapack->relation_of(needs, cmake);
	tool->relation_of(needs, cmake);
	tool->relation_of(needs, python);
	tool->relation_of(needs, lapack);
	xalan->relation_of(needs, xerces);

	std::cout << "-------------------------" << std::endl;
	frigo->relation_of(satisfy, hambre);
	frigo->relation_of(satisfy, sed);
	tonica->relation_of(satisfy, sed);
	tonica->relation_of(esta, frigo);

	std::cout << "Como satisfacer el hambre?" << std::endl;
	for(auto& node : hambre->how(satisfy))
	{
		std::cout << node->get_name() << std::endl;
	}

	std::cout << "Que satisface el frigo?" << std::endl;
	for(auto& node : frigo->what(satisfy))
	{
		std::cout << node->get_name() << std::endl;
	}

	std::cout << "Donde esta la tonica?" << std::endl;
	for(auto& node : tonica->where(esta))
	{
		std::cout << node->get_name() << std::endl;
	}

	std::cout << "En el frigo que hay?" << std::endl;
	for(auto& node : frigo->how(esta))
	{
		std::cout << node->get_name() << std::endl;
	}

	std::cout << "que necesita gcc?" << std::endl;
	for(auto& node : gcc->what(needs))
	{
		std::cout << node->get_name() << std::endl;
	}

	std::cout << "Por quien es necesitado xerces?" << std::endl;
	for(auto& node : xerces->who(needs))
	{
		std::cout << node->get_name() << std::endl;
	}

	//g.resolve_all(needs);
	//g.show_plan();

	return 0;
}

