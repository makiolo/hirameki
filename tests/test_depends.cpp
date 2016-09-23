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

	node_internal make_node(const std::string& key, int priority = 0)
	{
		auto newnode = std::make_shared<node>(key, priority);
		_nodes.insert(_nodes.begin(), newnode);
		return newnode;
	}

	const nodes_ordered& get_container() const { return _nodes; }

	void resolve_all(int kind = 0, bool merge_roots = true)
	{
		// 1/5: Generate solutions in each node
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

		// 2/5: remove solutions are subset of other solution
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
			// 3/5: merge solutions with same root
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

			// 4/5: write final plan
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

			// 5/5: sort using priority
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
    enum kind_layers
    {
        default_layer,
    	minimal,
    	tools,
    	third_party
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

	graph g;

    std::vector<graph::node_internal> recursos;
    std::vector<graph::node_internal> lugares;
    std::vector<graph::node_internal> necesidades;

    // recursos (tienen beneficios)
	auto tonica = g.make_node("la tonica");
	auto coca_cola = g.make_node("la coca cola");
	auto pollo = g.make_node("el pollo");
	recursos.push_back(tonica);
	recursos.push_back(coca_cola);
	recursos.push_back(pollo);

    // lugares (tienen costes)
	auto frigo = g.make_node("el frigo");
	auto encimera = g.make_node("la encimera");
	lugares.push_back(frigo);
	lugares.push_back(encimera);
	
	// necesidades
	auto hambre = g.make_node("el hambre");
	auto sed = g.make_node("la sed");
	necesidades.push_back(hambre);
	necesidades.push_back(sed);
	
	// gustos
	auto rico = g.make_node("rico");
	auto malo = g.make_node("malo");
	
	// relaciones para recurso
	//      su lugar
	//      la necesidad que satisface

	pollo->relation_of(satisfy, hambre);
	pollo->relation_of(esta, frigo);
	
	coca_cola->relation_of(satisfy, sed);
	coca_cola->relation_of(satisfy, hambre);
	coca_cola->relation_of(esta, encimera);
	
	tonica->relation_of(satisfy, sed);
	tonica->relation_of(esta, frigo);

	// como satisfacerme y donde esta
	// elegir el recurso de mayor beneficio con el menor coste de lugar
	for(auto& need : necesidades)
	{
	 	// std::cout << "Como satisfacer " << need->get_name() << "?" << std::endl;
	    	for(auto& node : need->how(satisfy))
	    	{
	    		// std::cout << "con " << node->get_name() << ". ¿donde esta el " << node->get_name() << "?" << std::endl;
			for(auto& node2 : node->where(esta))	
			{
				std::cout << "" << node->get_name() << " esta en " << node2->get_name() << " satisface " << need->get_name() << std::endl;
			}
	   	}
		// std::cout << std::endl;
	}

	return 0;
}
