#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__EDGE_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__EDGE_HPP


#include "./Node.hpp"

#include <math.h>


namespace LinkRbrain::Graph {

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    struct Edge {

    		//	Positive: attraction
    		//	Negative: repulsion
            T length_2, length;
    		T orientation_x, orientation_y;
    		T force_x, force_y;
    		T k_spring, k_repulsion;
    		Node<T>& node_i;
    		Node<T>& node_j;

    		Edge(Node<T>& _node_i, Node<T>& _node_j, const T _k_spring=0., const T _k_repulsion=1.5) :
                node_i(_node_i),
                node_j(_node_j),
                k_spring(_k_spring),
                k_repulsion(_k_repulsion) {}

    		void compute_position() {
    			T dx, dy;
    			dx = node_j.x - node_i.x;
    			dy = node_j.y - node_i.y;
    			//
    			const T length2 = dx * dx + dy * dy;
    			length_2 = 1 / length2;
    			length = sqrt(length2);
    			const T length_1 = 1 / length;
    			//
    			orientation_x = dx * length_1;
    			orientation_y = dy * length_1;
    		}
    		void compute_force() {
    			const T spring_force = k_spring * (length - node_i.r - node_j.r);
    			const T repulsion_force = k_repulsion * length_2;
    			const T force = spring_force - repulsion_force;
    			force_x = force * orientation_x;
    			force_y = force * orientation_y;
    		}
    		void compute_nodes() {
                node_i.ax += force_x;
                node_j.ax -= force_x;
                node_i.ay += force_y;
                node_j.ay -= force_y;
    		}
    		void compute() {
    			compute_position();
    			compute_force();
                compute_nodes();
    		}

    };


} // LinkRbrain::Graph


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__EDGE_HPP
