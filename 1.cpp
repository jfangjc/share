#include "gdwg_graph.h"

#include <catch2/catch.hpp>
#include <optional>

TEST_CASE("basic test") {
	auto g = gdwg::Graph<int, std::string>{};
	auto n = 5;
	g.insert_node(n);
	CHECK(g.is_node(n));
}

TEST_CASE("Graph Constructors and Assignment") {
	SECTION("Default Constructor") {
		gdwg::Graph<int, int> g;
		CHECK(g.empty());
		CHECK(g.nodes().empty());
		CHECK(g.begin() == g.end());
	}

	SECTION("Initializer List Constructor") {
		gdwg::Graph<std::string, int> g{"A", "B", "C"};
		CHECK(g.is_node("A"));
		CHECK(g.is_node("B"));
		CHECK(g.is_node("C"));
		CHECK(!g.is_node("D"));
		CHECK(g.nodes().size() == 3);
		CHECK(g.edges("A", "B").empty());
	}

	SECTION("Iterator Range Constructor") {
		std::vector<int> v{1, 2, 3, 4, 5};
		gdwg::Graph<int, int> g(v.begin(), v.end());
		CHECK(g.nodes().size() == 5);
		for (int i = 1; i <= 5; ++i) {
			CHECK(g.is_node(i));
		}
	}

	SECTION("Copy Constructor") {
		gdwg::Graph<std::string, int> g1;
		g1.insert_node("A");
		g1.insert_node("B");
		g1.insert_edge("A", "B", 5);

		gdwg::Graph<std::string, int> g2(g1);
		CHECK(g1 == g2);
		CHECK(g2.is_node("A"));
		CHECK(g2.is_node("B"));
		CHECK(g2.is_connected("A", "B"));
		CHECK(g2.edges("A", "B").front()->get_weight() == 5);

		g2.insert_node("C");
		CHECK(!g1.is_node("C"));
	}

	SECTION("Move Constructor") {
		gdwg::Graph<std::string, int> g1;
		g1.insert_node("A");
		g1.insert_node("B");
		g1.insert_edge("A", "B", 5);

		gdwg::Graph<std::string, int> g2(std::move(g1));
		CHECK(g1.empty()); // Moved-from object is empty
		CHECK(g2.is_node("A"));
		CHECK(g2.is_node("B"));
		CHECK(g2.is_connected("A", "B"));
		CHECK(g2.edges("A", "B").front()->get_weight() == 5);
	}

	SECTION("Copy Assignment") {
		gdwg::Graph<std::string, int> g1;
		g1.insert_node("A");
		g1.insert_node("B");
		g1.insert_edge("A", "B", 5);

		gdwg::Graph<std::string, int> g2;
		g2.insert_node("X"); // Should be overwritten
		g2 = g1;

		CHECK(g1 == g2);
		CHECK(g2.is_node("A"));
		CHECK(!g2.is_node("X"));

		// Ensure it's a deep copy
		g2.insert_node("C");
		CHECK(!g1.is_node("C"));
	}

	SECTION("Move Assignment") {
		gdwg::Graph<std::string, int> g1;
		g1.insert_node("A");
		g1.insert_node("B");
		g1.insert_edge("A", "B", 5);

		gdwg::Graph<std::string, int> g2;
		g2.insert_node("X");
		g2 = std::move(g1);

		CHECK(g1.empty());
		CHECK(g2.is_node("A"));
		CHECK(!g2.is_node("X"));
		CHECK(g2.is_connected("A", "B"));
	}
}

