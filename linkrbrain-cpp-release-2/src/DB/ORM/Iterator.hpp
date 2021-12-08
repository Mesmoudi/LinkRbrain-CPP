#ifndef LINKRBRAIN2019__SRC__DB__ORM__ITERATOR_HPP
#define LINKRBRAIN2019__SRC__DB__ORM__ITERATOR_HPP


#include "./Controller.hpp"


namespace DB::ORM {

    template <typename Model>
    class Iterator {
    public:

        Iterator(Controller<Model>& model_controller, ::DB::Iterator db_iterator) :
            _model_controller(model_controller),
            _db_iterator(db_iterator) {}

        inline Iterator& begin() {
            _db_iterator.begin();
            if ((bool) _db_iterator) {
                _model_controller.set_instance(_instance, * _db_iterator.get_cursor());
            }
            return *this;
        }
        inline static const bool end() {
            return false;
        }
        inline operator bool () {
            return (bool) _db_iterator;
        }
        inline Iterator& operator ++ () {
            ++_db_iterator;
            if ((bool) _db_iterator) {
                _model_controller.set_instance(_instance, * _db_iterator.get_cursor());
            }
            return *this;
        }
        inline Model& operator * () {
            return _instance;
        }
        inline Model& operator -> () {
            return _instance;
        }

    private:
        Controller<Model>& _model_controller;
        ::DB::Iterator _db_iterator;
        Model _instance;
    };


} // DB::ORM


#endif // LINKRBRAIN2019__SRC__DB__ORM__ITERATOR_HPP
