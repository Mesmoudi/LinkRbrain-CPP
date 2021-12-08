#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__GRAPH_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__GRAPH_HPP


#include "./Node.hpp"
#include "./Edge.hpp"
#include "Types/Table.hpp"

#include <math.h>
#include <vector>
#include <ostream>
#include <unordered_map>


namespace LinkRbrain::Graph {


    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    struct Graph {

		size_t count;
		T dt;
		T e;
		T k_spring;
		T f_max, df;
		T r, r_1;
		T Emax;
		std::vector<Node<T>> nodes;
		std::vector<Edge<T>> edges;

		Graph(size_t _count) :
            count(_count),
            dt(0.01),
            k_spring(5.0),
            r(0.5),
            f_max(log(1.0 + _count)),
            df(0.00001 * log(1 + _count)),
            Emax(1e-1 / (T)_count)
        {
			//	Nodes
			T R = 8. * r * count;
			T k = 2. * M_PI / (T)count;
            for (size_t i = 0; i < count; i++) {
                const T theta = k * (T)i;
                const T x = R * cos(theta);
                const T y = R * sin(theta);
                nodes.push_back({x, y, r, -x, -y});
			}
            for (size_t i = 0; i < count; i++) {
                _nodes_indexes.insert({&nodes[i], i});
            }
			//	Edges
            for (size_t i = 0; i < count; i++) {
                for (size_t j = 0; j < i; j++) {
                    edges.push_back({
                        nodes[i],
                        nodes[j]
                    });
                }
            }
		}

		const size_t get_edge_index(size_t i, size_t j) const {
            if (i < j) {
                std::swap(i, j);
            }
            return j + i * (i - 1) / 2;
        }
		Edge<T>& get_edge(const size_t i, const size_t j) {
            return edges[get_edge_index(i, j)];
        }
		void add_link(const size_t i, const size_t j, const T strength) {
            if (i == j) {
                return;
            }
            get_edge(i, j).k_spring += k_spring * strength;
		}

		void iterate(const T friction) {
            for (auto& node : nodes) {
                node.reset_acceleration();
                node.compute_friction(friction);
            }
            for (auto& edge : edges) {
                edge.compute();
            }
            for (auto& node : nodes) {
                node.compute_newton(dt);
            }
		}

		const T compute_energy() const {
			T energy = 0.;
			for (const auto& node : nodes) {
				energy += node.compute_energy();
			}
			return energy;
		}

		const size_t compute(const size_t batch_iterations_count=100, const size_t max_iterations_count=-1) {
			T friction = 0.;
			size_t iterations_count = 0;
			do {
				for (size_t i = 0; i < batch_iterations_count; i++) {
					if (friction < f_max){
						friction += df;
					}
					iterate(friction);
				}
                iterations_count += batch_iterations_count;
			} while (compute_energy() > Emax && iterations_count < max_iterations_count);
            return iterations_count;
		}

		void normalize_coordinates() {
			//	Detect extrema
            T x_min, x_max;
            T y_min, y_max;
			x_min = x_max = nodes[0].x;
			y_min = y_max = nodes[0].y;
			for (const auto& node : nodes) {
				if (node.x < x_min){
					x_min = node.x;
				} else if (node.x > x_max){
					x_max = node.x;
				}
				if (node.y < y_min){
					y_min = node.y;
				} else if (node.y > y_max){
					y_max = node.y;
				}
			}
			//	Scale factor
			const T dx = x_max - x_min;
			const T dy = y_max - y_min;
			const T scale = (dx > dy) ? 1/dx : 1/dy;
			//	Normalize
			for (auto& node : nodes) {
				node.r *= scale;
				node.x -= x_min;
				node.x *= scale;
				node.y -= y_min;
				node.y *= scale;
			}
		}
		void normalize_links() {
            if (edges.size() == 0) {
                return;
            }
			T k_min, k_max;
            k_min = k_max = edges[0].k_spring;
            for (const auto& edge : edges) {
                if (k_min > edge.k_spring){
                    k_min = edge.k_spring;
                } else if (k_max < edge.k_spring){
                    k_max = edge.k_spring;
                }
            }
            //
			const T dk = k_max - k_min;
            if (dk == 0.) {
                return;
            }
            for (auto& edge : edges) {
				edge.k_spring -= k_min;
				edge.k_spring /= dk;
			}
		}
		void normalize() {
			normalize_coordinates();
            normalize_links();
		}
        const size_t& get_node_index(const Node<T>& node) const {
            static const size_t not_found = -1;
            const auto& it = _nodes_indexes.find(&node);
            if (it == _nodes_indexes.end()) {
                return not_found;
            }
            return it->second;
        }

    private:

        std::unordered_map<const Node<T>*, size_t> _nodes_indexes;

    };


    template <typename T>
    std::ostream& operator<< (std::ostream& buffer, const Graph<T>& graph) {
        // parameters
        // nodes
        Types::Table nodes_table("i", "r", "r_1", "x", "y", "vx", "vy", "ax", "ay");
        for (size_t i = 0; i < graph.count; i++) {
            auto& node = graph.nodes[i];
            nodes_table.add_row(i, node.r, node.r_1, node.x, node.y, node.vx, node.vy, node.ax, node.ay);
        }
        buffer << "Graph nodes:\n" << nodes_table;
        // edges
        Types::Table edges_table("i", "node i", "node j", "stiffness", "length", "force.x", "force.y");
        for (size_t i = 0; i < graph.edges.size(); i++) {
            auto& edge = graph.edges[i];
            edges_table.add_row(i, graph.get_node_index(edge.node_i), graph.get_node_index(edge.node_j),
                edge.k_spring, edge.length, edge.force_x, edge.force_y);
        }
        buffer << "Graph edges:\n" << edges_table;
        // the end
        return buffer;
    }


} // LinkRbrain::Graph


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__GRAPH_HPP