TEST_CASE("Graph Modifiers") {
	gdwg::Graph<std::string, double> g;

	SECTION("insert_node") {
		CHECK(g.insert_node("A"));
		CHECK(g.is_node("A"));
		CHECK(g.nodes().size() == 1);

		CHECK(g.insert_node("B"));
		CHECK(g.is_node("B"));
		CHECK(g.nodes().size() == 2);
		CHECK(g.edges("A", "B").empty());

		CHECK(!g.insert_node("A"));
		CHECK(g.nodes().size() == 2);
	}

	SECTION("insert_edge") {
		g.insert_node("A");
		g.insert_node("B");
		g.insert_node("C");

		CHECK(g.insert_edge("A", "B"));
		CHECK(g.is_connected("A", "B"));
		CHECK(!g.insert_edge("A", "B"));

		CHECK(g.insert_edge("A", "B", 3.14));
		CHECK(g.insert_edge("A", "B", 6.28));
		CHECK(!g.insert_edge("A", "B", 3.14));

		CHECK(g.insert_edge("C", "C", 1.0));
		CHECK(g.is_connected("C", "C"));

		CHECK_THROWS_WITH(g.insert_edge("A", "D", 1.0),
		                  "Cannot call gdwg::Graph<N, E>::insert_edge when either src or dst node "
		                  "does not exist");
		CHECK_THROWS_WITH(g.insert_edge("D", "A", 1.0),
		                  "Cannot call gdwg::Graph<N, E>::insert_edge when either src or dst node "
		                  "does not exist");
	}

	SECTION("replace_node") {
		g.insert_node("A");
		g.insert_node("B");
		g.insert_edge("A", "B", 1.1);
		g.insert_edge("B", "A", 2.2);

		CHECK(g.replace_node("A", "C"));
		CHECK(!g.is_node("A"));
		CHECK(g.is_node("C"));
		CHECK(g.is_connected("C", "B"));
		CHECK(g.is_connected("B", "C"));
		CHECK(g.edges("C", "B").front()->get_weight().value() == 1.1);

		g.insert_node("D");
		CHECK(!g.replace_node("B", "D"));
		CHECK(g.is_node("B"));

		CHECK_THROWS_WITH(g.replace_node("X", "Y"),
		                  "Cannot call gdwg::Graph<N, E>::replace_node on a node that doesn't exist");
	}

	SECTION("merge_replace_node") {
		gdwg::Graph<int, int> g_int;
		g_int.insert_node(1);
		g_int.insert_node(2);
		g_int.insert_node(3);
		g_int.insert_node(4);
		g_int.insert_edge(1, 2, 10);
		g_int.insert_edge(3, 1, 20);
		g_int.insert_edge(2, 3, 30);
		g_int.insert_edge(2, 2, 5);

		g_int.merge_replace_node(1, 2);

		CHECK(!g_int.is_node(1));
		CHECK(g_int.is_node(2));
		CHECK(g_int.is_node(3));

		auto edges_2_2 = g_int.edges(2, 2);
		auto edges_2_3 = g_int.edges(2, 3);
		auto edges_3_2 = g_int.edges(3, 2);

		CHECK(edges_2_2.size() == 2);
		CHECK(edges_2_3.size() == 1);
		CHECK(edges_3_2.size() == 1);

		CHECK(edges_3_2[0]->get_weight() == 20);
		CHECK(edges_2_3[0]->get_weight() == 30);

		g_int.merge_replace_node(3, 4);
		CHECK(!g_int.is_node(3));
		CHECK(g_int.edges(4, 2).size() == 1);
	}

	SECTION("merge_replace_node duplicate") {
		gdwg::Graph<int, int> g_int;
		g_int.insert_node(1);
		g_int.insert_node(2);
		g_int.insert_node(3);
		g_int.insert_edge(1, 2, 10);
		g_int.insert_edge(1, 3, 30);
		g_int.insert_edge(2, 3, 30);

		g_int.merge_replace_node(1, 2);

		CHECK(!g_int.is_node(1));
		CHECK(g_int.is_node(2));
		CHECK(g_int.is_node(3));

		CHECK(g_int.edges(2, 3).size() == 1);
	}

	SECTION("merge_replace_node throw") {
		gdwg::Graph<char, int> g_char;
		g_char.insert_node('A');
		g_char.insert_node('B');
		g_char.insert_node('C');
		g_char.insert_edge('A', 'C', 1);
		g_char.insert_edge('B', 'C', 1);

		g_char.merge_replace_node('A', 'B');
		CHECK(!g_char.is_node('A'));
		CHECK(g_char.edges('B', 'C').size() == 1);

		CHECK_THROWS_WITH(g_char.merge_replace_node('A', 'X'),
		                  "Cannot call gdwg::Graph<N, E>::merge_replace_node on old or new data if "
		                  "they don't exist in the graph");
	}

	SECTION("erase_node") {
		g.insert_node("A");
		g.insert_node("B");
		g.insert_node("C");
		g.insert_edge("A", "B", 1.0);
		g.insert_edge("C", "A", 2.0);

		CHECK(g.erase_node("A"));
		CHECK(!g.is_node("A"));
		CHECK_THROWS_WITH(g.merge_replace_node("A", "B"),
		                  "Cannot call gdwg::Graph<N, E>::merge_replace_node on old or new data if "
		                  "they don't exist in the graph");
		CHECK_THROWS_WITH(g.merge_replace_node("C", "A"),
		                  "Cannot call gdwg::Graph<N, E>::merge_replace_node on old or new data if "
		                  "they don't exist in the graph");
		CHECK(g.nodes().size() == 2);

		CHECK(!g.erase_node("D"));
	}

	SECTION("erase_edge by value") {
		g.insert_node("A");
		g.insert_node("B");
		g.insert_edge("A", "B");
		g.insert_edge("A", "B", 1.1);
		g.insert_edge("A", "B", 2.2);

		CHECK(g.erase_edge("A", "B", 1.1));
		CHECK(g.edges("A", "B").size() == 2);
		CHECK(!g.erase_edge("A", "B", 9.9));
		CHECK(g.erase_edge("A", "B"));
		CHECK(g.edges("A", "B").size() == 1);
		CHECK(!g.erase_edge("A", "B"));

		CHECK_THROWS_WITH(g.erase_edge("A", "X", 1.0),
		                  "Cannot call gdwg::Graph<N, E>::erase_edge on src or dst if they don't "
		                  "exist in the graph");
	}

	SECTION("erase_edge by iterator") {
		g.insert_node("A");
		g.insert_node("B");
		g.insert_edge("A", "B", 1.1);
		g.insert_edge("A", "B", 2.2);
		g.insert_edge("A", "B", 3.3);

		auto it = g.find("A", "B", 2.2);
		REQUIRE(it != g.end());

		auto next_it = g.erase_edge(it);
		CHECK(g.is_connected("A", "B"));
		CHECK(g.edges("A", "B").size() == 2);
		CHECK((*next_it).weight.value() == 3.3);

		auto start_it = g.find("A", "B", 1.1);
		auto end_it = g.end();
		auto ret_it = g.erase_edge(start_it, end_it);
		CHECK_FALSE(g.empty());
		CHECK(ret_it == g.end());
	}

	SECTION("clear") {
		g.insert_node("A");
		g.insert_edge("A", "A", 1.0);
		g.clear();
		CHECK(g.empty());
		CHECK(g.nodes().empty());
	}
}

