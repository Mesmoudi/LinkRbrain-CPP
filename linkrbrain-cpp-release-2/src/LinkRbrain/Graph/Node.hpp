#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__NODE_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__NODE_HPP


#include <type_traits>
#include <ostream>
#include <string.h>
#include <cmath>


namespace LinkRbrain::Graph {

    template <typename T, std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
    struct Node {

    		T r;
            T r_1;
    		T x;
    		T y;
    		T vx;
    		T vy;
    		T ax;
    		T ay;

    		Node(T _x, T _y, T _r, T _vx=0., T _vy=0.) :
                x(_x),
                y(_y),
                r(_r),
                r_1(1. / _r),
                vx(_vx),
                vy(_vy),
                ax(0.),
                ay(0.) {}

    		void reset_acceleration(){
    			ax = ay = 0.f;
    		}
    		void compute_friction(T f){
    			ax -= f * vx;
    			ay -= f * vy;
    		}
    		void compute_newton(T dt){
                if (!std::isnan(ax)) vx += ax * dt;
    			if (!std::isnan(ay)) vy += ay * dt;
    			if (!std::isnan(vx)) x += vx * dt;
    			if (!std::isnan(vy)) y += vy * dt;
    		}
    		const T compute_energy() const {
    			return vx * vx + vy * vy;
    		}

            const bool has_valid_coordinates() const {
                return !std::isnan(x) && !std::isnan(y);
            }

    };


} // LinkRbrain::Graph


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__GRAPH__NODE_HPP