TEST_CASE("Graph Accessors") {
	gdwg::Graph<int, std::string> g;
	g.insert_node(1);
	g.insert_node(2);
	g.insert_node(3);
	g.insert_edge(1, 2, "hello");
	g.insert_edge(1, 3, "world");
	g.insert_edge(1, 2, "another");

	SECTION("is_node, empty") {
		CHECK(g.is_node(1));
		CHECK(!g.is_node(99));
		CHECK(!g.empty());
		gdwg::Graph<int, int> g_empty;
		CHECK(g_empty.empty());
	}

	SECTION("is_connected") {
		CHECK(g.is_connected(1, 2));
		CHECK(g.is_connected(1, 3));
		CHECK(!g.is_connected(2, 1));
		CHECK_THROWS_WITH(g.is_connected(1, 99),
		                  "Cannot call gdwg::Graph<N, E>::is_connected if src or dst node don't "
		                  "exist in the graph");
	}

	SECTION("nodes") {
		auto nodes = g.nodes();
		std::vector<int> expected{1, 2, 3};
		CHECK(nodes == expected);
	}

	SECTION("edges") {
		auto edges12 = g.edges(1, 2);
		CHECK(edges12.size() == 2);

		CHECK(edges12[0]->get_weight().value() == "another");
		CHECK(edges12[1]->get_weight().value() == "hello");

		auto edges13 = g.edges(1, 3);
		CHECK(edges13.size() == 1);
		CHECK(edges13[0]->get_weight().value() == "world");
		CHECK(g.edges(2, 1).empty());
		CHECK_THROWS_WITH(g.edges(1, 99),
		                  "Cannot call gdwg::Graph<N, E>::edges if src or dst node don't exist in "
		                  "the graph");
	}

	SECTION("connections") {
		auto conns = g.connections(1);
		std::vector<int> expected{2, 3};
		CHECK(conns == expected);

		CHECK(g.connections(2).empty());

		CHECK_THROWS_WITH(g.connections(99),
		                  "Cannot call gdwg::Graph<N, E>::connections if src doesn't exist in the "
		                  "graph");
	}
}

TEST_CASE("Graph Iterator") {
	gdwg::Graph<char, int> g;
	g.insert_node('A');
	g.insert_node('B');
	g.insert_node('C');
	g.insert_edge('A', 'B', 1);
	g.insert_edge('A', 'C', 2);
	g.insert_edge('B', 'C', 3);

	SECTION("begin, end, and traversal") {
		auto it = g.begin();
		CHECK((*it).from == 'A');
		CHECK((*it).to == 'B');
		CHECK((*it).weight.value() == 1);
		++it;
		CHECK((*it).from == 'A');
		CHECK((*it).to == 'C');
		CHECK((*it).weight.value() == 2);
		++it;
		CHECK((*it).from == 'B');
		CHECK((*it).to == 'C');
		CHECK((*it).weight.value() == 3);
		++it;
		CHECK(it == g.end());
		--it;
		CHECK((*it).from == 'B');
		CHECK((*it).to == 'C');
		--it;
		--it;
		CHECK(it == g.begin());
	}

	SECTION("find") {
		auto it_found = g.find('A', 'C', 2);
		REQUIRE(it_found != g.end());
		CHECK((*it_found).from == 'A');
		CHECK((*it_found).to == 'C');

		auto it_not_found = g.find('A', 'C', 99);
		CHECK(it_not_found == g.end());

		auto it_no_node = g.find('X', 'A', 1);
		CHECK(it_no_node == g.end());
	}

	SECTION("empty graph iterators") {
		gdwg::Graph<int, int> empty_g;
		CHECK(empty_g.begin() == empty_g.end());
	}
}

TEST_CASE("Comparisons and Extractor") {
	gdwg::Graph<int, int> g1;
	g1.insert_node(1);
	g1.insert_node(2);
	g1.insert_edge(1, 2, 100);

	gdwg::Graph<int, int> g2;
	g2.insert_node(1);
	g2.insert_node(2);
	g2.insert_edge(1, 2, 100);

	gdwg::Graph<int, int> g3;
	g3.insert_node(1);
	g3.insert_node(2);
	g3.insert_edge(1, 2, 200); // Different edge

	gdwg::Graph<int, int> g4;
	g4.insert_node(1); // Different nodes

	SECTION("operator==") {
		CHECK(g1 == g2);
		CHECK(!(g1 == g3));
		CHECK(!(g1 == g4));
	}

	SECTION("Extractor operator<< modify from example") {
		using graph = gdwg::Graph<int, int>;
		auto const v = std::vector<std::tuple<int, int, std::optional<int>>>{
		    {4, 1, -4},
		    {3, 2, 2},
		    {2, 4, std::nullopt},
		    {2, 4, 2},
		    {2, 1, 1},
		    {4, 1, std::nullopt},
		    {6, 2, 5},
		    {5, 2, std::nullopt},
		};
		auto g = graph{};
		for (const auto& [from, to, weight] : v) {
			g.insert_node(from);
			g.insert_node(to);
			g.insert_edge(from, to, weight);
		}
		g.insert_node(64);

		auto out = std::ostringstream{};
		out << g;
		auto const expected_output = std::string_view(R"(1 (
)
2 (
  2 -> 1 | W | 1
  2 -> 4 | U
  2 -> 4 | W | 2
)
3 (
  3 -> 2 | W | 2
)
4 (
  4 -> 1 | U
  4 -> 1 | W | -4
)
5 (
  5 -> 2 | U
)
6 (
  6 -> 2 | W | 5
)
64 (
)
)");
		CHECK(out.str() == expected_output);
	}
}

TEST_CASE("Edge Class Hierarchy and Details") {
	using graph = gdwg::Graph<int, int>;

	SECTION("UnweightedEdge functionality") {
		graph g;
		g.insert_node(1);
		g.insert_node(2);
		g.insert_edge(1, 2, std::nullopt);

		auto edges = g.edges(1, 2);
		REQUIRE(edges.size() == 1);
		auto& edge = edges[0];

		CHECK(!edge->is_weighted());
		CHECK(!edge->get_weight().has_value());

		auto nodes = edge->get_nodes();
		CHECK(nodes.first == 1);
		CHECK(nodes.second == 2);

		CHECK(edge->print_edge() == "1 -> 2 | U");
	}

	SECTION("WeightedEdge functionality") {
		graph g;
		g.insert_node(1);
		g.insert_node(2);
		g.insert_edge(1, 2, 100);

		auto edges = g.edges(1, 2);
		REQUIRE(edges.size() == 1);
		auto& edge = edges[0];

		CHECK(edge->is_weighted());
		CHECK(edge->get_weight().has_value());
		CHECK(edge->get_weight().value() == 100);

		auto nodes = edge->get_nodes();
		CHECK(nodes.first == 1);
		CHECK(nodes.second == 2);

		CHECK(edge->print_edge() == "1 -> 2 | W | 100");
	}

	SECTION("Unweighted and Weighted edge sorting in edges") {
		gdwg::Graph<int, int> g;
		g.insert_node(1);
		g.insert_node(2);
		g.insert_edge(1, 2, 5);
		g.insert_edge(1, 2, std::nullopt);
		g.insert_edge(1, 2, -10);

		auto edges_vec = g.edges(1, 2);
		REQUIRE(edges_vec.size() == 3);

		// Unweighted edge should be first
		CHECK_FALSE(edges_vec[0]->is_weighted());

		// Weighted edges should be sorted
		CHECK(edges_vec[1]->get_weight().value() == -10);
		CHECK(edges_vec[2]->get_weight().value() == 5);
	}
}

TEST_CASE("Graph Iterator for Weight and Unweight") {
	gdwg::Graph<std::string, int> g;
	g.insert_node("A");
	g.insert_node("B");
	g.insert_node("C");
	g.insert_node("D");
	g.insert_node("E");

	g.insert_edge("A", "C", 5);
	g.insert_edge("A", "B", 10);
	g.insert_edge("B", "D", 20);
	g.insert_edge("D", "D");
	g.insert_edge("A", "C");

	SECTION("Forward ") {
		auto it = g.begin();

		CHECK((*it).from == "A");
		CHECK((*it).to == "B");
		CHECK((*it).weight.value() == 10);
		++it;
		CHECK((*it).from == "A");
		CHECK((*it).to == "C");
		CHECK_FALSE((*it).weight.has_value());
		++it;
		CHECK((*it).from == "A");
		CHECK((*it).to == "C");
		CHECK((*it).weight.value() == 5);
		++it;
		CHECK((*it).from == "B");
		CHECK((*it).to == "D");
		CHECK((*it).weight.value() == 20);
		++it;
		CHECK((*it).from == "D");
		CHECK((*it).to == "D");
		CHECK_FALSE((*it).weight.has_value());
		++it;
		CHECK(it == g.end());
	}

	SECTION("Forward ") {
		auto it = g.end();
		--it;
		CHECK((*it).from == "D");
		CHECK((*it).to == "D");
		CHECK_FALSE((*it).weight.has_value());
		--it;
		CHECK((*it).from == "B");
		CHECK((*it).to == "D");
		CHECK((*it).weight.value() == 20);
		--it;
		CHECK((*it).from == "A");
		CHECK((*it).to == "C");
		CHECK((*it).weight.value() == 5);
		--it;
		CHECK((*it).from == "A");
		CHECK((*it).to == "C");
		CHECK_FALSE((*it).weight.has_value());
		--it;
		CHECK((*it).from == "A");
		CHECK((*it).to == "B");
		CHECK((*it).weight.value() == 10);

		CHECK(it == g.begin());
	}
}
